#pragma once

#include <stdio.h>
#include <windows.h>
#include <Wincrypt.h>
#include <string>

#pragma comment (lib, "Crypt32")
#pragma comment (lib, "advapi32")

#define KEYLENGTH			0x00800000
#define ENCRYPT_ALGORITHM	CALG_RC4 
#define ENCRYPT_BLOCK_SIZE	8 

class Crypt {
public:
	Crypt(DWORD prov = PROV_RSA_FULL) : m_prov(prov), m_hCryptProv(NULL) {
	}

	~Crypt() {
		if (m_hCryptProv)
			CryptReleaseContext(m_hCryptProv, 0);
	}

	bool EncodeHex(BYTE* content, DWORD len, std::string& out) {
		DWORD base64len;
		if (!CryptBinaryToStringA(content, len, CRYPT_STRING_HEXRAW | CRYPT_STRING_NOCRLF, NULL, &base64len))
			return false;

		LPSTR base64Content = new char[base64len];
		if (!CryptBinaryToStringA(content, len, CRYPT_STRING_HEXRAW | CRYPT_STRING_NOCRLF, base64Content, &base64len))
			return false;

		out = base64Content;
		delete[] base64Content;
		return true;
	}

	DWORD Encrypt(BYTE* pbBuffer, DWORD len, BYTE* pszPassword, DWORD lenPassword)	{
		if (!m_hCryptProv) {
			if (!CryptAcquireContext(
				&m_hCryptProv,
				NULL,
				MS_ENHANCED_PROV,
				m_prov,
				0))
			{
				DWORD err = GetLastError();
				if (err != NTE_BAD_KEYSET) {
					return err;
				}

				if (!CryptAcquireContext(
					&m_hCryptProv,
					NULL,
					MS_ENHANCED_PROV,
					m_prov,
					CRYPT_NEWKEYSET))
				{
					return GetLastError();
				}
			}
		}

		HCRYPTHASH hHash;
		if (!CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &hHash)) {
			return GetLastError();
		}

		if (!CryptHashData(hHash, pszPassword, lenPassword, 0)) {
			CryptDestroyHash(hHash);
			return GetLastError();
		}
		
				HCRYPTKEY hKey = NULL;
		if (!CryptDeriveKey(m_hCryptProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey)) {
			CryptDestroyHash(hHash);
			return GetLastError();
		}
		
					DWORD dwCount = len;
		if (!CryptEncrypt(hKey, NULL, TRUE, 0, pbBuffer, &dwCount, len)) {
			CryptDestroyKey(hKey);
			CryptDestroyHash(hHash);
			return GetLastError();
		}
								
					CryptDestroyKey(hKey);
		CryptDestroyHash(hHash);

		return ERROR_SUCCESS;
	}

	DWORD Decrypt(BYTE* pbBuffer, DWORD len, BYTE* pszPassword, DWORD lenPassword) {

		if (!m_hCryptProv) {
			if (!CryptAcquireContext(
				&m_hCryptProv,
				NULL,
				MS_ENHANCED_PROV,
				m_prov,
				0))
			{
				DWORD err = GetLastError();
				if (err != NTE_BAD_KEYSET) {
					return err;
				}

				if (!CryptAcquireContext(
					&m_hCryptProv,
					NULL,
					MS_ENHANCED_PROV,
					m_prov,
					CRYPT_NEWKEYSET))
				{
					return GetLastError();
				}
				}
			}

		HCRYPTHASH hHash;
		if (!CryptCreateHash(m_hCryptProv, CALG_MD5, 0, 0, &hHash)) {
			return GetLastError();
		}

		if (!CryptHashData(hHash, pszPassword, lenPassword, 0)) {
			CryptDestroyHash(hHash);
			return GetLastError();
		}
		
		HCRYPTKEY hKey = NULL;
		if (!CryptDeriveKey(m_hCryptProv, ENCRYPT_ALGORITHM, hHash, KEYLENGTH, &hKey)) {
			CryptDestroyHash(hHash);
			return GetLastError();
		}

		DWORD dwCount = len;
		if (!CryptDecrypt(hKey, NULL, TRUE, 0, pbBuffer, &dwCount)) {
			CryptDestroyKey(hKey);
			CryptDestroyHash(hHash);
			return GetLastError();
	}

		CryptDestroyKey(hKey);
		CryptDestroyHash(hHash);

		return ERROR_SUCCESS;
	}
protected:
	DWORD m_prov;
	HCRYPTPROV m_hCryptProv;
};

class CryptHasher : public Crypt {
public:
	CryptHasher(LPCSTR key, ALG_ID hashAlgorithmID = CALG_SHA_256) :
		Crypt(PROV_RSA_AES),
		m_key(key), m_hHash(NULL)
	{
		if (m_hCryptProv)
			CryptCreateHash(m_hCryptProv, hashAlgorithmID, 0, 0, &m_hHash);
	}

	~CryptHasher() {
		if (m_hHash)
			CryptDestroyHash(m_hHash);
	}

	bool Hash(const std::string& in, std::string& out) {
		if (!m_hHash)
			return false;

		std::string full = m_key + in;
		if (!CryptHashData(m_hHash, (const BYTE*)full.c_str(), (DWORD)full.size(), 0))
			return false;

		DWORD dwCount = sizeof(DWORD);
		DWORD cbHashSize = 0;
		if (!CryptGetHashParam(m_hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0))
			return false;

		BYTE* outBuff = new BYTE[cbHashSize];
		if (!CryptGetHashParam(m_hHash, HP_HASHVAL, outBuff, &cbHashSize, 0))
			return false;

		if (!EncodeHex(outBuff, cbHashSize, out))
			return false;

		delete[] outBuff;
		return true;
	}

protected:
	std::string m_key;
	HCRYPTHASH m_hHash;
};

