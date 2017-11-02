////////////////////////////////////////////////
// File:	SKServerConnection.cpp
//
// Author:	Liam Russell (A.K.A. BitSlapper)
//
// Copyright:	2014-2015 Liam Russell
//
// License:	GNU GENERAL PUBLIC LICENSE V3
//////////////////////////////////////////////

#include "SKServerConnection.h"
#include "../zcore/bignum.h"
#include "../gpu/BaseGPU.h"
#include "../config/GPUSetting.h"
#include "../threads/SKMinerThread.h"
#include "../data/SKMinerData.h"
#include "../kernel/KernelFuncs.h"
#include "../compute/CLFuncs.h"

SKServerConnection::SKServerConnection() : ServerConnection()
{

}

SKServerConnection::SKServerConnection(const SKServerConnection& skServerConnection) : ServerConnection(skServerConnection)
{
	std::vector<GPUData*> vecGPUs = skServerConnection.GetGPUs();
	for (size_t index = 0; index < vecGPUs.size(); ++index)
	{
		this->m_vecGPUs.push_back(vecGPUs[index]);
	}
}

SKServerConnection& SKServerConnection::operator=(const SKServerConnection& serverConnection)
{
	std::vector<GPUData*> vecGPUs = serverConnection.GetGPUs();
	for (size_t index = 0; index < vecGPUs.size(); ++index)
	{
		this->m_vecGPUs.push_back(vecGPUs[index]);
	}

	return *this;
}

SKServerConnection::SKServerConnection(std::vector<GPUData*> gpus, std::string ip, std::string port, std::string login, int nMaxTimeout)
{
	this->m_szIP = ip;
	this->m_szPORT = port;
	this->m_szLogin = login;

	m_thTHREAD = boost::thread(&SKServerConnection::ServerThread, this);

	m_nThreads = 0;
	for (int nIndex = 0; nIndex < gpus.size(); ++nIndex)
	{
		if (!gpus[nIndex]->GetGPU()->GetGPUSetting()->GetIsEnabled())
		{
			continue;
		}

		MinerData* pMinerData = new SKMinerData();
		pMinerData->SetGPUData(gpus[nIndex]);
		pMinerData->SetBlock(NULL);
		((SKMinerData*)pMinerData)->SetTarget(new CBigNum());

		MinerThread* pThread = new SKMinerThread((SKMinerData*)pMinerData);
		pThread->SetHashFunc(sk1024_kernel_djm2);
		m_vecTHREADS.push_back(pThread);

		int currThreads = gpus[nIndex]->GetGPU()->GetGPUSetting()->GetThreads();
		if (currThreads > 1)
		{
			for (int gpuThreads = 1; gpuThreads < currThreads; ++gpuThreads)
			{
				MinerData* pMinerData = new SKMinerData();

				GPUData* pGPUData = CreateNewOpenCLDevice(gpus[nIndex]);
				pMinerData->SetGPUData(pGPUData);
				pMinerData->SetBlock(NULL);
				((SKMinerData*)pMinerData)->SetTarget(new CBigNum());

				MinerThread* pThread = new SKMinerThread((SKMinerData*)pMinerData);
				pThread->SetHashFunc(sk1024_kernel_djm2);
				m_vecTHREADS.push_back(pThread);
			}
		}
	}

	m_vecGPUs = gpus;
	m_nTimeout = nMaxTimeout;

	m_nThreads = m_vecTHREADS.size();
}

SKServerConnection::~SKServerConnection()
{

	for (size_t gpuIndex = 0; gpuIndex < m_vecGPUs.size(); ++gpuIndex)
	{
		GPUData* pGPUData = m_vecGPUs[gpuIndex];
		if (pGPUData)
		{
			delete(pGPUData);
			pGPUData = NULL;
		}
	}

	m_vecGPUs.clear();
	ServerConnection::DelServerConnection();

}

//Main Connection Thread. Handles all the networking to allow
//Mining threads the most performance.
void SKServerConnection::ServerThread()
{
	/** Don't begin until all mining threads are Created. **/
	while ((m_vecTHREADS.size() != m_nThreads) && !m_bIsShutDown)
		Sleep(1000);

	/** Initialize the Server Connection. **/
	m_pCLIENT = new LLP::Miner(m_szIP, m_szPORT);

	/** Initialize a Timer for the Hash Meter. **/
	m_tTIMER.Start();

	unsigned int nBestHeight = 0;
	loop
	{
		if (m_bIsShutDown)
		{
			break;
		}

		try
		{

			Core::PoolWork *pWork = 0;

			/** Attempt with best efforts to keep the Connection Alive. **/
			if (!m_pCLIENT->Connected() || m_pCLIENT->Errors())
			{
				ResetThreads();
				try
				{
					if (!m_pCLIENT->Connect())
						throw std::runtime_error("Failed to reconnect, retrying...");
					else
					{
						if (m_pCLIENT->Login(m_szLogin, 5))
						{
							printf("Logged In Successfully...\n");
							pWork = m_pCLIENT->WaitWorkUpdate(1);
						}
						else
						{
							m_pCLIENT->Disconnect();
							printf("Failed to Log In...\n");
							throw std::runtime_error("Failed to Log In...");
						}
					}
				}
				catch (std::exception &e)
				{
					Sleep(5000);
					continue;
				}
				
			}
			else {
				Sleep(1000);

				if (pWork = m_pCLIENT->WaitWorkUpdate(1))
					ResetThreads();
			}

			/** Rudimentary Meter **/
			if (m_tTIMER.Elapsed() > 10)
			{
				unsigned int nElapsed = m_tTIMER.ElapsedMilliseconds();
				unsigned int nHashes = Hashes();

				double KHASH = (double)nHashes / nElapsed / 1000;
				printf("[METERS] %u Hashes | %f MHash/s | Height %u\n", nHashes, KHASH, nBestHeight);

				m_tTIMER.Reset();

				if (!m_pCLIENT->Ping(5))
				{
					printf("Connection lost, reconnecting...\n");
					m_pCLIENT->Disconnect();
					continue;
				}
			}


			/** Check if there is work to do for each Miner Thread. **/
			for (int nIndex = 0; nIndex < m_vecTHREADS.size(); nIndex++)
			{
				if (m_bIsShutDown)
				{
					break;
				}

				if (!m_vecTHREADS[nIndex]->GetMinerData()->GetGPUData()->GetGPU()->GetGPUSetting()->GetIsEnabled())
				{
					continue;
				}				

				/** Attempt to get a new block from the Server if Thread needs One. **/
				if (m_vecTHREADS[nIndex]->GetIsNewBlock())
				{
					/** Delete the Block Pointer if it Exists. **/
					Core::CBlock* pBlock = m_vecTHREADS[nIndex]->GetMinerData()->GetBlock();

					if (pBlock != NULL)
					{
						delete(pBlock);
						pBlock = NULL;
					}

					/** Retrieve new block from Server. **/
					if(!pWork)
						pWork = m_pCLIENT->RequestWork(5);

					/** If the block is good, tell the Mining Thread its okay to Mine. **/
					if (pWork)
					{
						pBlock = pWork->m_pBLOCK;
						m_vecTHREADS[nIndex]->SetIsNewBlock(false);
						m_vecTHREADS[nIndex]->SetIsBlockFound(false);
						m_vecTHREADS[nIndex]->GetMinerData()->SetBits(pWork->m_unBits);
						m_vecTHREADS[nIndex]->GetMinerData()->SetBlock(pBlock);

						nBestHeight = m_vecTHREADS[nIndex]->GetMinerData()->GetBlock()->GetHeight();

						printf("=================================\n"
							"Miner data updated\nheight: %u\ndiff: %f\n"
							"=================================\n",
							m_vecTHREADS[nIndex]->GetMinerData()->GetBlock()->GetHeight(),
							Core::GetDifficulty(m_vecTHREADS[nIndex]->GetMinerData()->GetBits()));

						delete pWork;
						pWork = 0;
					}
					/** If the Block didn't come in properly, Reconnect to the Server. **/
					else
					{
						m_pCLIENT->Disconnect();
					}

				}
				/** Submit a block from Mining Thread if Flagged. **/
				else if (m_vecTHREADS[nIndex]->GetIsBlockFound())
				{
					/** Attempt to Submit the Block to Network. **/

					unsigned char RESPONSE = m_pCLIENT->SubmitBlock(m_vecTHREADS[nIndex]->GetMinerData()->GetBlock()->GetMerkleRoot(), m_vecTHREADS[nIndex]->GetMinerData()->GetBlock()->GetNonce(), 30);
					printf("[MASTER] Hash Found on Miner Thread %i\n", nIndex);

					unsigned long long truc = m_vecTHREADS[nIndex]->GetMinerData()->GetBlock()->GetNonce();
					printf("[MASTER] nonce %08x %08x\n", (uint32_t)(truc >> 32), (uint32_t)(truc & 0xFFFFFFFFULL));

					Core::CBlock* pBlock = m_vecTHREADS[nIndex]->GetMinerData()->GetBlock();

					pBlock->SetNonce(truc + 1);

					m_vecTHREADS[nIndex]->GetMinerData()->SetBlock(pBlock);

					/** Check the Response from the Server.**/
					if (RESPONSE == 200)
					{
						printf("[MASTER] Share Accepted By Pool.\n");

						m_vecTHREADS[nIndex]->SetIsBlockFound(false);
					}
					else if (RESPONSE == 201)
					{
						printf("[MASTER] Share Rejected by Pool.\n");

						m_vecTHREADS[nIndex]->SetIsNewBlock(true);
						m_vecTHREADS[nIndex]->SetIsBlockFound(false);
					}
					/** If the Response was Incomplete, Reconnect to Server and try to Submit Block Again. **/
					else
					{
						printf("[MASTER] Failure to Submit Share. Reconnecting...\n");
						m_pCLIENT->Disconnect();
					}

					break;                           
				}
			}
		}
		catch (std::exception& e)
		{ 
			std::cout << e.what() << std::endl;
		}
	}

	if (m_bIsShutDown)
	{
		for (size_t index = 0; index < m_vecTHREADS.size(); ++index)
		{
			m_vecTHREADS[index]->SetIsShuttingDown(true);

			while (!m_vecTHREADS[index]->GetDidShutDown()){}
		}

		m_bDidShutDown = true;
	}
}
