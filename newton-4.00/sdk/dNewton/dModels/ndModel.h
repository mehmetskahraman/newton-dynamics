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

#ifndef __ND_MODEL_H__
#define __ND_MODEL_H__

#include "ndNewtonStdafx.h"
#include "ndModelList.h"

class ndMultiBodyVehicle;
class ndConstraintDebugCallback;

D_MSV_NEWTON_ALIGN_32
class ndModel: public ndContainersFreeListAlloc<ndModel>
{
	template<class T>
	class ndReferencedObjects : public ndList<ndSharedPtr<T>, ndContainersFreeListAlloc<ndSharedPtr<T>*>>
	{
		public:
		ndReferencedObjects();
		void AddReferenceBody(ndSharedPtr<T>& object);
		ndSharedPtr<T>* FindReference(const T* const object) const;
	};

	public:
	D_CLASS_REFLECTION(ndModel);
	ndModel();
	D_NEWTON_API ndModel(const ndLoadSaveBase::ndLoadDescriptor& desc);
	virtual ~ndModel ();


	D_NEWTON_API virtual void AddBody(ndSharedPtr<ndBodyKinematic>& body);
	D_NEWTON_API virtual void AddJoint(ndSharedPtr<ndJointBilateralConstraint>& joint);

	virtual ndModel* GetAsModel();
	virtual ndMultiBodyVehicle* GetAsMultiBodyVehicle();
	virtual void Debug(ndConstraintDebugCallback& context) const;

	protected:
	D_NEWTON_API virtual void AddToWorld(ndWorld* const world);
	D_NEWTON_API virtual void RemoveFromToWorld();
	D_NEWTON_API virtual void Save(const ndLoadSaveBase::ndSaveDescriptor& desc) const;
	ndSharedPtr<ndBodyKinematic>* FindBodyReference(const ndBodyKinematic* const body) const;
	ndSharedPtr<ndJointBilateralConstraint>* FindJointReference(const ndJointBilateralConstraint* const joint) const;

	virtual void Update(ndWorld* const world, ndFloat32 timestep);
	virtual void PostUpdate(ndWorld* const world, ndFloat32 timestep);
	virtual void PostTransformUpdate(ndWorld* const world, ndFloat32 timestep);

	private:
	ndReferencedObjects<ndBodyKinematic> m_referencedBodies;
	ndReferencedObjects<ndJointBilateralConstraint> m_referencedJoints;
	ndModelList::ndNode* m_node;
	ndWorld* m_world;
	ndInt8 m_markedForRemoved;

	friend class ndWorld;
	friend class ndLoadSave;
	friend class ndModelList;
} D_GCC_NEWTON_ALIGN_32;

template<class T>
ndModel::ndReferencedObjects<T>::ndReferencedObjects()
	:ndList<ndSharedPtr<T>, ndContainersFreeListAlloc<ndSharedPtr<T>*>>()
{
}

template<class T>
void ndModel::ndReferencedObjects<T>::AddReferenceBody(ndSharedPtr<T>& object)
{
	T* const obj = *object;

	for (typename ndReferencedObjects<T>::ndNode* node = ndReferencedObjects<T>::GetFirst(); node; node = node->GetNext())
	{
		if (*node->GetInfo() == obj)
		{
			return;
		}
	}
	ndReferencedObjects<T>::Append(object);
}

template<class T>
ndSharedPtr<T>* ndModel::ndReferencedObjects<T>::FindReference(const T* const object) const
{
	for (typename ndReferencedObjects<T>::ndNode* node = ndReferencedObjects<T>::GetFirst(); node; node = node->GetNext())
	{
		if (*node->GetInfo() == object)
		{
			return &node->GetInfo();
		}
	}
	return nullptr;
}

inline ndModel::ndModel()
	:ndContainersFreeListAlloc<ndModel>()
	,m_referencedBodies()
	,m_node(nullptr)
	,m_world(nullptr)
	,m_markedForRemoved(0)
{
}

inline ndModel::~ndModel()
{
	ndAssert(!m_node);
}

inline ndModel* ndModel::GetAsModel()
{ 
	return this; 
}

inline ndMultiBodyVehicle* ndModel::GetAsMultiBodyVehicle()
{ 
	return nullptr; 
}

inline void ndModel::Debug(ndConstraintDebugCallback&) const
{
}

inline void ndModel::Update(ndWorld* const, ndFloat32)
{
}

inline void ndModel::PostUpdate(ndWorld* const, ndFloat32)
{
}

inline void ndModel::PostTransformUpdate(ndWorld* const, ndFloat32)
{
}

inline ndSharedPtr<ndBodyKinematic>* ndModel::FindBodyReference(const ndBodyKinematic* const body) const
{
	return m_referencedBodies.FindReference(body);
}

inline ndSharedPtr<ndJointBilateralConstraint>* ndModel::FindJointReference(const ndJointBilateralConstraint* const joint) const
{
	return m_referencedJoints.FindReference(joint);
}

#endif 


