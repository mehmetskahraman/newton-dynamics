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
#include "ndCharacter.h"
#include "ndBodyDynamic.h"
#include "ndJointSpherical.h"
#include "ndCharacterForwardDynamicNode.h"

D_CLASS_REFLECTION_IMPLEMENT_LOADER(ndCharacterForwardDynamicNode)

ndCharacterForwardDynamicNode::ndCharacterForwardDynamicNode(const ndMatrix& matrixInGlobalSpace, ndBodyDynamic* const body, ndCharacterNode* const parent)
	:ndCharacterNode(parent)
	,m_body(body)
	,m_joint(new ndJointSpherical(matrixInGlobalSpace, body, parent->GetBody()))
{
	m_localPose = m_body->GetMatrix() * parent->GetBody()->GetMatrix().Inverse();
}

ndCharacterForwardDynamicNode::ndCharacterForwardDynamicNode(const ndCharacterLoadDescriptor& desc)
	:ndCharacterNode(desc)
{
	const nd::TiXmlNode* const xmlNode = desc.m_rootNode;

	//const char* const name = xmlGetString(xmlNode, "name");
	//SetName(name);
	ndAssert(0);
	m_localPose = xmlGetMatrix(xmlNode, "localPose");
	ndInt32 bodyHash = xmlGetInt(xmlNode, "bodyHash");
	ndInt32 jointHash = xmlGetInt(xmlNode, "jointHash");
	
	const ndBody* const body = desc.m_bodyMap->Find(bodyHash)->GetInfo();
	const ndJointBilateralConstraint* const joint = desc.m_jointMap->Find(jointHash)->GetInfo();
	m_body = (ndBodyDynamic*)body;
	m_joint = (ndJointSpherical*)joint;
}

ndCharacterForwardDynamicNode::~ndCharacterForwardDynamicNode()
{
	delete m_joint;
	delete m_body;
}

void ndCharacterForwardDynamicNode::Save(const ndCharacterSaveDescriptor& desc) const
{
	nd::TiXmlElement* const childNode = new nd::TiXmlElement(ClassName());
	desc.m_rootNode->LinkEndChild(childNode);
	childNode->SetAttribute("hashId", desc.m_limbMap->GetCount());
	ndCharacterNode::Save(ndCharacterSaveDescriptor(desc, childNode));

	ndTree<ndInt32, const ndJointBilateralConstraint*>::ndNode* jointNode = desc.m_jointMap->Find(m_joint);
	if (!jointNode)
	{
		jointNode = desc.m_jointMap->Insert(desc.m_jointMap->GetCount(), m_joint);
	}
	ndAssert(jointNode);

	ndTree<ndInt32, const ndBodyKinematic*>::ndNode* bodyNode = desc.m_bodyMap->Find(m_body);
	if (!bodyNode)
	{
		bodyNode = desc.m_bodyMap->Insert(desc.m_bodyMap->GetCount(), m_body);
	}
	ndAssert(bodyNode);

	ndAssert(0);
	//xmlSaveParam(childNode, "name", GetName().GetStr());
	xmlSaveParam(childNode, "localPose", m_localPose);
	xmlSaveParam(childNode, "bodyHash", ndInt32(bodyNode->GetInfo()));
	xmlSaveParam(childNode, "jointHash", ndInt32(jointNode->GetInfo()));
}
