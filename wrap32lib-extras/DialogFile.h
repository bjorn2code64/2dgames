#pragma once
#include <string>

#include <shobjidl_core.h>

#include <FileHandler.h>

/* Example Filterspec

COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
	{ L"FCS/LMD Files", L"*.fcs;*.lmd" },
	{ L"All Files", L"*.*" },
};

*/

class DialogFileOpen
{
public:
	DialogFileOpen(COMDLG_FILTERSPEC* saveTypes, int saveTypesSize) : m_pfd(NULL) {

		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_pfd))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(m_pfd->GetOptions(&dwFlags))) {

				// In this case, get shell items only for file system items.
				if (SUCCEEDED(m_pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM))) {

					// Set the file types to display only. 
					// Notice that this is a 1-based array.
					if (SUCCEEDED(m_pfd->SetFileTypes(saveTypesSize, saveTypes))) {
						m_pfd->SetFileTypeIndex(1);
					}
				}
			}
		}
	}

	~DialogFileOpen() {
		if (m_pfd)
			m_pfd->Release();
	}

	DWORD DoModal()
	{
		// Show the dialog
		HRESULT hr = m_pfd->Show(NULL);
		if (!SUCCEEDED(hr))
			return hr;	// ERROR_CANCELLED if the user cancelled it

		// Obtain the result once the user clicks 
		// the 'Open' button.
		// The result is an IShellItem object.
		IShellItem* psiResult;
		hr = m_pfd->GetResult(&psiResult);
		if (!SUCCEEDED(hr))
			return hr;

		// We are just going to print out the 
		// name of the file for sample sake.
		PWSTR pszFilePath = NULL;
		hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		if (!SUCCEEDED(hr))
			return hr;

		m_strPath = pszFilePath;
		CoTaskMemFree(pszFilePath);

		psiResult->Release();
		return hr;
	}

	LPCWSTR GetPath() { return m_strPath.c_str(); }

protected:
	IFileDialog* m_pfd;
	std::wstring m_strPath;
};

class DialogFileSave
{
public:
	DialogFileSave(COMDLG_FILTERSPEC* saveTypes, int saveTypesSize, LPCWSTR szDefaultExt) : m_pfd(NULL), m_strDefaultExt(szDefaultExt) {
		if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_pfd))))
		{
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			if (SUCCEEDED(m_pfd->GetOptions(&dwFlags))) {

				// In this case, get shell items only for file system items.
				if (SUCCEEDED(m_pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM))) {

					// Set the file types to display only. 
					// Notice that this is a 1-based array.
					if (SUCCEEDED(m_pfd->SetFileTypes(saveTypesSize, saveTypes))) {
						m_pfd->SetFileTypeIndex(1);
					}
				}
			}
		}
	}

	~DialogFileSave() {
		if (m_pfd)
			m_pfd->Release();
	}

	DWORD DoModal()
	{
		// Show the dialog
		HRESULT hr = m_pfd->Show(NULL);
		if (!SUCCEEDED(hr))
			return hr;	// ERROR_CANCELLED if the user cancelled it

		// Obtain the result once the user clicks 
		// the 'Open' button.
		// The result is an IShellItem object.
		IShellItem* psiResult;
		hr = m_pfd->GetResult(&psiResult);
		if (!SUCCEEDED(hr))
			return hr;

		// We are just going to print out the 
		// name of the file for sample sake.
		PWSTR pszFilePath = NULL;
		hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		if (!SUCCEEDED(hr))
			return hr;

		FileHandler fh(pszFilePath, m_strDefaultExt.c_str());
		CoTaskMemFree(pszFilePath);

		m_strPath = fh;

		psiResult->Release();
		return hr;
	}

	LPCWSTR GetPath() { return m_strPath.c_str(); }

protected:
	IFileDialog* m_pfd;
	std::wstring m_strPath;
	std::wstring m_strDefaultExt;
};