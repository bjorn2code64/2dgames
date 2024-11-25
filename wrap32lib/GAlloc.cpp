#include "GAlloc.h"

namespace GAlloc 
{

	size_t CGBlock::m_blockSize = GALLOC_BLOCK_SIZE;

char* CGBlock::Alloc(const char* szText)
{
	size_t nLen = sizeof(char) * (strlen(szText) + 1);	// for '\0'
	if (m_nIndex + nLen > GetBlockSize())
		return NULL;	// No room

	strcpy_s((char*)(m_buff + m_nIndex), GetBlockSize() - m_nIndex, szText);
	m_nIndex += nLen;
	return (char*)(m_buff + m_nIndex - nLen);
}


bool CGBlock::ContinuousAlloc(const char* szText)
{
	size_t nLen = strlen(szText) + 1;	// for '\0'

	if (m_nIndex == 0)
	{
		if (m_nIndex + nLen > GetBlockSize())
			return false;	// No room
	}
	else
	{
		if (m_nIndex + nLen - 1 > GetBlockSize())
			return false;	// No room

		m_nIndex -= 1;	// chop the last '\0'
	}

	strcpy_s((char*)(m_buff + m_nIndex), GetBlockSize() - m_nIndex, szText);
	m_nIndex += nLen;
	return true;
}

bool CGBlock::ContinuousAlloc(char ch)
{
	int nLen = 2 * sizeof(char);	// for '\0'

	if (m_nIndex == 0)
	{
		if (m_nIndex + nLen > GetBlockSize())
			return false;	// No room
	}
	else
	{
		if (m_nIndex + nLen - 1 > GetBlockSize())
			return false;	// No room

		m_nIndex -= sizeof(char);	// chop the last '\0'
	}

	*(char*)(m_buff + m_nIndex) = ch;
	m_nIndex += sizeof(char);
	*(char*)(m_buff + m_nIndex) = '\0';
	m_nIndex += sizeof(char);
	return true;
}

void* CGBlock::ContinuousAlloc(size_t nLen)
{
	if (m_nIndex + nLen > GetBlockSize())
		return NULL;	// No room

	void* p = m_buff + m_nIndex;
	m_nIndex += nLen;
	return p;
}

size_t CGBlock::GetContinuousSize(BOOL bText)
{
	// Strip the \0 if it's text
	return m_nIndex - (bText ? sizeof(char) : 0) + (m_pNext ? m_pNext->GetContinuousSize(bText) : 0);	// take one off for '\0'
}

int CGBlock::CopyToBuff(char* sz, size_t nBuffSize) const
{
	int nLen = 0;

	// Linked list is reversed. Do m_pNext and append
	if (m_pNext)
	{
		nLen = m_pNext->CopyToBuff(sz, nBuffSize);
		if (nLen == -1)
			return -1;
	}

	nBuffSize -= nLen;

	if (nBuffSize < m_nIndex)	// eek. no room!
		return -1;

	memcpy(sz + nLen, m_buff, m_nIndex - 1);
	return static_cast<int>(nLen + m_nIndex - 1);	// return how big the buffer is
}

CGAlloc& CGAlloc::operator=(const CGAlloc& rhs)
{
	FreeAll();
	m_pCurrent = m_pBlocks = new CGBlock();
	for (CGBlock* p = rhs.m_pBlocks; p; p = p->m_pNext)
	{
		CGBlock* pNew = new CGBlock(*p);
		pNew->m_pNext = NULL;

		// Hook it on the end
		m_pCurrent->m_pNext = pNew;

		m_pCurrent = pNew;
	}

	return *this;
}

char* CGAlloc::Alloc(const char* szText)
{
	char* szRet = m_pCurrent->Alloc(szText);
	if (!szRet)
	{
		CGBlock* pNew = new CGBlock;
		m_pCurrent->m_pNext = pNew;
		m_pCurrent = pNew;
		szRet = m_pCurrent->Alloc(szText);
	}

	return szRet;
}

void* CGAlloc::Alloc(size_t nLen)
{
	void* szRet = m_pCurrent->Alloc(nLen);
	if (!szRet)
	{
		CGBlock* pNew = new CGBlock;
		m_pCurrent->m_pNext = pNew;
		m_pCurrent = pNew;
		szRet = m_pCurrent->Alloc(nLen);
	}

	return szRet;
}

void CGAlloc::ContinuousAlloc(char ch)
{
	if (!m_pCurrent->ContinuousAlloc(ch))	// full
	{
		CGBlock* pNew = new CGBlock;
		m_pCurrent->m_pNext = pNew;
		m_pCurrent = pNew;
		m_pCurrent->ContinuousAlloc(ch);
	}
}

void CGAlloc::ContinuousAlloc(const char* szText)
{
	if (!m_pCurrent->ContinuousAlloc(szText))	// full
	{
		CGBlock* pNew = new CGBlock;
		m_pCurrent->m_pNext = pNew;
		m_pCurrent = pNew;
		m_pCurrent->ContinuousAlloc(szText);
	}
}

void* CGAlloc::ContinuousAlloc(size_t nLen)
{
	void* p = m_pCurrent->ContinuousAlloc(nLen);
	if (!p)	// full
	{
		CGBlock* pNew = new CGBlock;
		m_pCurrent->m_pNext = pNew;
		m_pCurrent = pNew;
		p = m_pCurrent->ContinuousAlloc(nLen);
	}

	return p;
}

void CGAlloc::FreeAll()
{
	while (m_pBlocks)
	{
		CGBlock* pOld = m_pBlocks;
		m_pBlocks = m_pBlocks->m_pNext;
		delete pOld;
	}
}

int CGAlloc::CopyToBuff(char* sz, size_t nBuffSize) const
{
	if (!m_pBlocks)	// empty but valid
	{
		if (nBuffSize <= 0)
			return -1;

		sz[0] = '\0';
		return 0;
	}

	int nLen = m_pBlocks->CopyToBuff(sz, nBuffSize);
	if (nLen == -1)
		return -1;

	sz[nLen] = '\0';
	return nLen + 1;
}

char* CGAlloc::AllocACopy(size_t* pLen) const
{
	size_t nSize = GetContinuousSize(TRUE);	// size in BYTES
	char* sz = new char[nSize];
	if (CopyToBuff(sz, nSize) == -1)
	{
		sz[0] = '\0';
		nSize = 1;
	}

	if (pLen)
		*pLen = nSize;
	return sz;
}

}

