#include <Windows.h>
#include <stdio.h>
#include "QrInternals.h"


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// File I/O
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region FILE_IO

static BOOL ReadFileFromDiskW(IN LPCWSTR szFileName, OUT PBYTE* ppFileBuffer, OUT PDWORD pdwFileSize)
{
	HANDLE		hFile					= INVALID_HANDLE_VALUE;
	DWORD		dwFileSize				= 0x00,
				dwNumberOfBytesRead		= 0x00;
	PBYTE		pBaseAddress			= NULL;

	if (!szFileName || !pdwFileSize || !ppFileBuffer)
		return FALSE;

	if ((hFile = CreateFileW(szFileName, GENERIC_READ, 0x00, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) 
    {
        DBG_LAST_ERROR("CreateFileW");
		goto _END_OF_FUNC;
	}

	if ((dwFileSize = GetFileSize(hFile, NULL)) == INVALID_FILE_SIZE) 
    {
        DBG_LAST_ERROR("GetFileSize");
		goto _END_OF_FUNC;
	}

    HEAP_ALLOC(pBaseAddress, dwFileSize);
    if (!pBaseAddress) goto _END_OF_FUNC;

	if (!ReadFile(hFile, pBaseAddress, dwFileSize, &dwNumberOfBytesRead, NULL) || dwFileSize != dwNumberOfBytesRead) 
    {
        DBG_LAST_ERROR("ReadFile");
        DBG("[i] Read %d Of %d Bytes", dwNumberOfBytesRead, dwFileSize);
		goto _END_OF_FUNC;
	}

	*ppFileBuffer = pBaseAddress;
	*pdwFileSize  = dwFileSize;

_END_OF_FUNC:
    CLOSE_HANDLE(hFile);
    if (!*ppFileBuffer) { HEAP_FREE(pBaseAddress); }
	return (*ppFileBuffer && *pdwFileSize) ? TRUE : FALSE;
}

static BOOL WriteFileToDiskW(IN LPCWSTR pwszFileName, IN CONST BYTE* pbDataBuffer, IN DWORD dwDataLength)
{
    HANDLE  hFile                   = INVALID_HANDLE_VALUE;
    DWORD   dwNumerOfBytesWritten   = 0x00;
    BOOL    bResult                 = FALSE;

    if (!pwszFileName || !pbDataBuffer || dwDataLength == 0x00)
        return FALSE;

    if ((hFile = CreateFileW(pwszFileName, GENERIC_WRITE, 0x00, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        DBG_LAST_ERROR("CreateFileW");
        goto _END_OF_FUNC;
    }

    if (!WriteFile(hFile, pbDataBuffer, dwDataLength, &dwNumerOfBytesWritten, NULL) || dwNumerOfBytesWritten != dwDataLength)
    {
        DBG_LAST_ERROR("WriteFile");
        DBG("[i] Wrote %d Of %d Bytes", dwNumerOfBytesWritten, dwDataLength);
        goto _END_OF_FUNC;
    }

    bResult = TRUE;

_END_OF_FUNC:
    CLOSE_HANDLE(hFile);
    return bResult;
}

static BOOL PathIsExistingFileW(IN LPCWSTR pwszPath)
{
    DWORD dwAttribute = GetFileAttributesW(pwszPath);
    return (dwAttribute != INVALID_FILE_ATTRIBUTES && !(dwAttribute & FILE_ATTRIBUTE_DIRECTORY));
}

static BOOL PathIsExistingDirW(IN LPCWSTR pwszPath)
{
    DWORD dwAttribute = GetFileAttributesW(pwszPath);
    return (dwAttribute != INVALID_FILE_ATTRIBUTES && (dwAttribute & FILE_ATTRIBUTE_DIRECTORY));
}

#pragma endregion


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Entry Point
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==


int wmain(IN int argc, IN WCHAR** argv)
{
    if (argc < 3)
    {
_HELP_MSG:
        wprintf(
            L"Usage:\n"
            L"  %s encode <input_file> <output_dir>\n"
            L"  %s decode <input_dir>\n",
            GET_FILENAMEW(argv[0]), GET_FILENAMEW(argv[0])
        );
        
        return -1;
    }

    LPCWSTR pwszCmd             = argv[1];
    PBYTE   pFileData           = NULL;
    DWORD   dwFileDataLength    = 0x00;
    BOOL    bResult             = FALSE;

    if (StrCmpIW(pwszCmd, L"encode") == 0 || StrCmpIW(pwszCmd, L"-e") == 0)
    {
        if (argc < 4) goto _HELP_MSG;

        if (!PathIsExistingFileW(argv[2]))
        {
            wprintf(L"[!] Input File Not Found: %s\n", argv[2]);
            return -1;
        }
        
        if (!PathIsExistingDirW(argv[3]))
        {
            wprintf(L"[!] Output Directory Not Found: %s\n", argv[3]);
            return -1;
        }

        if (ReadFileFromDiskW(argv[2], &pFileData, &dwFileDataLength))
        {
            bResult = CmdEncode(pFileData, dwFileDataLength, GET_FILENAMEW(argv[2]), argv[3]);
            HEAP_FREE(pFileData);
        }
    }
    else if (StrCmpIW(pwszCmd, L"decode") == 0 || StrCmpIW(pwszCmd, L"-d") == 0)
    {
        LPWSTR pwszEmbeddedName = NULL;

        if (!PathIsExistingDirW(argv[2]))
        {
            wprintf(L"[!] Input Directory Not Found: %s\n", argv[2]);
            return -1;
        }

        if (CmdDecode(argv[2], &pFileData, &dwFileDataLength, &pwszEmbeddedName))
        {
            WCHAR wszOutPath[MAX_PATH] = { 0 };

            if (pwszEmbeddedName)
            {
                StringCchPrintfW(wszOutPath, MAX_PATH, L"%s\\%s", argv[2], pwszEmbeddedName);
            }
            else
            {
                WCHAR   wszFileName[MAX_PATH]   = { 0 };
                DWORD   dwFileNameLen           = 0x00;

                while (1)
                {
                    wprintf(L"[-] No Embedded Filename Found. Enter Output Filename: ");
                    
                    if (fgetws(wszFileName, MAX_PATH, stdin))
                    {
                        dwFileNameLen = (DWORD)lstrlenW(wszFileName);
                        if (dwFileNameLen > 0 && wszFileName[dwFileNameLen - 1] == L'\n') wszFileName[dwFileNameLen - 1] = L'\0';
                        if (wszFileName[0]) break;
                    }
                    
                    wprintf(L"[!] No Filename Provided. Try Again.\n");
                }

                StringCchPrintfW(wszOutPath, MAX_PATH, L"%s\\%s", argv[2], wszFileName);
            }

            if ((bResult = WriteFileToDiskW(wszOutPath, pFileData, dwFileDataLength)))
                wprintf(L"[i] Decoded Data Written To: %s\n", GET_FILENAMEW(wszOutPath));

            HEAP_FREE(pFileData);
        }

        HEAP_FREE(pwszEmbeddedName);
    }
    else
    {
        wprintf(L"[!] Unknown Command: %s\n", pwszCmd);
        goto _HELP_MSG;
    }

    wprintf(bResult ? L"[+] Done.\n" : L"[!] Failed.\n");

    return bResult ? 0 : -1;
}