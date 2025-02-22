/* Copyright (c) <2003-2021> <Julio Jerez, Newton Game Dynamics>
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

#include "ndCudaStdafx.h"
#include "ndCudaUtils.h"
#include "ndCudaDevice.h"

ndCudaDevice::ndCudaDevice()
{
	cudaError_t cudaStatus;
	cudaStatus = cudaGetDeviceProperties(&m_prop, 0);
	ndAssert(cudaStatus == cudaSuccess);
	
	cudaStatus = cudaSetDevice(0);
	ndAssert(cudaStatus == cudaSuccess);
	if (cudaStatus != cudaSuccess)
	{
		ndAssert(0);
	}
	m_valid = (cudaStatus == cudaSuccess);
	
	cuTrace(("gpu: %s\n", m_prop.name));
	cuTrace(("compute capability: %d.%d\n", m_prop.major, m_prop.minor));
	
	cuTrace(("warp size: %d\n", m_prop.warpSize));
	cuTrace(("multiprocessors: %d\n", m_prop.multiProcessorCount));
	cuTrace(("threads per blocks %d\n", m_prop.maxThreadsPerBlock));
	cuTrace(("blocks per multiprocessors %d\n", m_prop.maxBlocksPerMultiProcessor));
	cuTrace(("memory bus with: %d bits\n", m_prop.memoryBusWidth));
	cuTrace(("memory: (mbytes) %d\n", m_prop.totalGlobalMem / (1024 * 1024)));
	
	m_frequency = m_prop.clockRate * 1000;
	m_blocksPerKernelCall = m_prop.maxBlocksPerMultiProcessor * m_prop.multiProcessorCount;
}

ndCudaDevice::~ndCudaDevice()
{
	cudaError_t cudaStatus;
	cudaStatus = cudaDeviceReset();
	ndAssert(cudaStatus == cudaSuccess);
	
	if (cudaStatus != cudaSuccess)
	{
		ndAssert(0);
	}
}

