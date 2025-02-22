/* Copyright (c) <2003-2022> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "ndCoreStdafx.h"
#include "ndNewtonStdafx.h"
#include "ndWorld.h"
#include "ndJointHinge.h"
#include "ndBodyDynamic.h"
#include "ndBodyKinematic.h"
#include "ndMultiBodyVehicle.h"
#include "ndMultiBodyVehicleMotor.h"
#include "ndMultiBodyVehicleGearBox.h"
#include "ndMultiBodyVehicleTireJoint.h"
#include "ndMultiBodyVehicleTorsionBar.h"
#include "ndMultiBodyVehicleDifferential.h"
#include "ndMultiBodyVehicleDifferentialAxle.h"

#define D_MAX_CONTACT_SPEED_TRESHOLD  ndFloat32 (0.25f)
#define D_MAX_CONTACT_PENETRATION	  ndFloat32 (1.0e-2f)
#define D_MIN_CONTACT_CLOSE_DISTANCE2 ndFloat32 (5.0e-2f * 5.0e-2f)

D_CLASS_REFLECTION_IMPLEMENT_LOADER(ndMultiBodyVehicle)

ndMultiBodyVehicle::ndMultiBodyVehicle(const ndVector& frontDir, const ndVector& upDir)
	:ndModel()
	,m_localFrame(ndGetIdentityMatrix())
	,m_chassis(nullptr)
	,m_invDynamicsSolver()
	,m_motor(nullptr)
	,m_tireShape(new ndShapeChamferCylinder(ndFloat32(0.75f), ndFloat32(0.5f)))
	,m_gearBox(nullptr)
	,m_torsionBar(nullptr)
	,m_tireList()
	,m_extraBodiesAttachmentList()
	,m_axleList()
	,m_differentialList()
	,m_extraJointsAttachmentList()
	,m_downForce()
{
	m_tireShape->AddRef();
	m_localFrame.m_front = frontDir & ndVector::m_triplexMask;
	m_localFrame.m_up = upDir & ndVector::m_triplexMask;
	m_localFrame.m_right = m_localFrame.m_front.CrossProduct(m_localFrame.m_up).Normalize();
	m_localFrame.m_up = m_localFrame.m_right.CrossProduct(m_localFrame.m_front).Normalize();
}

ndMultiBodyVehicle::ndMultiBodyVehicle(const ndLoadSaveBase::ndLoadDescriptor& desc)
	:ndModel(ndLoadSaveBase::ndLoadDescriptor(desc))
	,m_localFrame(ndGetIdentityMatrix())
	,m_chassis(nullptr)
	,m_invDynamicsSolver()
	,m_motor(nullptr)
	,m_tireShape(new ndShapeChamferCylinder(ndFloat32(0.75f), ndFloat32(0.5f)))
	,m_gearBox(nullptr)
	,m_torsionBar(nullptr)
	,m_tireList()
	,m_extraBodiesAttachmentList()
	,m_axleList()
	,m_differentialList()
	,m_extraJointsAttachmentList()
	,m_downForce()
{
	ndAssert(0);
	//m_tireShape->AddRef();
	//
	//const nd::TiXmlNode* const xmlNode = desc.m_rootNode;
	//for (const nd::TiXmlNode* node = xmlNode->FirstChild(); node; node = node->NextSibling())
	//{
	//	const char* const partName = node->Value();
	//
	//	if (strcmp(partName, "ndModel") == 0)
	//	{
	//		// do nothing
	//	}
	//	else if (strcmp(partName, "localFrame") == 0)
	//	{
	//		m_localFrame = xmlGetMatrix(xmlNode, "localFrame");
	//	} 
	//	else if (strcmp(partName, "chassis") == 0)
	//	{
	//		ndInt32 hash = xmlGetInt(xmlNode, "chassis");
	//		const ndBody* const body = desc.m_bodyMap->Find(hash)->GetInfo();
	//		m_chassis = ((ndBody*)body)->GetAsBodyDynamic();
	//	}
	//	else if (strcmp(partName, "tire") == 0)
	//	{
	//		ndInt32 hash;
	//		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
	//		element->Attribute("int32", &hash);
	//		ndMultiBodyVehicleTireJoint* const tire = (ndMultiBodyVehicleTireJoint*)desc.m_jointMap->Find(hash)->GetInfo();
	//		tire->m_vehicle = this;
	//		m_tireList.Append(tire);
	//	}
	//	else if (strcmp(partName, "diff") == 0)
	//	{
	//		ndInt32 hash;
	//		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
	//		element->Attribute("int32", &hash);
	//		ndMultiBodyVehicleDifferential* const tire = (ndMultiBodyVehicleDifferential*)desc.m_jointMap->Find(hash)->GetInfo();
	//		m_differentialList.Append(tire);
	//	}
	//	else if (strcmp(partName, "axle") == 0)
	//	{
	//		ndInt32 hash;
	//		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
	//		element->Attribute("int32", &hash);
	//		ndMultiBodyVehicleDifferentialAxle* const tire = (ndMultiBodyVehicleDifferentialAxle*)desc.m_jointMap->Find(hash)->GetInfo();
	//		m_axleList.Append(tire);
	//	}
	//	else if (strcmp(partName, "motor") == 0)
	//	{
	//		ndInt32 hash = xmlGetInt(xmlNode, "motor");
	//		m_motor = (ndMultiBodyVehicleMotor*)desc.m_jointMap->Find(hash)->GetInfo();
	//		m_motor->m_vehicelModel = this;
	//	}
	//	else if (strcmp(partName, "gearBox") == 0)
	//	{
	//		ndInt32 hash = xmlGetInt(xmlNode, "gearBox");
	//		m_gearBox = (ndMultiBodyVehicleGearBox*)desc.m_jointMap->Find(hash)->GetInfo();
	//		m_gearBox->m_chassis = this;
	//	}
	//	else if (strcmp(partName, "torsionBar") == 0)
	//	{
	//		ndInt32 hash = xmlGetInt(xmlNode, "torsionBar");
	//		m_torsionBar = (ndMultiBodyVehicleTorsionBar*)desc.m_jointMap->Find(hash)->GetInfo();
	//
	//		const nd::TiXmlNode* barNode = node->FirstChild();
	//		for (ndInt32 i = 0; i < m_torsionBar->m_axleCount; ++i)
	//		{
	//			ndInt32 bodyHash0 = xmlGetInt(barNode, "bodyHash0");
	//			ndInt32 bodyHash1 = xmlGetInt(barNode, "bodyHash1");
	//			ndBody* const body0 = (ndBody*)desc.m_bodyMap->Find(bodyHash0)->GetInfo();
	//			ndBody* const body1 = (ndBody*)desc.m_bodyMap->Find(bodyHash1)->GetInfo();
	//			m_torsionBar->m_axles[i].m_leftTire = body0->GetAsBodyKinematic();
	//			m_torsionBar->m_axles[i].m_rightTire = body1->GetAsBodyKinematic();
	//			barNode = barNode->NextSibling();
	//		}
	//	}
	//	else if (strcmp(partName, "aerodynamics") == 0)
	//	{
	//		m_downForce.Load(node);
	//	}
	//	else if (strcmp(partName, "extraBody") == 0)
	//	{
	//		ndInt32 hash;
	//		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
	//		element->Attribute("int32", &hash);
	//		const ndBody* const body = desc.m_bodyMap->Find(hash)->GetInfo();
	//		m_extraBodiesAttachmentList.Append(((ndBody*)body)->GetAsBodyDynamic());
	//	}
	//	else if (strcmp(partName, "extraJoint") == 0)
	//	{
	//		ndInt32 hash;
	//		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
	//		element->Attribute("int32", &hash);
	//		const ndJointBilateralConstraint* const joint = desc.m_jointMap->Find(hash)->GetInfo();
	//		m_extraJointsAttachmentList.Append((ndJointBilateralConstraint*)joint);
	//	}
	//	else
	//	{
	//		ndAssert(0);
	//	}
	//}
}

ndMultiBodyVehicle::~ndMultiBodyVehicle()
{
	m_tireShape->Release();
}

void ndMultiBodyVehicle::PostUpdate(ndWorld* const, ndFloat32)
{
	ApplyAligmentAndBalancing();
}

void ndMultiBodyVehicle::Update(ndWorld* const world, ndFloat32 timestep)
{
	ApplyInputs(world, timestep);

	// apply down force
	ApplyAerodynamics();
	// apply tire model
	ApplyTireModel(timestep);
}

void ndMultiBodyVehicle::Save(const ndLoadSaveBase::ndSaveDescriptor& desc) const
{
	nd::TiXmlElement* const childNode = new nd::TiXmlElement(ClassName());
	desc.m_rootNode->LinkEndChild(childNode);
	childNode->SetAttribute("hashId", desc.m_nodeNodeHash);
	ndModel::Save(ndLoadSaveBase::ndSaveDescriptor(desc, childNode));

	xmlSaveParam(childNode, "localFrame", m_localFrame);
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("chassis");
		childNode->LinkEndChild(paramNode);

		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(m_chassis);
		if (!bodyNode)
		{
			bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), m_chassis);
		}
		ndAssert(bodyNode);
		paramNode->SetAttribute("int32", bodyNode->GetInfo());
	}

	// save all wheels
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("tire");
		childNode->LinkEndChild(paramNode);

		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(node->GetInfo()->GetBody0());
		if (!bodyNode)
		{
			bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), node->GetInfo()->GetBody0());
		}

		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(node->GetInfo());
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), node->GetInfo());
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	// save all differentials
	for (ndList<ndMultiBodyVehicleDifferential*>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("diff");
		childNode->LinkEndChild(paramNode);

		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(node->GetInfo()->GetBody0());
		if (!bodyNode)
		{
			bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), node->GetInfo()->GetBody0());
		}

		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(node->GetInfo());
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), node->GetInfo());
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	// save all axles
	for (ndList<ndMultiBodyVehicleDifferentialAxle*>::ndNode* node = m_axleList.GetFirst(); node; node = node->GetNext())
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("axle");
		childNode->LinkEndChild(paramNode);
		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(node->GetInfo());
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), node->GetInfo());
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	if (m_motor)
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("motor");
		childNode->LinkEndChild(paramNode);

		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(m_motor->GetBody0());
		if (!bodyNode)
		{
			bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), m_motor->GetBody0());
		}

		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(m_motor);
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), m_motor);
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	for (ndList<ndBodyKinematic*>::ndNode* node = m_extraBodiesAttachmentList.GetFirst(); node; node = node->GetNext())
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("extraBody");
		childNode->LinkEndChild(paramNode);

		const ndBodyKinematic* const body = node->GetInfo();
		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(body);
		if (!bodyNode)
		{
			bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), body);
		}
		ndAssert(bodyNode);
		paramNode->SetAttribute("int32", bodyNode->GetInfo());
	}

	for (ndList<ndJointBilateralConstraint*>::ndNode* node = m_extraJointsAttachmentList.GetFirst(); node; node = node->GetNext())
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("extraJoint");
		childNode->LinkEndChild(paramNode);

		const ndBodyKinematic* const body0 = node->GetInfo()->GetBody0();
		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode0 = desc.m_bodyMap->Find(body0);
		if (!bodyNode0)
		{
			desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), body0);
		}

		const ndBodyKinematic* const body1 = node->GetInfo()->GetBody1();
		ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode1 = desc.m_bodyMap->Find(body1);
		if (!bodyNode1)
		{
			desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), body1);
		}

		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(node->GetInfo());
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), node->GetInfo());
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	if (m_gearBox)
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("gearBox");
		childNode->LinkEndChild(paramNode);

		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(m_gearBox);
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), m_gearBox);
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());
	}

	if (m_torsionBar)
	{
		nd::TiXmlElement* const paramNode = new nd::TiXmlElement("torsionBar");
		childNode->LinkEndChild(paramNode);

		ndAssert(m_torsionBar->GetBody1()->GetAsBodySentinel());
		ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(m_torsionBar);
		if (!jointNode)
		{
			jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), m_torsionBar);
		}
		ndAssert(jointNode);
		paramNode->SetAttribute("int32", jointNode->GetInfo());

		for (ndInt32 i = 0; i < m_torsionBar->m_axleCount; ++i)
		{
			nd::TiXmlElement* const barNode = new nd::TiXmlElement("barAxle");
			paramNode->LinkEndChild(barNode);

			ndMultiBodyVehicleTorsionBar::ndAxles& axle = m_torsionBar->m_axles[i];
			ndInt32 bodyHash0 = ndInt32(desc.m_bodyMap->Find(axle.m_leftTire)->GetInfo());
			ndInt32 bodyHash1 = ndInt32(desc.m_bodyMap->Find(axle.m_rightTire)->GetInfo());
			xmlSaveParam(barNode, "bodyHash0", bodyHash0);
			xmlSaveParam(barNode, "bodyHash1", bodyHash1);
		}
	}

	m_downForce.Save(childNode);
}

ndFloat32 ndMultiBodyVehicle::ndDownForce::CalculateFactor(const ndSpeedForcePair* const entry0) const
{
	const ndSpeedForcePair* const entry1 = entry0 + 1;
	ndFloat32 num = ndMax(entry1->m_forceFactor - entry0->m_forceFactor, ndFloat32(0.0f));
	ndFloat32 den = ndMax(ndAbs(entry1->m_speed - entry0->m_speed), ndFloat32(1.0f));
	return num / (den * den);
}

ndFloat32 ndMultiBodyVehicle::ndDownForce::GetDownforceFactor(ndFloat32 speed) const
{
	ndAssert(speed >= ndFloat32(0.0f));
	ndInt32 index = 0;
	for (ndInt32 i = sizeof(m_downForceTable) / sizeof(m_downForceTable[0]) - 1; i; i--)
	{
		if (m_downForceTable[i].m_speed <= speed)
		{
			index = i;
			break;
		}
	}

	ndFloat32 deltaSpeed = speed - m_downForceTable[index].m_speed;
	ndFloat32 downForceFactor = m_downForceTable[index].m_forceFactor + m_downForceTable[index + 1].m_aerodynamicDownforceConstant * deltaSpeed * deltaSpeed;
	return downForceFactor * m_gravity;
}

void ndMultiBodyVehicle::ndDownForce::Save(nd::TiXmlNode* const parentNode) const
{
	nd::TiXmlElement* const childNode = new nd::TiXmlElement("aerodynamics");
	parentNode->LinkEndChild(childNode);

	xmlSaveParam(childNode, "gravity", m_gravity);
	for (ndInt32 i = 0; i < ndInt32(sizeof(m_downForceTable) / sizeof(m_downForceTable[0])); ++i)
	{
		ndVector nod(m_downForceTable[i].m_speed, m_downForceTable[i].m_forceFactor, m_downForceTable[i].m_aerodynamicDownforceConstant, ndFloat32(0.0f));
		xmlSaveParam(childNode, "downforceCurve", nod);
	}
}

void ndMultiBodyVehicle::ndDownForce::Load(const nd::TiXmlNode* const xmlNode)
{
	m_gravity = xmlGetFloat(xmlNode, "gravity");
	const nd::TiXmlNode* node = xmlNode->FirstChild();
	for (ndInt32 i = 0; i < ndInt32(sizeof(m_downForceTable) / sizeof(m_downForceTable[0])); ++i)
	{
		node = node->NextSibling();
		const nd::TiXmlElement* const element = (nd::TiXmlElement*) node;
		const char* const data = element->Attribute("float3");

		ndFloat64 fx;
		ndFloat64 fy;
		ndFloat64 fz;
		sscanf(data, "%lf %lf %lf", &fx, &fy, &fz);
		m_downForceTable[i].m_speed = ndFloat32(fx);
		m_downForceTable[i].m_forceFactor = ndFloat32(fy);
		m_downForceTable[i].m_aerodynamicDownforceConstant = ndFloat32(fz);
	}
}

ndMultiBodyVehicle* ndMultiBodyVehicle::GetAsMultiBodyVehicle()
{
	return this;
}

ndFloat32 ndMultiBodyVehicle::GetSpeed() const
{
	//const ndBodyKinematic* const chassis = *m_chassis;
	const ndVector dir(m_chassis->GetMatrix().RotateVector(m_localFrame.m_front));
	const ndFloat32 speed = ndAbs(m_chassis->GetVelocity().DotProduct(dir).GetScalar());
	return speed;
}

void ndMultiBodyVehicle::AddChassis(ndSharedPtr<ndBodyKinematic>& chassis)
{
	m_chassis = *chassis;
	AddBody(chassis);
}

ndMultiBodyVehicleTireJoint* ndMultiBodyVehicle::AddAxleTire(const ndMultiBodyVehicleTireJointInfo& desc, ndSharedPtr<ndBodyKinematic>& tire, ndSharedPtr<ndBodyKinematic>& axleBody)
{
	ndMatrix tireFrame(ndGetIdentityMatrix());
	tireFrame.m_front = ndVector(0.0f, 0.0f, 1.0f, 0.0f);
	tireFrame.m_up = ndVector(0.0f, 1.0f, 0.0f, 0.0f);
	tireFrame.m_right = ndVector(-1.0f, 0.0f, 0.0f, 0.0f);
	ndMatrix matrix(tireFrame * m_localFrame * axleBody->GetMatrix());
	matrix.m_posit = tire->GetMatrix().m_posit;
	
	// make tire inertia spherical
	ndVector inertia(tire->GetMassMatrix());
	ndFloat32 maxInertia(ndMax(ndMax(inertia.m_x, inertia.m_y), inertia.m_z));
	inertia.m_x = maxInertia;
	inertia.m_y = maxInertia;
	inertia.m_z = maxInertia;
	tire->SetMassMatrix(inertia);

	AddBody(tire);
	ndMultiBodyVehicleTireJoint* const tireJoint = new ndMultiBodyVehicleTireJoint(matrix, *tire, *axleBody, desc, this);
	m_tireList.Append(tireJoint);

	ndSharedPtr<ndJointBilateralConstraint> jointPtr(tireJoint);
	AddJoint(jointPtr);
	
	tire->SetDebugMaxLinearAndAngularIntegrationStep(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(10.0f));
	return tireJoint;
}

ndMultiBodyVehicleTireJoint* ndMultiBodyVehicle::AddTire(const ndMultiBodyVehicleTireJointInfo& desc, ndSharedPtr<ndBodyKinematic>& tire)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBodyKinematic>& chassis = *FindBodyReference(m_chassis);
	return AddAxleTire(desc, tire, chassis);
}

ndBodyKinematic* ndMultiBodyVehicle::CreateInternalBodyPart(ndFloat32 mass, ndFloat32 radius) const
{
	ndShapeInstance diffCollision(new ndShapeSphere(radius));
	diffCollision.SetCollisionMode(false);

	ndBodyDynamic* const body = new ndBodyDynamic();
	ndAssert(m_chassis);
	body->SetMatrix(m_localFrame * m_chassis->GetMatrix());
	body->SetCollisionShape(diffCollision);
	body->SetMassMatrix(mass, diffCollision);
	//body->SetDebugMaxAngularIntegrationSteepAndLinearSpeed(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(100.0f));
	body->SetDebugMaxLinearAndAngularIntegrationStep(ndFloat32(2.0f * 360.0f) * ndDegreeToRad, ndFloat32(10.0f));
	return body;
}

ndMultiBodyVehicleDifferential* ndMultiBodyVehicle::AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleTireJoint* const leftTire, ndMultiBodyVehicleTireJoint* const rightTire, ndFloat32 slipOmegaLock)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBodyKinematic> differentialBody (CreateInternalBodyPart(mass, radius));
	AddBody(differentialBody);

	ndMultiBodyVehicleDifferential* const differential = new ndMultiBodyVehicleDifferential(*differentialBody, m_chassis, slipOmegaLock);
	m_differentialList.Append(differential);

	ndVector pin0(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_front));
	ndVector upPin(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_up));
	ndVector leftPin1(leftTire->GetBody0()->GetMatrix().RotateVector(leftTire->GetLocalMatrix0().m_front));

	ndMultiBodyVehicleDifferentialAxle* const leftAxle = new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, *differentialBody, leftPin1, leftTire->GetBody0());
	m_axleList.Append(leftAxle);

	ndMultiBodyVehicleDifferentialAxle* const rightAxle = new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale (ndFloat32 (-1.0f)), *differentialBody, leftPin1, rightTire->GetBody0());
	m_axleList.Append(rightAxle);

	ndSharedPtr<ndJointBilateralConstraint> jointPtr0(leftAxle);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr1(rightAxle);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr2(differential);

	AddJoint(jointPtr0);
	AddJoint(jointPtr1);
	AddJoint(jointPtr2);
	return differential;
}

ndMultiBodyVehicleDifferential* ndMultiBodyVehicle::AddDifferential(ndFloat32 mass, ndFloat32 radius, ndMultiBodyVehicleDifferential* const leftDifferential, ndMultiBodyVehicleDifferential* const rightDifferential, ndFloat32 slipOmegaLock)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBodyKinematic> differentialBody(CreateInternalBodyPart(mass, radius));
	AddBody(differentialBody);

	ndMultiBodyVehicleDifferential* const differential = new ndMultiBodyVehicleDifferential(*differentialBody, m_chassis, slipOmegaLock);
	m_differentialList.Append(differential);

	ndVector pin0(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_front));
	ndVector upPin(differentialBody->GetMatrix().RotateVector(differential->GetLocalMatrix0().m_up));
	ndVector leftPin1(leftDifferential->GetBody0()->GetMatrix().RotateVector(leftDifferential->GetLocalMatrix0().m_front));
	leftPin1 = leftPin1.Scale(ndFloat32(-1.0f));

	ndMultiBodyVehicleDifferentialAxle* const leftAxle = new ndMultiBodyVehicleDifferentialAxle(pin0, upPin, *differentialBody, leftPin1, leftDifferential->GetBody0());
	m_axleList.Append(leftAxle);

	ndMultiBodyVehicleDifferentialAxle* const rightAxle = new ndMultiBodyVehicleDifferentialAxle(pin0, upPin.Scale(ndFloat32(-1.0f)), *differentialBody, leftPin1, rightDifferential->GetBody0());
	m_axleList.Append(rightAxle);
	
	ndSharedPtr<ndJointBilateralConstraint> jointPtr0(leftAxle);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr1(rightAxle);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr2(differential);
	AddJoint(jointPtr0);
	AddJoint(jointPtr1);
	AddJoint(jointPtr2);

	return differential;
}

void ndMultiBodyVehicle::AddExtraBody(ndSharedPtr<ndBodyKinematic>& body)
{
	AddBody(body);
	m_extraBodiesAttachmentList.Append(*body);
}

void ndMultiBodyVehicle::AddExtraJoint(ndSharedPtr<ndJointBilateralConstraint>& joint)
{
	AddJoint(joint);
	m_extraJointsAttachmentList.Append(*joint);
}

ndMultiBodyVehicleMotor* ndMultiBodyVehicle::AddMotor(ndFloat32 mass, ndFloat32 radius)
{
	ndAssert(m_chassis);
	ndSharedPtr<ndBodyKinematic> motorBody (CreateInternalBodyPart(mass, radius));
	AddBody(motorBody);
	m_motor = new ndMultiBodyVehicleMotor(*motorBody, this);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr(m_motor);
	AddJoint(jointPtr);
	return m_motor;
}

ndMultiBodyVehicleGearBox* ndMultiBodyVehicle::AddGearBox(ndMultiBodyVehicleDifferential* const differential)
{
	ndAssert(m_motor);
	m_gearBox = new ndMultiBodyVehicleGearBox(m_motor->GetBody0(), differential->GetBody0(), this);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr(m_gearBox);
	AddJoint(jointPtr);

	return m_gearBox;
}

ndMultiBodyVehicleTorsionBar* ndMultiBodyVehicle::AddTorsionBar(ndBodyKinematic* const sentinel)
{
	m_torsionBar = new ndMultiBodyVehicleTorsionBar(this, sentinel);
	ndSharedPtr<ndJointBilateralConstraint> jointPtr(m_torsionBar);
	AddJoint(jointPtr);
	return m_torsionBar;
}

ndShapeInstance ndMultiBodyVehicle::CreateTireShape(ndFloat32 radius, ndFloat32 width) const
{
	ndShapeInstance tireCollision(m_tireShape);
	ndVector scale(2.0f * width, radius, radius, 0.0f);
	tireCollision.SetScale(scale);
	return tireCollision;
}

void ndMultiBodyVehicle::ApplyAerodynamics()
{
	m_downForce.m_suspensionStiffnessModifier = ndFloat32(1.0f);
	ndFloat32 gravity = m_downForce.GetDownforceFactor(GetSpeed());
	if (ndAbs (gravity) > ndFloat32(1.0e-2f))
	{
		const ndVector up(m_chassis->GetMatrix().RotateVector(m_localFrame.m_up));
		const ndVector weight(m_chassis->GetForce());
		const ndVector downForce(up.Scale(gravity * m_chassis->GetMassMatrix().m_w));
		m_chassis->SetForce(weight + downForce);
		m_downForce.m_suspensionStiffnessModifier = up.DotProduct(weight).GetScalar() / up.DotProduct(weight + downForce.Scale (0.5f)).GetScalar();
		//dTrace(("%f\n", m_suspensionStiffnessModifier));
		
		for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
		{
			ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
			ndBodyKinematic* const tireBody = tire->GetBody0();
			const ndVector tireWeight(tireBody->GetForce());
			const ndVector tireDownForce(up.Scale(gravity * tireBody->GetMassMatrix().m_w));
			tireBody->SetForce(tireWeight + tireDownForce);
		}
	}
}

void ndMultiBodyVehicle::SetVehicleSolverModel(bool hardJoint)
{
	ndJointBilateralSolverModel openLoopMode = hardJoint ? m_jointkinematicOpenLoop : m_jointIterativeSoft;
	
	ndAssert(m_chassis);
	const ndBodyKinematic::ndJointList& chassisJoints = m_chassis->GetJointList();
	for (ndBodyKinematic::ndJointList::ndNode* node = chassisJoints.GetFirst(); node; node = node->GetNext())
	{
		ndJointBilateralConstraint* const joint = node->GetInfo();
		const char* const className = joint->ClassName();
		if (!strcmp(className, "ndMultiBodyVehicleTireJoint") ||
			!strcmp(className, "ndMultiBodyVehicleDifferential") ||
			!strcmp(className, "ndMultiBodyVehicleMotor"))
		{
			joint->SetSolverModel(openLoopMode);
		}
	}
	
	ndJointBilateralSolverModel driveTrainMode = hardJoint ? m_jointkinematicCloseLoop : m_jointIterativeSoft;
	for (ndList<ndMultiBodyVehicleDifferential*>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
	{
		ndJointBilateralConstraint* const joint = node->GetInfo();
		const ndBodyKinematic::ndJointList& jointList = joint->GetBody0()->GetJointList();
		for (ndBodyKinematic::ndJointList::ndNode* node1 = jointList.GetFirst(); node1; node1 = node1->GetNext())
		{
			ndJointBilateralConstraint* const axle = node1->GetInfo();
			const char* const clasName = axle->ClassName();
			if (strcmp(clasName, "ndMultiBodyVehicleDifferential"))
			{
				axle->SetSolverModel(driveTrainMode);
			}
		}
	}
	
	if (m_torsionBar)
	{
		m_torsionBar->SetSolverModel(driveTrainMode);
	}
}

void ndMultiBodyVehicle::ApplyAligmentAndBalancing()
{
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
		ndBodyKinematic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
		ndBodyKinematic* const chassisBody = tire->GetBody1()->GetAsBodyDynamic();
	
		bool savedSleepState = tireBody->GetSleepState();
		tire->UpdateTireSteeringAngleMatrix();
		
		ndMatrix tireMatrix;
		ndMatrix chassisMatrix;
		tire->CalculateGlobalMatrix(tireMatrix, chassisMatrix);
		
		// align tire velocity
		const ndVector chassisVelocity(chassisBody->GetVelocityAtPoint(tireMatrix.m_posit));
		const ndVector relVeloc(tireBody->GetVelocity() - chassisVelocity);
		ndVector localVeloc(chassisMatrix.UnrotateVector(relVeloc));
		bool applyProjection = (localVeloc.m_x * localVeloc.m_x + localVeloc.m_z * localVeloc.m_z) > (ndFloat32(0.05f) * ndFloat32(0.05f));
		localVeloc.m_x *= ndFloat32(0.3f);
		localVeloc.m_z *= ndFloat32(0.3f);
		const ndVector tireVelocity(chassisVelocity + chassisMatrix.RotateVector(localVeloc));
		
		// align tire angular velocity
		const ndVector chassisOmega(chassisBody->GetOmega());
		const ndVector relOmega(tireBody->GetOmega() - chassisOmega);
		ndVector localOmega(chassisMatrix.UnrotateVector(relOmega));
		applyProjection = applyProjection || (localOmega.m_y * localOmega.m_y + localOmega.m_z * localOmega.m_z) > (ndFloat32(0.05f) * ndFloat32(0.05f));
		localOmega.m_y *= ndFloat32(0.3f);
		localOmega.m_z *= ndFloat32(0.3f);
		const ndVector tireOmega(chassisOmega + chassisMatrix.RotateVector(localOmega));
		
		if (applyProjection)
		{
			tireBody->SetOmega(tireOmega);
			tireBody->SetVelocity(tireVelocity);
		}
		tireBody->RestoreSleepState(savedSleepState);
	}
	
	for (ndList<ndMultiBodyVehicleDifferential*>::ndNode* node = m_differentialList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleDifferential* const diff = node->GetInfo();
		diff->AlignMatrix();
	}
	
	if (m_motor)
	{
		m_motor->AlignMatrix();
	}
}

void ndMultiBodyVehicle::Debug(ndConstraintDebugCallback& context) const
{
	// draw vehicle cordinade system;
	const ndBodyKinematic* const chassis = m_chassis;
	ndAssert(chassis);
	ndMatrix chassisMatrix(chassis->GetMatrix());
	chassisMatrix.m_posit = chassisMatrix.TransformVector(chassis->GetCentreOfMass());
	//context.DrawFrame(chassisMatrix);

	ndFloat32 totalMass = chassis->GetMassMatrix().m_w;
	ndVector effectiveCom(chassisMatrix.m_posit.Scale(totalMass));

	// draw front direction for side slip angle reference

	// draw velocity vector
	ndVector veloc(chassis->GetVelocity());
	ndVector p0(chassisMatrix.m_posit + m_localFrame.m_up.Scale(1.0f));
	ndVector p1(p0 + chassisMatrix.RotateVector(m_localFrame.m_front).Scale(2.0f));
	ndVector p2(p0 + veloc.Scale (0.25f));

	context.DrawLine(p0, p2, ndVector(1.0f, 1.0f, 0.0f, 0.0f));
	context.DrawLine(p0, p1, ndVector(1.0f, 0.0f, 0.0f, 0.0f));

	// draw body acceleration
	//ndVector accel(m_chassis->GetAccel());
	//ndVector p3(p0 + accel.Scale(0.5f));
	//context.DrawLine(p0, p3, ndVector(0.0f, 1.0f, 1.0f, 0.0f));

	ndFloat32 scale = ndFloat32 (3.0f);
	ndVector weight(chassis->GetForce().Scale (scale * chassis->GetInvMass() / m_downForce.m_gravity));

	// draw vehicle weight;
	ndVector forceColor(ndFloat32 (0.8f), ndFloat32(0.8f), ndFloat32(0.8f), ndFloat32(0.0f));
	ndVector lateralColor(ndFloat32(0.3f), ndFloat32(0.7f), ndFloat32(0.0f), ndFloat32(0.0f));
	ndVector longitudinalColor(ndFloat32(0.7f), ndFloat32(0.3f), ndFloat32(0.0f), ndFloat32(0.0f));
	context.DrawLine(chassisMatrix.m_posit, chassisMatrix.m_posit + weight, forceColor);

	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tireJoint = node->GetInfo();
		ndBodyKinematic* const tireBody = tireJoint->GetBody0()->GetAsBodyDynamic();
		ndMatrix tireFrame(tireBody->GetMatrix());
		totalMass += tireBody->GetMassMatrix().m_w;
		effectiveCom += tireFrame.m_posit.Scale(tireBody->GetMassMatrix().m_w);
	}

	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tireJoint = node->GetInfo();
		ndBodyKinematic* const tireBody = tireJoint->GetBody0()->GetAsBodyDynamic();

		// draw upper bumper
		ndMatrix upperBumberMatrix(tireJoint->CalculateUpperBumperMatrix());
		//context.DrawFrame(tireJoint->CalculateUpperBumperMatrix());

		ndMatrix tireBaseFrame(tireJoint->CalculateBaseFrame());
		//context.DrawFrame(tireBaseFrame);

		// show tire center of mass;
		ndMatrix tireFrame(tireBody->GetMatrix());
		//context.DrawFrame(tireFrame);
		upperBumberMatrix.m_posit = tireFrame.m_posit;
		//context.DrawFrame(upperBumberMatrix);

		// draw tire forces
		const ndBodyKinematic::ndContactMap& contactMap = tireBody->GetContactMap();
		ndFloat32 tireGravities = scale /(totalMass * m_downForce.m_gravity);
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				const ndContactPointList& contactPoints = contact->GetContactPoints();
				for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
				{
					const ndContactMaterial& contactPoint = contactNode->GetInfo();
					ndMatrix frame(contactPoint.m_normal, contactPoint.m_dir0, contactPoint.m_dir1, contactPoint.m_point);

					ndVector localPosit(m_localFrame.UntransformVector(chassisMatrix.UntransformVector(contactPoint.m_point)));
					ndFloat32 offset = (localPosit.m_z > ndFloat32(0.0f)) ? ndFloat32(0.2f) : ndFloat32(-0.2f);
					frame.m_posit += contactPoint.m_dir0.Scale(offset);
					frame.m_posit += contactPoint.m_normal.Scale(0.1f);

					// normal force
					ndFloat32 normalForce = -tireGravities * contactPoint.m_normal_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_normal.Scale (normalForce), forceColor);

					// lateral force
					ndFloat32 lateralForce = -tireGravities * contactPoint.m_dir0_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir0.Scale(lateralForce), lateralColor);

					// longitudinal force
					ndFloat32 longitudinalForce = tireGravities * contactPoint.m_dir1_Force.m_force;
					context.DrawLine(frame.m_posit, frame.m_posit + contactPoint.m_dir1.Scale(longitudinalForce), longitudinalColor);
				}
			}
		}
	}

	effectiveCom = effectiveCom.Scale(ndFloat32(1.0f) / totalMass);
	chassisMatrix.m_posit = effectiveCom;
	chassisMatrix.m_posit.m_w = ndFloat32(1.0f);
	context.DrawFrame(chassisMatrix);
}

ndMultiBodyVehicle::ndDownForce::ndDownForce()
	:m_gravity(ndFloat32(-10.0f))
	, m_suspensionStiffnessModifier(ndFloat32(1.0f))
{
	m_downForceTable[0].m_speed = ndFloat32(0.0f) * ndFloat32(0.27f);
	m_downForceTable[0].m_forceFactor = 0.0f;
	m_downForceTable[0].m_aerodynamicDownforceConstant = ndFloat32(0.0f);

	m_downForceTable[1].m_speed = ndFloat32(30.0f) * ndFloat32(0.27f);
	m_downForceTable[1].m_forceFactor = 1.0f;
	m_downForceTable[1].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[0]);

	m_downForceTable[2].m_speed = ndFloat32(60.0f) * ndFloat32(0.27f);
	m_downForceTable[2].m_forceFactor = 1.6f;
	m_downForceTable[2].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[1]);

	m_downForceTable[3].m_speed = ndFloat32(140.0f) * ndFloat32(0.27f);
	m_downForceTable[3].m_forceFactor = 3.0f;
	m_downForceTable[3].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[2]);

	m_downForceTable[4].m_speed = ndFloat32(1000.0f) * ndFloat32(0.27f);
	m_downForceTable[4].m_forceFactor = 3.0f;
	m_downForceTable[4].m_aerodynamicDownforceConstant = CalculateFactor(&m_downForceTable[3]);
}

//void ndMultiBodyVehicle::CoulombTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint) const
void ndMultiBodyVehicle::CoulombTireModel(ndMultiBodyVehicleTireJoint* const, ndContactMaterial& contactPoint, ndFloat32) const
{
	const ndFloat32 frictionCoefficient = contactPoint.m_material.m_staticFriction0;
	const ndFloat32 normalForce = contactPoint.m_normal_Force.GetInitialGuess() + ndFloat32(1.0f);
	const ndFloat32 maxForceForce = frictionCoefficient * normalForce;
	
	contactPoint.m_material.m_staticFriction0 = maxForceForce;
	contactPoint.m_material.m_dynamicFriction0 = maxForceForce;
	contactPoint.m_material.m_staticFriction1 = maxForceForce;
	contactPoint.m_material.m_dynamicFriction1 = maxForceForce;
	contactPoint.m_material.m_flags = contactPoint.m_material.m_flags | m_override0Friction | m_override1Friction;
}

void ndMultiBodyVehicle::CalculateNormalizedAlgningTorque(ndMultiBodyVehicleTireJoint* const tire, ndFloat32 sideSlipTangent) const
{
	//I need to calculate the integration of the align torque 
	//using the calculate contact patch, form the standard brush model.
	//for now just set the torque to zero.
	ndFloat32 angle = ndAtan(sideSlipTangent);
	ndFloat32 a = ndFloat32(0.1f);

	ndFloat32 slipCos(ndCos(angle));
	ndFloat32 slipSin(ndSin(angle));
	ndFloat32 y1 = ndFloat32(2.0f) * slipSin * slipCos;
	ndFloat32 x1 = -a + ndFloat32(2.0f) * slipCos * slipCos;

	ndVector p1(x1, y1, ndFloat32(0.0f), ndFloat32(0.0f));
	ndVector p0(-a, ndFloat32(0.0f), ndFloat32(0.0f), ndFloat32(0.0f));
	
	
	//ndFloat32 alignTorque = ndFloat32(0.0f);
	//ndFloat32 sign = ndSign(alignTorque);
	//tire->m_normalizedAligningTorque = sign * ndMax(ndAbs(alignTorque), ndAbs(tire->m_normalizedAligningTorque));
}

void ndMultiBodyVehicle::BrushTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	// calculate longitudinal slip ratio
	const ndBodyKinematic* const chassis = m_chassis;
	ndAssert(chassis);
	const ndBodyKinematic* const tireBody = tire->GetBody0()->GetAsBodyDynamic();
	const ndBodyKinematic* const otherBody = (contactPoint.m_body0 == tireBody) ? ((ndBodyKinematic*)contactPoint.m_body1)->GetAsBodyDynamic() : ((ndBodyKinematic*)contactPoint.m_body0)->GetAsBodyDynamic();
	ndAssert(tireBody != otherBody);
	ndAssert((tireBody == contactPoint.m_body0) || (tireBody == contactPoint.m_body1));

	//tire non linear brush model is only considered 
	//when is moving faster than 0.5 m/s (approximately 1.0 miles / hours) 
	//this is just an arbitrary limit, based of the model 
	//not been defined for stationary tires.
	const ndVector contactVeloc0(tireBody->GetVelocity());
	const ndVector contactVeloc1(otherBody->GetVelocityAtPoint(contactPoint.m_point));
	const ndVector relVeloc(contactVeloc0 - contactVeloc1);
	const ndVector lateralDir = contactPoint.m_dir1;
	const ndVector longitudDir = contactPoint.m_dir0;
	const ndFloat32 relSpeed = ndAbs(relVeloc.DotProduct(longitudDir).GetScalar());
	if (relSpeed > D_MAX_CONTACT_SPEED_TRESHOLD)
	{
		// tire is in breaking and traction mode.
		const ndVector contactVeloc(tireBody->GetVelocityAtPoint(contactPoint.m_point) - contactVeloc1);

		const ndVector tireVeloc(tireBody->GetVelocity());
		const ndFloat32 vr = contactVeloc.DotProduct(longitudDir).GetScalar();
		const ndFloat32 longitudialSlip = ndAbs(vr) / relSpeed;

		//const ndFloat32 sideSpeed = ndAbs(relVeloc.DotProduct(lateralDir).GetScalar());
		const ndFloat32 sideSpeed = relVeloc.DotProduct(lateralDir).GetScalar();
		const ndFloat32 signedLateralSlip = sideSpeed / (relSpeed + ndFloat32 (1.0f));
		CalculateNormalizedAlgningTorque(tire, signedLateralSlip);

		const ndFloat32 lateralSlip = ndAbs(signedLateralSlip);

		ndAssert(longitudialSlip >= ndFloat32(0.0f));

		CalculateNormalizedAlgningTorque(tire, lateralSlip);

		tire->m_lateralSlip = ndMax(tire->m_lateralSlip, lateralSlip);
		tire->m_longitudinalSlip = ndMax(tire->m_longitudinalSlip, longitudialSlip);

		const ndFloat32 den = ndFloat32(1.0f) / (longitudialSlip + ndFloat32(1.0f));
		const ndFloat32 v = lateralSlip * den;
		const ndFloat32 u = longitudialSlip * den;

		const ndTireFrictionModel& info = tire->m_frictionModel;
		const ndFloat32 vehicleMass = chassis->GetMassMatrix().m_w;
		const ndFloat32 cz = vehicleMass * info.m_laterialStiffness  * v;
		const ndFloat32 cx = vehicleMass * info.m_longitudinalStiffness  * u;

		const ndFloat32 gamma = ndMax(ndSqrt(cx * cx + cz * cz), ndFloat32(1.0e-8f));
		const ndFloat32 frictionCoefficient = contactPoint.m_material.m_staticFriction0;
		const ndFloat32 normalForce = contactPoint.m_normal_Force.GetInitialGuess() + ndFloat32(1.0f);

		const ndFloat32 maxForceForce = frictionCoefficient * normalForce;
		ndFloat32 f = maxForceForce;
		if (gamma < (ndFloat32(3.0f) * maxForceForce))
		{
			const ndFloat32 b = ndFloat32(1.0f) / (ndFloat32(3.0f) * maxForceForce);
			const ndFloat32 c = ndFloat32(1.0f) / (ndFloat32(27.0f) * maxForceForce * maxForceForce);
			f = gamma * (ndFloat32(1.0f) - b * gamma + c * gamma * gamma);
		}
	
		const ndFloat32 lateralForce = f * cz / gamma;
		const ndFloat32 longitudinalForce = f * cx / gamma;
		//ndTrace(("(%d: %f %f)  ", tireBody->GetId(), longitudinalForce, lateralForce));

		contactPoint.OverrideFriction0Accel(-vr / timestep);
		contactPoint.m_material.m_staticFriction0 = longitudinalForce;
		contactPoint.m_material.m_dynamicFriction0 = longitudinalForce;
		contactPoint.m_material.m_staticFriction1 = lateralForce;
		contactPoint.m_material.m_dynamicFriction1 = lateralForce;
		contactPoint.m_material.m_flags = contactPoint.m_material.m_flags | m_override0Friction | m_override1Friction;
	}
	else
	{
		CoulombTireModel(tire, contactPoint, timestep);
	}
}

void ndMultiBodyVehicle::PacejkaTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	BrushTireModel(tire, contactPoint, timestep);
}

void ndMultiBodyVehicle::CoulombFrictionCircleTireModel(ndMultiBodyVehicleTireJoint* const tire, ndContactMaterial& contactPoint, ndFloat32 timestep) const
{
	BrushTireModel(tire, contactPoint, timestep);
}

//void ndMultiBodyVehicle::ApplyVehicleDynamicControl(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 contactCount)
void ndMultiBodyVehicle::ApplyVehicleDynamicControl(ndFloat32, ndTireContactPair* const, ndInt32)
{
	//contactCount = 0;
	//for (ndInt32 i = contactCount - 1; i >= 0; --i)
	//{
	//	ndContact* const contact = tireContacts[i].m_contact;
	//	ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
	//	ndContactPointList& contactPoints = contact->GetContactPoints();
	//	ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
	//	tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
	//	for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
	//	{
	//		ndContactMaterial& contactPoint = contactNode->GetInfo();
	//		ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
	//		// contact are consider on the contact patch strip only if the are less than 
	//		// 45 degree angle from the tire axle
	//		if (contactPathLocation < ndFloat32(0.71f))
	//		{
	//			// align tire friction direction
	//			const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
	//			const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));
	//
	//			contactPoint.m_dir1 = lateralDir;
	//			contactPoint.m_dir0 = longitudinalDir;
	//
	//			// check if the contact is in the contact patch,
	//			// the is the 45 degree point around the tire vehicle axis. 
	//			ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
	//			ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
	//			ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
	//			if (contactPatch < ndFloat32(-0.71f))
	//			{
	//				switch (tire->m_frictionModel.m_frictionModel)
	//				{
	//					case ndTireFrictionModel::m_brushModel:
	//					{
	//						BrushTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_pacejka:
	//					{
	//						PacejkaTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_coulombCicleOfFriction:
	//					{
	//						CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//
	//					case ndTireFrictionModel::m_coulomb:
	//					default:
	//					{
	//						CoulombTireModel(tire, contactPoint, timestep);
	//						break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

#if 0
void ndMultiBodyVehicle::ApplyTireModel(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 tireCount)
{
	for (ndInt32 i = tireCount - 1; i >= 0; --i)
	{
		ndContact* const contact = tireContacts[i].m_contact;
		ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
		ndContactPointList& contactPoints = contact->GetContactPoints();
		ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
		tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
		for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
		{
			ndContactMaterial& contactPoint = contactNode->GetInfo();
			ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
			// contact are consider on the contact patch strip only if the are less than 
			// 45 degree angle from the tire axle
			if (contactPathLocation < ndFloat32(0.71f))
			{
				// align tire friction direction
				const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
				const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));

				contactPoint.m_dir1 = lateralDir;
				contactPoint.m_dir0 = longitudinalDir;

				// check if the contact is in the contact patch,
				// the is the 45 degree point around the tire vehicle axis. 
				ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
				ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
				ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
				if (contactPatch < ndFloat32(-0.71f))
				{
					switch (tire->m_frictionModel.m_frictionModel)
					{
					case ndTireFrictionModel::m_brushModel:
					{
						BrushTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_pacejka:
					{
						PacejkaTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulombCicleOfFriction:
					{
						CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulomb:
					default:
					{
						CoulombTireModel(tire, contactPoint, timestep);
						break;
					}
					}
				}
			}
		}
	}
}

#else
void ndMultiBodyVehicle::ApplyTireModel(ndFloat32 timestep, ndTireContactPair* const tireContacts, ndInt32 contactCount)
{
	ndInt32 oldCount = contactCount;
	for (ndInt32 i = contactCount - 1; i >= 0; --i)
	{
		ndContact* const contact = tireContacts[i].m_contact;
		ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
		ndContactPointList& contactPoints = contact->GetContactPoints();
		ndMatrix tireBasisMatrix(tire->GetLocalMatrix1() * tire->GetBody1()->GetMatrix());
		tireBasisMatrix.m_posit = tire->GetBody0()->GetMatrix().m_posit;
		const ndMaterial* const material = contact->GetMaterial();
		bool useCoulombModel = (material->m_flags & m_useBrushTireModel) ? false : true;
		for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
		{
			ndContactMaterial& contactPoint = contactNode->GetInfo();
			ndFloat32 contactPathLocation = ndAbs(contactPoint.m_normal.DotProduct(tireBasisMatrix.m_front).GetScalar());
			// contact are consider on the contact patch strip only if the are less than 
			// 45 degree angle from the tire axle
			if (contactPathLocation < ndFloat32(0.71f))
			{
				// align tire friction direction
				const ndVector longitudinalDir(contactPoint.m_normal.CrossProduct(tireBasisMatrix.m_front).Normalize());
				const ndVector lateralDir(longitudinalDir.CrossProduct(contactPoint.m_normal));

				contactPoint.m_dir1 = lateralDir;
				contactPoint.m_dir0 = longitudinalDir;

				// check if the contact is in the contact patch,
				// the is the 45 degree point around the tire vehicle axis. 
				ndVector dir(contactPoint.m_point - tireBasisMatrix.m_posit);
				ndAssert(dir.DotProduct(dir).GetScalar() > ndFloat32(0.0f));
				ndFloat32 contactPatch = tireBasisMatrix.m_up.DotProduct(dir.Normalize()).GetScalar();
				if (useCoulombModel || (contactPatch > ndFloat32(-0.71f)))
				{
					contactCount--;
					tireContacts[i] = tireContacts[contactCount];
				}
			}
		}
	}

	if (contactCount == oldCount)
	{
		for (ndInt32 i = contactCount - 1; i >= 0; --i)
		{
			ndContact* const contact = tireContacts[i].m_contact;
			ndMultiBodyVehicleTireJoint* const tire = tireContacts[i].m_tireJoint;
			ndContactPointList& contactPoints = contact->GetContactPoints();
			for (ndContactPointList::ndNode* contactNode = contactPoints.GetFirst(); contactNode; contactNode = contactNode->GetNext())
			{
				ndContactMaterial& contactPoint = contactNode->GetInfo();
				switch (tire->m_frictionModel.m_frictionModel)
				{
					case ndTireFrictionModel::m_brushModel:
					{
						BrushTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_pacejka:
					{
						PacejkaTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulombCicleOfFriction:
					{
						CoulombFrictionCircleTireModel(tire, contactPoint, timestep);
						break;
					}

					case ndTireFrictionModel::m_coulomb:
					default:
					{
						CoulombTireModel(tire, contactPoint, timestep);
						break;
					}
				}
			}
		}
	}
}
#endif

void ndMultiBodyVehicle::ApplyTireModel(ndFloat32 timestep)
{
	ndFixSizeArray<ndTireContactPair, 128> tireContacts;
	for (ndList<ndMultiBodyVehicleTireJoint*>::ndNode* node = m_tireList.GetFirst(); node; node = node->GetNext())
	{
		ndMultiBodyVehicleTireJoint* const tire = node->GetInfo();
		ndAssert(((ndShape*)tire->GetBody0()->GetCollisionShape().GetShape())->GetAsShapeChamferCylinder());

		tire->m_lateralSlip = ndFloat32(0.0f);
		tire->m_longitudinalSlip = ndFloat32(0.0f);
		tire->m_normalizedAligningTorque = ndFloat32(0.0f);

		const ndBodyKinematic::ndContactMap& contactMap = tire->GetBody0()->GetContactMap();
		ndBodyKinematic::ndContactMap::Iterator it(contactMap);
		for (it.Begin(); it; it++)
		{
			ndContact* const contact = *it;
			if (contact->IsActive())
			{
				ndContactPointList& contactPoints = contact->GetContactPoints();
				// for mesh collision we need to remove contact duplicates, 
				// these are contact produced by two or more polygons, 
				// that can produce two contact so are close that they can generate 
				// ill formed rows in the solver mass matrix
				for (ndContactPointList::ndNode* contactNode0 = contactPoints.GetFirst(); contactNode0; contactNode0 = contactNode0->GetNext())
				{
					const ndContactPoint& contactPoint0 = contactNode0->GetInfo();
					for (ndContactPointList::ndNode* contactNode1 = contactNode0->GetNext(); contactNode1; contactNode1 = contactNode1->GetNext())
					{
						const ndContactPoint& contactPoint1 = contactNode1->GetInfo();
						const ndVector error(contactPoint1.m_point - contactPoint0.m_point);
						ndFloat32 err2 = error.DotProduct(error).GetScalar();
						if (err2 < D_MIN_CONTACT_CLOSE_DISTANCE2)
						{
							contactPoints.Remove(contactNode1);
							break;
						}
					}
				}
				ndTireContactPair pair;
				pair.m_contact = contact;
				pair.m_tireJoint = tire;
				tireContacts.PushBack(pair);
			}
		}
	}

	//ApplyVehicleDynamicControl(timestep, &tireContacts[0], tireContacts.GetCount());
	ApplyTireModel(timestep, &tireContacts[0], tireContacts.GetCount());
}


