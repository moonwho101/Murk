//-------------------------------------------------------------------
// CVORegistry implementation file
//-------------------------------------------------------------------
//
// Copyright ©2000-2001 Virtual Office Systems Incorporated
// All Rights Reserved
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included.
//
// This code can be compiled, modified and distributed freely, providing
// that this copyright information remains intact in the distribution.
//
// This code may be compiled in original or modified form in any private
// or commercial application.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage, in any form, caused
// by this code. Use it at your own risk.
//-------------------------------------------------------------------

#include "stdafx.h"
#include "VORegistry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

BOOL DeleteSubkey(HKEY hkey, LPCTSTR pcszSubkey);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVORegistry::CVORegistry(const CVORegistry &rSrc) {
	if (rSrc.m_hkey) {
		if (RegCreateKeyEx(rSrc.m_hkey, rSrc.m_strSubkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hkey, &m_dwDisposition) != ERROR_SUCCESS) {
			ASSERT(FALSE);
			TRACE(TEXT("CVORegistry() Copy Constructor: Unable to create subkey: %s\n"), rSrc.m_strSubkey);
		}
	} else
		m_hkey = 0;

	m_strSubkey = rSrc.m_strSubkey;
	m_nSubkeyIndex = 0;
}

CVORegistry::CVORegistry(HKEY hkeyParent, LPCTSTR pcszSubkey, BOOL fCreateIfNew) : m_nSubkeyIndex(0) {
	if (fCreateIfNew) {
		if (RegCreateKeyEx(hkeyParent, pcszSubkey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &m_hkey, &m_dwDisposition) != ERROR_SUCCESS) {
			TRACE(TEXT("CVORegistry() Constructor: Unable to create subkey: %s\n"), pcszSubkey);
		}
	} else {
		if (RegOpenKeyEx(hkeyParent, pcszSubkey, 0, KEY_ALL_ACCESS, &m_hkey) != ERROR_SUCCESS) {
			m_hkey = 0;
			TRACE(TEXT("CVORegistry() Constructor: Unable to open subkey: %s\n"), pcszSubkey);
		}
	}
}

CVORegistry::~CVORegistry() {
	RegCloseKey(m_hkey);
}

BOOL CVORegistry::Close() {
	if (m_hkey) {
		RegCloseKey(m_hkey);
		m_hkey = 0;
		return TRUE;
	}

	return FALSE;
}

BOOL CVORegistry::ReadBinary(LPCTSTR pcszKey, LPBYTE pData, LPDWORD pcbData) {
	if (pData && pcbData && *pcbData && (!AfxIsValidAddress(pData, *pcbData, TRUE))) {
		TRACE(TEXT("CVORegistry::ReadBinary() Invalid buffer specified\n"));
		return FALSE;
	}

	DWORD dwType = REG_BINARY;

	return (RegQueryValueEx(m_hkey, pcszKey, 0, &dwType, (PBYTE)&pData, pcbData) == ERROR_SUCCESS);
}

BOOL CVORegistry::WriteBinary(LPCTSTR pcszKey, LPBYTE pData, DWORD cbData) {
	if (!AfxIsValidAddress(pData, cbData, FALSE)) {
		TRACE(TEXT("CVORegistry::WriteBinary() Invalid buffer specified\n"));
		return FALSE;
	}

	return (RegSetValueEx(m_hkey, pcszKey, 0, REG_BINARY, (PBYTE)pData, cbData) == ERROR_SUCCESS);
}

DWORD CVORegistry::ReadDWORD(LPCTSTR pcszKey, DWORD dwDefault) {
	DWORD dwValue;
	DWORD dwValueSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	if (RegQueryValueEx(m_hkey, pcszKey, 0, &dwType, (PBYTE)&dwValue, &dwValueSize) == ERROR_SUCCESS)
		return dwValue;

	if (dwDefault != 0xDEF0) // Default specified
		RegSetValueEx(m_hkey, pcszKey, 0, REG_DWORD, (PBYTE)&dwDefault, sizeof(DWORD));

	return dwDefault;
}

BOOL CVORegistry::WriteDWORD(LPCTSTR pcszKey, DWORD dwValue) {
	if (!AfxIsValidString(pcszKey)) {
		TRACE(TEXT("CVORegistry::WriteDWORD() Invalid Key specified\n"));
		return FALSE;
	}

	return (RegSetValueEx(m_hkey, pcszKey, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(DWORD)) == ERROR_SUCCESS);
}

CString CVORegistry::ReadString(LPCTSTR pcszKey, LPCTSTR pcszDefault) {
	if (!AfxIsValidString(pcszKey)) {
		TRACE(TEXT("CVORegistry::ReadString() Invalid Key specified\n"));
		return TEXT("");
	}

	DWORD dwDataSize = 0;
	DWORD dwType = REG_SZ;

	if (RegQueryValueEx(m_hkey, pcszKey, 0, &dwType, (PBYTE)NULL, &dwDataSize) == ERROR_SUCCESS) {
		CString strValue;

		if (RegQueryValueEx(m_hkey, pcszKey, 0, &dwType, (PBYTE)(LPTSTR)strValue.GetBuffer(dwDataSize + 1), &dwDataSize) == ERROR_SUCCESS) {
			strValue.ReleaseBuffer();
			return strValue;
		} else {
			strValue.ReleaseBuffer();
			TRACE(TEXT("CVORegistry(%s)::ReadString() Unable to read variable %s\n"), m_strSubkey, pcszKey);
		}
	}

	// Write the default value (if any)  Note: If there was a value in the registry, it
	// would already have been written
	if (AfxIsValidString(pcszDefault)) {
		if (!WriteString(pcszKey, pcszDefault))
			TRACE(TEXT("CVORegistry(%s)::ReadString() Unable to write value %s to variable %s\n"), m_strSubkey, pcszDefault, pcszKey);
		return pcszDefault;
	} else
		return TEXT("");
}

BOOL CVORegistry::WriteString(LPCTSTR pcszKey, LPCTSTR pcszValue) {
	if (!AfxIsValidString(pcszKey)) {
		TRACE(TEXT("CVORegistry::WriteString() Invalid Key specified\n"));
		return FALSE;
	}
	if (!AfxIsValidString(pcszValue)) {
		TRACE(TEXT("CVORegistry::WriteString() Invalid Value specified\n"));
		return FALSE;
	}

	return (RegSetValueEx(m_hkey, pcszKey, 0, REG_SZ, (PBYTE)pcszValue, (_tcslen(pcszValue) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS);
}

BOOL CVORegistry::DeleteSubkey(LPCTSTR pcszSubkey) {
	return ::DeleteSubkey(m_hkey, pcszSubkey);
}

BOOL CVORegistry::GetFirstSubkey(CString &strSubkey) {
	DWORD dwNameSize = MAX_PATH;

	m_nSubkeyIndex = 0;

	if (RegEnumKeyEx(m_hkey, m_nSubkeyIndex, strSubkey.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL) != ERROR_SUCCESS) {
		strSubkey.ReleaseBuffer();
		strSubkey = TEXT("");
		return FALSE; // No subkeys
	}

	strSubkey.ReleaseBuffer();
	return TRUE;
}

BOOL CVORegistry::GetNextSubkey(CString &strSubkey) {
	DWORD dwNameSize = MAX_PATH;

	m_nSubkeyIndex++;

	if (RegEnumKeyEx(m_hkey, m_nSubkeyIndex, strSubkey.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL) != ERROR_SUCCESS) {
		strSubkey.ReleaseBuffer();
		strSubkey = TEXT("");
		return FALSE; // No more subkeys
	}

	strSubkey.ReleaseBuffer();
	return TRUE;
}

BOOL CVORegistry::GetSubkey(CString &strSubkey, int nOffset) {
	DWORD dwNameSize = MAX_PATH;
	LONG rc;

	m_nSubkeyIndex = nOffset;

	rc = RegEnumKeyEx(m_hkey, m_nSubkeyIndex, strSubkey.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL);
	strSubkey.ReleaseBuffer();

	if (rc != ERROR_SUCCESS) {
		if (rc != ERROR_NO_MORE_ITEMS) {
			TRACE(TEXT("CVORegistry::GetSubkey() : RegEnumKeyEx() Error#%ld\n"), GetLastError());
		}

		strSubkey = TEXT("");
		return FALSE; // No subkeys
	}

	return TRUE;
}

BOOL CVORegistry::SubkeyExists(LPCTSTR pcszSubkey) {
	HKEY hkeySubkey;
	DWORD dwRC = RegOpenKeyEx(m_hkey, pcszSubkey, 0, KEY_ALL_ACCESS, &hkeySubkey);

	if (dwRC != ERROR_SUCCESS)
		return FALSE;

	RegCloseKey(hkeySubkey);
	return TRUE;
}

BOOL CVORegistry::DeleteValue(LPCTSTR pcszValueName) {
	return (RegDeleteValue(m_hkey, pcszValueName) == ERROR_SUCCESS);
}

BOOL CVORegistry::GetFirstValue(CString &strValueName) {
	DWORD dwNameSize = MAX_PATH;

	m_nSubkeyIndex = 0;

	if (RegEnumValue(m_hkey, m_nSubkeyIndex, strValueName.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL) != ERROR_SUCCESS) {
		strValueName.ReleaseBuffer();
		return FALSE; // No subkeys
	}

	strValueName.ReleaseBuffer();
	return TRUE;
}

BOOL CVORegistry::GetNextValue(CString &strValueName) {
	DWORD dwNameSize = MAX_PATH;

	m_nSubkeyIndex++;

	if (RegEnumValue(m_hkey, m_nSubkeyIndex, strValueName.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL) != ERROR_SUCCESS) {
		strValueName.ReleaseBuffer();
		return FALSE; // No subkeys
	}

	strValueName.ReleaseBuffer();
	return TRUE;
}

BOOL CVORegistry::ValueExists(LPCTSTR pcszValueName) {
	if (!AfxIsValidString(pcszValueName)) {
		TRACE(TEXT("CVORegistry::ValueExists() Invalid Key specified\n"));
		return FALSE;
	}

	DWORD dwDataSize = 0;
	DWORD dwType = REG_SZ;

	return (RegQueryValueEx(m_hkey, pcszValueName, 0, &dwType, (PBYTE)NULL, &dwDataSize) == ERROR_SUCCESS);
}

// Recursively delete a key in the registry
BOOL DeleteSubkey(HKEY hkey, LPCTSTR pcszSubkey) {
	if (!pcszSubkey)
		return FALSE;
	else {
		// Recursively delete any subkeys for the target subkey
		HKEY hkeySubkey;
		CString strSubkey;

		DWORD dwRC = RegOpenKeyEx(hkey, pcszSubkey, 0, KEY_ALL_ACCESS, &hkeySubkey);

		if (dwRC != ERROR_SUCCESS) {
			ASSERT(FALSE);
			return FALSE;
		}

		DWORD dwNameSize = MAX_PATH;

		while (RegEnumKeyEx(hkeySubkey, 0, strSubkey.GetBuffer(dwNameSize), &dwNameSize, 0, NULL, NULL, NULL) == ERROR_SUCCESS) {
			strSubkey.ReleaseBuffer();

			if (!DeleteSubkey(hkeySubkey, strSubkey)) {
				RegCloseKey(hkeySubkey);
				return FALSE;
			}

			dwNameSize = MAX_PATH;
		}

		strSubkey.ReleaseBuffer();

		RegCloseKey(hkeySubkey);
	}

	return (RegDeleteKey(hkey, pcszSubkey) == ERROR_SUCCESS);
}
