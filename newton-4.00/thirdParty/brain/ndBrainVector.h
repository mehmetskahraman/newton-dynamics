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

#ifndef _ND_BRAIN_VECTOR_H__
#define _ND_BRAIN_VECTOR_H__

#include "ndBrainStdafx.h"

class ndBrainVector: public ndArray<ndReal> 
{
	public: 
	ndBrainVector();
	~ndBrainVector();
	
	void InitGaussianWeights(ndReal mean, ndReal variance);

	void Set(ndReal value);
	void Set(const ndBrainVector& data);

	ndInt32 GetMaxIndex() const;

	ndReal Dot(const ndBrainVector& a) const;
	void ScaleSet(const ndBrainVector& a, ndReal scale);
	void Add(const ndBrainVector& a, const ndBrainVector& b);
	void Sub(const ndBrainVector& a, const ndBrainVector& b);
	void Mul(const ndBrainVector& a, const ndBrainVector& b);

	void ScaleAdd(const ndBrainVector& a, ndReal b);
	void MulAdd(const ndBrainVector& a, const ndBrainVector& b);
	void MulSub(const ndBrainVector& a, const ndBrainVector& b);
};

class ndDeepBrainMemVector : public ndBrainVector
{
	public:
	ndDeepBrainMemVector();
	ndDeepBrainMemVector(const ndReal* const mem, ndInt32 size);
	~ndDeepBrainMemVector();

	void SetSize(ndInt32 size);
	void SetPointer(ndReal* const memmory);
};

#endif 
