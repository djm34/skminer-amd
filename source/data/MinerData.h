////////////////////////////////////////////////
// File:	MinerData.h
//
// Author:	Liam Russell (A.K.A. BitSlapper)
//
// Copyright: 2014-2015 Liam Russell
//
// License:	GNU GENERAL PUBLIC LICENSE V3
//////////////////////////////////////////////

#ifndef _MINERDATA_H
#define _MINERDATA_H

#include <string>

#include "../base/IPrototype.h"

namespace Core
{
	class CBlock;
}

class GPUData;

class MinerData : public IPrototype
{

protected:

	Core::CBlock* m_pBLOCK;
	GPUData* m_pGPUData;	

public:

	///////////////////////////////////////////////////////////////////////////////
	//Constructor
	///////////////////////////////////////////////////////////////////////////////
	MinerData();

	///////////////////////////////////////////////////////////////////////////////
	//Copy Constructor
	///////////////////////////////////////////////////////////////////////////////
	MinerData(const MinerData& minerData);

	///////////////////////////////////////////////////////////////////////////////
	//Assignment Operator
	///////////////////////////////////////////////////////////////////////////////
	MinerData& operator=(const MinerData& minerData);

	///////////////////////////////////////////////////////////////////////////////
	//Destructor
	///////////////////////////////////////////////////////////////////////////////
	void DelMinerData();

	MinerData* Clone();
	MinerData* DeepCopy();

	///////////////////////////////////////////////////////////////////////////////
	//Accessors
	///////////////////////////////////////////////////////////////////////////////
	Core::CBlock*		GetBlock()		const		{ return this->m_pBLOCK;			}
	GPUData*			GetGPUData()	const		{ return this->m_pGPUData;			}

	///////////////////////////////////////////////////////////////////////////////
	//Mutators
	///////////////////////////////////////////////////////////////////////////////
	void	SetBlock(Core::CBlock* pBlock)			{   this->m_pBLOCK = pBlock;		}
	void	SetGPUData(GPUData* pGPUData)			{	this->m_pGPUData = pGPUData;	}

};


#endif //_MINERDATA_H
