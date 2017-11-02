////////////////////////////////////////////////
// File:	SKMinerThread.cpp
//
// Author:	Liam Russell (A.K.A. BitSlapper)
//
// Copyright:	2014-2015 Liam Russell
//
// License:	GNU GENERAL PUBLIC LICENSE V3
//////////////////////////////////////////////

#include "SKMinerThread.h"

#include "../kernel/KernelFuncs.h"
#include "../gpu/BaseGPU.h"
#include "../zcore/util.h"
#include "../data/MinerData.h"
#include "../data/SKMinerData.h"
#include "../zcore/bignum.h"
#include "../compute/CLDevice.h"
#include "../compute/CLKernel.h"
#include "../compute/CLMemory.h"

SKMinerThread::SKMinerThread() : MinerThread()
{

}

SKMinerThread::SKMinerThread(SKMinerData* pData) : MinerThread(pData)
{
	if (m_pTHREAD)
	{
		delete(m_pTHREAD);
	}
	m_pTHREAD = new boost::thread(&SKMinerThread::Miner, this);
}

SKMinerThread::SKMinerThread(const SKMinerThread& miner) : MinerThread(miner)
{
	if (m_pTHREAD)
	{
		delete(m_pTHREAD);
	}
	m_pTHREAD = new boost::thread(&SKMinerThread::Miner, this);
}

SKMinerThread& SKMinerThread::operator=(const SKMinerThread& miner)
{
	m_pMinerData = miner.GetMinerData();
	m_bBlockFound = miner.GetIsBlockFound();
	m_bNewBlock = miner.GetIsNewBlock();
	m_bReady = miner.GetIsReady();
	m_unHashes = miner.GetHashes();
	m_pTHREAD = new boost::thread(&SKMinerThread::Miner, this);
	m_bShutown = miner.GetIsShuttingDown();
	m_bDidShutDown = miner.GetDidShutDown();

	return *this;
}

SKMinerThread::~SKMinerThread()
{
	MinerThread::DelMinerThread();
}


void SKMinerThread::Initialize()
{
	////////////////////////////

	cl_int error = CL_SUCCESS;

	CLDevice* pDevice = (CLDevice*)((SKMinerData*)m_pMinerData)->GetGPUData()->GetDevice();
	
	std::string strKernelName = "sk1024";
#ifdef _WIN32
	//force terminating null byte. otherwise fails to find kernel on windows
	strKernelName.resize(7);
#endif

	CLKernel* pKernel = pDevice->GetKernel(strKernelName);

	cl_context TheContext = pDevice->GetContext()();

	// nonce candidate

	cl_mem NonceBuffer = clCreateBuffer(TheContext, CL_MEM_READ_WRITE, sizeof(cl_ulong), NULL, &error);

	CLMemory* pNonceMemory = new CLMemory();
	pNonceMemory->SetBufferName("NonceBuffer");
	pNonceMemory->SetBufferSize(sizeof(cl_ulong));
	pNonceMemory->SetBuffer(NonceBuffer);
	pNonceMemory->SetBufferType(CL_MEM_READ_WRITE);

	unsigned long long* pEmptyNonce = (unsigned long long*)calloc(sizeof(unsigned long long), 1);
	pNonceMemory->SetEmptyBuffer((void*)pEmptyNonce);

	// input data msg
	cl_mem DataMsg = clCreateBuffer(TheContext, CL_MEM_READ_ONLY, sizeof(cl_ulong)* 27, NULL, &error);
	CLMemory* pDataMsgMemory = new CLMemory();
	pDataMsgMemory->SetBufferName("DataMsg");
	pDataMsgMemory->SetBufferSize(sizeof(cl_ulong)* 27);
	pDataMsgMemory->SetBuffer(DataMsg);
	pDataMsgMemory->SetBufferType(CL_MEM_READ_ONLY);

	// communication between kernel (not used in current one kernel setting)
	//cl_mem HashBuffer; 

	// h state after 1st Skein loop
	cl_mem HvBuffer = clCreateBuffer(TheContext, CL_MEM_READ_ONLY, sizeof(cl_ulong)* 17, NULL, &error);
	CLMemory* pHvBufferMemory = new CLMemory();
	pHvBufferMemory->SetBufferName("HvBuffer");
	pHvBufferMemory->SetBufferSize(sizeof(cl_ulong)* 17);
	pHvBufferMemory->SetBuffer(HvBuffer);
	pHvBufferMemory->SetBufferType(CL_MEM_READ_ONLY);

	pKernel->SetMemoryBuffer("NonceBuffer", pNonceMemory);
	pKernel->SetMemoryBuffer("DataMsg", pDataMsgMemory);
	pKernel->SetMemoryBuffer("HvBuffer", pHvBufferMemory);
}

void SKMinerThread::Miner()
{
	Initialize();

	loop
	{
		try
		{
			/** Don't mine if the Connection Thread is Submitting Block or Receiving New. **/
			while ((m_bNewBlock || m_bBlockFound || !m_pMinerData->GetBlock()) && !this->m_bShutown)
			{
				Sleep(10);
			}
			

			if (m_bShutown)
			{
				break;
			}

			/** Set the Target from nBits. **/	
			((SKMinerData*)m_pMinerData)->GetTarget()->SetCompact(m_pMinerData->GetBits());

			while (!m_bNewBlock && !m_bShutown)
			{

				/*m_pKernelThread = new boost::thread(m_hashFunPtr, m_pMinerData);
				m_pKernelThread->join();
	
				m_pKernelThread->detach();
				delete(m_pKernelThread);
				m_pKernelThread = NULL;*/

				bool bIsFound = false;
				Lock();
				{
					bIsFound = m_hashFunPtr(m_pMinerData);
				}
				Unlock();
			
				m_unHashes += m_pMinerData->GetGPUData()->GetHashes();				

				if (bIsFound)
				{
					m_bBlockFound = true;
					break;
				}

				if (m_pMinerData->GetBlock()->GetNonce() >= MAX_THREADS)
				{
					m_bNewBlock = true;
					break;
				}		

			}

			if (m_bShutown)
			{
				m_bDidShutDown = true;
				break;
			}

		}
		catch (std::exception& e)
		{
			printf("ERROR: %s\n", e.what());
		}
	}
}
