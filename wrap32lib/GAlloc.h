#pragma once

#include "wrap32lib.h"

#define GALLOC_BLOCK_SIZE	65536	// 64k

namespace GAlloc 
{

class CGBlock
{
public:
	CGBlock() {
		m_nIndex = 0;
		m_pNext = NULL;
		m_buff = new BYTE[GetBlockSize()];
		ZeroMemory(m_buff, GetBlockSize());
	}
	CGBlock(const CGBlock& rhs) : m_pNext(NULL) {
		m_nIndex = rhs.m_nIndex;
		m_buff = new BYTE[GetBlockSize()];
		memcpy(m_buff, rhs.m_buff, sizeof(m_buff));
	}

	~CGBlock() {
		delete[] m_buff;
	}

	char* Alloc(const char* szText);
	void* Alloc(size_t nLen)
	{
		if (m_nIndex + nLen > GetBlockSize())
			return NULL;	// No room

		m_nIndex += nLen;
		return m_buff + m_nIndex - nLen;
	}

	bool ContinuousAlloc(const char* szText);
	bool ContinuousAlloc(char ch);
	void* ContinuousAlloc(size_t nLen);

	size_t GetContinuousSize(BOOL bText);
	int CopyToBuff(char* sz, size_t nBuffSize) const;

	size_t GetSize()					{	return m_nIndex;	}

	CGBlock* m_pNext;

	BYTE* GetOffset(size_t nOffset) {
		CGBlock* p = this;
		while (nOffset > GetBlockSize()) {
			nOffset -= GetBlockSize();
			p = p->m_pNext;
		}

		return p->m_buff + nOffset;
	}

	static size_t GetBlockSize() { return m_blockSize;	}
	static void SetBlockSize(size_t b) { m_blockSize = b;	}

protected:
	size_t m_nIndex;
	static size_t m_blockSize;
	BYTE* m_buff;
};

class CGAlloc
{
public:
	CGAlloc() : m_pBlocks(NULL), m_pCurrent(NULL) {
		Init();
	}

	~CGAlloc() {
		FreeAll();
	}

	void Init() {
		FreeAll();
		m_pBlocks = m_pCurrent = new CGBlock();
	}

	CGAlloc& operator=(const CGAlloc& rhs);

	char* Alloc(const char* szText);
	void* Alloc(size_t nLen);
	void ContinuousAlloc(const char* szText);
	void ContinuousAlloc(char ch);
	void* ContinuousAlloc(size_t nLen);

	char* AllocACopy(size_t* pLen = NULL) const;
	int CopyToBuff(char* sz, size_t nBuffSize) const;

	bool IsEmpty() const	{	return m_pBlocks == NULL;	}

	// If text, add one to put the terminator back on
	size_t GetContinuousSize(BOOL bText) const	{	return 	(m_pBlocks ? m_pBlocks->GetContinuousSize(bText) : 0) + (bText ? sizeof(char) : 0);		}
	BYTE* GetOffset(size_t nOffset) { return m_pBlocks ? m_pBlocks->GetOffset(nOffset) : NULL; }

protected:
	void FreeAll(void);

protected:
	CGBlock* m_pBlocks;
	CGBlock* m_pCurrent;
};

}

using namespace GAlloc;
