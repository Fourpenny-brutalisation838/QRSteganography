#include "QrInternals.h"

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// WIC HELPERS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region WIC_HELPERS

// Renders a QR code module matrix to a scaled grayscale PNG file via WIC.
static BOOL QrRenderToPng(IN CONST BYTE* pbQr, IN LPCWSTR pwszOutPath)
{
    IWICImagingFactory      *pFactory       = NULL;
    IWICBitmap              *pBitmap        = NULL;
    IWICBitmapLock          *pLock          = NULL;
    IWICStream              *pStream        = NULL;
    IWICBitmapEncoder       *pEncoder       = NULL;
    IWICBitmapFrameEncode   *pFrame         = NULL;
    IPropertyBag2           *pProps         = NULL;
    INT                     nQrSize         = 0x00,
                            nQuiet          = 0x04,
                            nPxSize         = 0x00,
                            nStride         = 0x00;
    UINT                    cbData          = 0x00;
    PBYTE                   pbData          = NULL;
    WICRect                 rcLock          = { 0 };
    WICPixelFormatGUID      PixFmt          = GUID_WICPixelFormat8bppGray;
    HRESULT                 hResult         = S_OK;
    BOOL                    bResult         = FALSE;

    if (!pbQr || !pwszOutPath)
        return FALSE;

    if ((nQrSize = QrGetSize(pbQr)) <= 0)
        return FALSE;

    // Compute output image size: (QR modules + quiet zone) * pixel scale
    if (nQrSize > (INT_MAX - (nQuiet * QR_QUIET_ZONE_SIDES)))
        return FALSE;

    nPxSize = nQrSize + (nQuiet * QR_QUIET_ZONE_SIDES);
    if (nPxSize > (INT_MAX / QR_MODULE_PX))
        return FALSE;

    nPxSize *= QR_MODULE_PX;
    if (nPxSize <= 0)
        return FALSE;

    rcLock.X      = 0;
    rcLock.Y      = 0;
    rcLock.Width  = nPxSize;
    rcLock.Height = nPxSize;

    if (FAILED(hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        DBG_HEX_ERROR("CoInitializeEx", hResult);
        return FALSE;
    }

    if (FAILED(hResult = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (PVOID*)&pFactory)))
    {
        DBG_HEX_ERROR("CoCreateInstance", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFactory->lpVtbl->CreateBitmap(pFactory, (UINT)nPxSize, (UINT)nPxSize, &GUID_WICPixelFormat8bppGray, WICBitmapCacheOnLoad, &pBitmap)))
    {
        DBG_HEX_ERROR("IWICImagingFactory::CreateBitmap", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pBitmap->lpVtbl->Lock(pBitmap, &rcLock, WICBitmapLockWrite, &pLock)))
    {
        DBG_HEX_ERROR("IWICBitmap::Lock", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pLock->lpVtbl->GetStride(pLock, &nStride)))
    {
        DBG_HEX_ERROR("IWICBitmapLock::GetStride", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pLock->lpVtbl->GetDataPointer(pLock, &cbData, &pbData)))
    {
        DBG_HEX_ERROR("IWICBitmapLock::GetDataPointer", hResult);
        goto _END_OF_FUNC;
    }

    if ((SIZE_T)nStride * (SIZE_T)nPxSize > (SIZE_T)cbData)
    {
        DBG("[!] Locked Bitmap Buffer Is Smaller Than Expected");
        goto _END_OF_FUNC;
    }

    // Start with all white image
    FillMemory(pbData, cbData, 0xFF);

    // Draw each dark module as a QR_MODULE_PX x QR_MODULE_PX black square
    for (INT y = 0x00; y < nQrSize; y++)
    {
        for (INT x = 0x00; x < nQrSize; x++)
        {
            if (!QrGetModule(pbQr, x, y))
                continue;

            for (INT dy = 0x00; dy < QR_MODULE_PX; dy++)
            {
                for (INT dx = 0x00; dx < QR_MODULE_PX; dx++)
                {
                    INT nRow    = (nQuiet + y) * QR_MODULE_PX + dy;
                    INT nCol    = (nQuiet + x) * QR_MODULE_PX + dx;

                    if (nRow < 0 || nRow >= nPxSize || nCol < 0 || nCol >= nPxSize)
                        goto _END_OF_FUNC;
                    
                    pbData[((SIZE_T)nRow * (SIZE_T)nStride) + (SIZE_T)nCol] = 0x00;
                }
            }
        }
    }

    pLock->lpVtbl->Release(pLock);
    pLock = NULL;

    if (FAILED(hResult = pFactory->lpVtbl->CreateStream(pFactory, &pStream)))
    {
        DBG_HEX_ERROR("IWICImagingFactory::CreateStream", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pStream->lpVtbl->InitializeFromFilename(pStream, pwszOutPath, GENERIC_WRITE)))
    {
        DBG_HEX_ERROR("IWICStream::InitializeFromFilename", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFactory->lpVtbl->CreateEncoder(pFactory, &GUID_ContainerFormatPng, NULL, &pEncoder)))
    {
        DBG_HEX_ERROR("IWICImagingFactory::CreateEncoder", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pEncoder->lpVtbl->Initialize(pEncoder, (IStream*)pStream, WICBitmapEncoderNoCache)))
    {
        DBG_HEX_ERROR("IWICBitmapEncoder::Initialize", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pEncoder->lpVtbl->CreateNewFrame(pEncoder, &pFrame, &pProps)))
    {
        DBG_HEX_ERROR("IWICBitmapEncoder::CreateNewFrame", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->Initialize(pFrame, pProps)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameEncode::Initialize", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->SetSize(pFrame, (UINT)nPxSize, (UINT)nPxSize)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameEncode::SetSize", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->SetPixelFormat(pFrame, &PixFmt)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameEncode::SetPixelFormat", hResult);
        goto _END_OF_FUNC;
    }

    if (!IsEqualGUID(&PixFmt, &GUID_WICPixelFormat8bppGray))
    {
        DBG("[!] Encoder Rejected 8bppGray Pixel Format, Got: {%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            PixFmt.Data1, PixFmt.Data2, PixFmt.Data3,
            PixFmt.Data4[0], PixFmt.Data4[1], PixFmt.Data4[2], PixFmt.Data4[3],
            PixFmt.Data4[4], PixFmt.Data4[5], PixFmt.Data4[6], PixFmt.Data4[7]);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->WriteSource(pFrame, (IWICBitmapSource*)pBitmap, NULL)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameEncode::WriteSource", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->Commit(pFrame)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameEncode::Commit", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pEncoder->lpVtbl->Commit(pEncoder)))
    {
        DBG_HEX_ERROR("IWICBitmapEncoder::Commit", hResult);
        goto _END_OF_FUNC;
    }

    bResult = TRUE;

_END_OF_FUNC:
    if (pProps)     pProps->lpVtbl->Release(pProps);
    if (pFrame)     pFrame->lpVtbl->Release(pFrame);
    if (pEncoder)   pEncoder->lpVtbl->Release(pEncoder);
    if (pStream)    pStream->lpVtbl->Release(pStream);
    if (pLock)      pLock->lpVtbl->Release(pLock);
    if (pBitmap)    pBitmap->lpVtbl->Release(pBitmap);
    if (pFactory)   pFactory->lpVtbl->Release(pFactory);
    CoUninitialize();
    return bResult;
}

// Loads any WIC-supported image and converts it to a flat 8bpp grayscale buffer.
static PBYTE WicLoadGrayscale(IN LPCWSTR pwszFilePath, OUT PDWORD pdwWidth, OUT PDWORD pdwHeight)
{
    IWICImagingFactory      *pFactory       = NULL;
    IWICBitmapDecoder       *pDecoder       = NULL;
    IWICBitmapFrameDecode   *pFrame         = NULL;
    IWICFormatConverter     *pConverter     = NULL;
    PBYTE                   pbOutput        = NULL;
    UINT                    uWidth          = 0x00,
                            uHeight         = 0x00;
    HRESULT                 hResult         = S_OK;

    if (!pwszFilePath || !pdwWidth || !pdwHeight)
        return NULL;

    *pdwWidth  = 0x00;
    *pdwHeight = 0x00;

    if (FAILED(hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        DBG_HEX_ERROR("CoInitializeEx", hResult);
        return NULL;
    }

    if (FAILED(hResult = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (PVOID*)&pFactory)))
    {
        DBG_HEX_ERROR("CoCreateInstance", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFactory->lpVtbl->CreateDecoderFromFilename(pFactory, pwszFilePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder)))
    {
        DBG_HEX_ERROR("IWICImagingFactory::CreateDecoderFromFilename", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pDecoder->lpVtbl->GetFrame(pDecoder, 0x00, &pFrame)))
    {
        DBG_HEX_ERROR("IWICBitmapDecoder::GetFrame", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFrame->lpVtbl->GetSize(pFrame, &uWidth, &uHeight)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameDecode::GetSize", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pFactory->lpVtbl->CreateFormatConverter(pFactory, &pConverter)))
    {
        DBG_HEX_ERROR("IWICImagingFactory::CreateFormatConverter", hResult);
        goto _END_OF_FUNC;
    }

    if (FAILED(hResult = pConverter->lpVtbl->Initialize(pConverter, (IWICBitmapSource*)pFrame, &GUID_WICPixelFormat8bppGray, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut)))
    {
        DBG_HEX_ERROR("IWICFormatConverter::Initialize", hResult);
        goto _END_OF_FUNC;
    }

    if ((SIZE_T)uWidth > (SIZE_T)-1 / (SIZE_T)uHeight)
        goto _END_OF_FUNC;

    HEAP_ALLOC(pbOutput, (SIZE_T)uWidth * (SIZE_T)uHeight);
    if (!pbOutput) goto _END_OF_FUNC;

    if (FAILED(hResult = pFrame->lpVtbl->CopyPixels(pFrame, NULL, uWidth, (SIZE_T)uWidth * (SIZE_T)uHeight, pbOutput)))
    {
        DBG_HEX_ERROR("IWICBitmapFrameDecode::CopyPixels", hResult);
        HEAP_FREE(pbOutput);
        goto _END_OF_FUNC;
    }

    *pdwWidth  = (DWORD)uWidth;
    *pdwHeight = (DWORD)uHeight;

_END_OF_FUNC:
    if (pConverter)     pConverter->lpVtbl->Release(pConverter);
    if (pFrame)         pFrame->lpVtbl->Release(pFrame);
    if (pDecoder)       pDecoder->lpVtbl->Release(pDecoder);
    if (pFactory)       pFactory->lpVtbl->Release(pFactory);
    CoUninitialize();
    return pbOutput;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// PACKING & CHUNKING PACKETS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region CHUNK_PACKET

// Serialises one chunk into a flat binary packet
static BOOL  BuildChunkPacket(IN DWORD dwTotal, IN DWORD dwIndex, IN DWORD dwOrigSize, IN LPCSTR pszName, IN CONST BYTE* pbPayload, IN DWORD cbPayload, OUT PBYTE* ppbPacket, OUT PDWORD pcbPacket)
{
    SIZE_T  cbName      = lstrlenA(pszName);
    DWORD   cbPacket    = 0x00;
    PBYTE   pbPacket    = NULL;
    PBYTE   pbCur       = NULL;
    WORD    wNameLen    = 0x00;

    *ppbPacket = NULL;
    *pcbPacket = 0x00;

    if (cbName > MAXWORD) return FALSE;

    wNameLen = (WORD)cbName;
    // header + name + data
    cbPacket = CHUNK_HDR_FIXED_SIZE + wNameLen + cbPayload;

    HEAP_ALLOC(pbPacket, cbPacket);
    if (!pbPacket) return FALSE;

    pbCur = pbPacket;
    
    // Write fields: magic, total chunks, index, original size, name length, name, payload
#pragma warning(suppress: 6386)
    RtlCopyMemory(pbCur, CHUNK_MAGIC, CHUNK_MAGIC_LEN);     pbCur += CHUNK_MAGIC_LEN;
    RtlCopyMemory(pbCur, &dwTotal, sizeof(dwTotal));        pbCur += sizeof(dwTotal);
    RtlCopyMemory(pbCur, &dwIndex, sizeof(dwIndex));        pbCur += sizeof(dwIndex);
    RtlCopyMemory(pbCur, &dwOrigSize, sizeof(dwOrigSize));  pbCur += sizeof(dwOrigSize);
    RtlCopyMemory(pbCur, &wNameLen, sizeof(wNameLen));      pbCur += sizeof(wNameLen);
    RtlCopyMemory(pbCur, pszName, wNameLen);                pbCur += wNameLen;
    RtlCopyMemory(pbCur, pbPayload, cbPayload);

    *ppbPacket = pbPacket;
    *pcbPacket = cbPacket;

    return TRUE;
}

// Deserialises a flat binary packet back into its component fields
static BOOL  ParseChunkPacket(IN BYTE* pbPacket, IN DWORD cbPacket, OUT PDWORD pdwTotal, OUT PDWORD pdwIndex, OUT PDWORD pdwOrigSize, OUT LPCSTR* ppszName, OUT PWORD pwNameLen, OUT BYTE** ppbPayload, OUT PDWORD pcbPayload)
{
    BYTE*   pbCur   = NULL;
    DWORD   dwUsed  = 0x00;

    if (cbPacket < CHUNK_HDR_FIXED_SIZE)
    {
        DBG("[!] Packet Too Small! Got: %lu Instead Of: %d", cbPacket, CHUNK_HDR_FIXED_SIZE);
        return FALSE;
    }

    if (RtlCompareMemory(pbPacket, CHUNK_MAGIC, CHUNK_MAGIC_LEN) != CHUNK_MAGIC_LEN)
    {
        DBG("[!] Chunk Magic Mismatch! Got: 0x%0.4X (%.4s) Instead Of: %.4s", *(DWORD*)pbPacket, (CHAR*)pbPacket, CHUNK_MAGIC);
        return FALSE;
    }
    
    pbCur = pbPacket + CHUNK_MAGIC_LEN;

    // Read header fields: total, index, original size, name length
    RtlCopyMemory(pdwTotal, pbCur, sizeof(*pdwTotal));          pbCur += sizeof(*pdwTotal);
    RtlCopyMemory(pdwIndex, pbCur, sizeof(*pdwIndex));          pbCur += sizeof(*pdwIndex);
    RtlCopyMemory(pdwOrigSize, pbCur, sizeof(*pdwOrigSize));    pbCur += sizeof(*pdwOrigSize);
    RtlCopyMemory(pwNameLen, pbCur, sizeof(*pwNameLen));        pbCur += sizeof(*pwNameLen);

    dwUsed = CHUNK_HDR_FIXED_SIZE + *pwNameLen;
    if (dwUsed > cbPacket)
    {
        DBG("[!] Name Length Exceeds Packet Bounds! Required: %lu, Available: %lu", dwUsed, cbPacket);
        return FALSE;
    }

    *ppszName   = (LPCSTR)pbCur;
    *ppbPayload = pbCur + *pwNameLen;
    *pcbPayload = cbPacket - dwUsed;

    return TRUE;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// ENCODING COMMAND
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region CMD_ENCODE

// Splits input data into chunks of CHUNK_PAYLOAD_MAX bytes. Builds a raw binary packet for each chunk, then encodes it directly into a QR code. All QR codes are written as a numbered PNG into pwszOutDir.
BOOL CmdEncode(IN PBYTE pbData, IN DWORD cbData, IN LPCWSTR pwszName, IN LPCWSTR pwszOutDir)
{
    PBYTE   pbPacket            = NULL;
    PBYTE   pbQr                = NULL;
    BOOL    bResult             = FALSE;
    DWORD   dwTotal             = 0x00,
            dwPacketSz          = 0x00;
    CHAR    szName[MAX_PATH]    = { 0 };
    WCHAR   wszOutDir[MAX_PATH] = { 0 };

    if (!pbData || !cbData || !pwszName || !pwszOutDir)
        goto _END_OF_FUNC;

    StringCchCopyW(wszOutDir, MAX_PATH, pwszOutDir);
    PathRemoveBackslashW(wszOutDir);

    WideCharToMultiByte(CP_UTF8, 0x00, pwszName, -1, szName, MAX_PATH, NULL, NULL);

    dwTotal = (cbData + CHUNK_PAYLOAD_MAX - 1) / CHUNK_PAYLOAD_MAX;

    HEAP_ALLOC(pbQr, QR_BUFFER_LEN_MAX);
    if (!pbQr) goto _END_OF_FUNC;

    wprintf(L"[*] Encoding '%s' (%lu Bytes) Into: '%s\\%s'\n", pwszName, cbData, wszOutDir, QR_PNG_GLOB);
    wprintf(L"[i] %lu Chunk(s) At Up To %lu Bytes Each\n", dwTotal, (DWORD)CHUNK_PAYLOAD_MAX);

    for (DWORD dwI = 0x00; dwI < dwTotal; dwI++)
    {
        DWORD   dwOffset                = dwI * CHUNK_PAYLOAD_MAX;
        DWORD   cbPayload               = min(CHUNK_PAYLOAD_MAX, cbData - dwOffset);
        WCHAR   szOutFile[MAX_PATH]     = { 0 };

        if (!BuildChunkPacket(dwTotal, dwI, cbData, szName, pbData + dwOffset, cbPayload, &pbPacket, &dwPacketSz))
        {
            DBG("[!] Unable To Serialize Chunk %lu/%lu Into Packet (Offset: %lu, PayloadSize: %lu)", dwI, dwTotal, dwOffset, cbPayload);
            goto _END_OF_FUNC;
        }

        if (!QrEncode(pbPacket, dwPacketSz, pbQr, QR_ECC_LEVEL, QR_VERSION_MIN, QR_VERSION_MAX))
        {
            DBG("[!] Unable To Encode Chunk %lu/%lu As QR Code (PacketSize: %lu)", dwI, dwTotal, dwPacketSz);
            HEAP_FREE(pbPacket);
            goto _END_OF_FUNC;
        }

        HEAP_FREE(pbPacket);

        StringCchPrintfW(szOutFile, MAX_PATH, L"%s\\" QR_PNG_FMT, wszOutDir, dwI);

        if (!QrRenderToPng(pbQr, szOutFile))
        {
            DBG("[!] Unable To Render Chunk %lu/%lu To PNG: %ls", dwI, dwTotal, szOutFile);
            goto _END_OF_FUNC;
        }

        wprintf(L"  [+] Chunk %lu/%lu : %s\n", dwI + 1, dwTotal, szOutFile);
    }

    wprintf(L"[+] Done.\n[i] %lu PNG(s) Written To: '%s'\n", dwTotal, wszOutDir);
    bResult = TRUE;

_END_OF_FUNC:
    HEAP_FREE(pbPacket);
    HEAP_FREE(pbQr);
    return bResult;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// DECODING COMMAND
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region CMD_DECODE

// Loads a PNG via WIC, feeds it into the quirc decoder, and returns the raw decoded packet
static BOOL QrDecodePng(IN LPCWSTR pwszPng, OUT PBYTE* ppbPacket, OUT PDWORD pcbPacket)
{
    PQR_CTX     pQrCtx      = NULL;
    PQR_CODE    pQrCode     = NULL;
    PQR_DATA    pQrData     = NULL;
    PBYTE       pbPixels    = NULL;
    PBYTE       pbPacket    = NULL;
    DWORD       dwW         = 0x00,
                dwH         = 0x00;
    BOOL        bResult     = FALSE;

    *ppbPacket = NULL;
    *pcbPacket = 0x00;

    HEAP_ALLOC(pQrCtx,  sizeof(QR_CTX));
    if (!pQrCtx)  goto _END_OF_FUNC;
    HEAP_ALLOC(pQrCode, sizeof(QR_CODE));
    if (!pQrCode) goto _END_OF_FUNC;
    HEAP_ALLOC(pQrData, sizeof(QR_DATA));
    if (!pQrData) goto _END_OF_FUNC;

    if (!(pbPixels = WicLoadGrayscale(pwszPng, &dwW, &dwH)))
    {
        DBG("[!] Failed To Load Grayscale Image: %ws", pwszPng);
        goto _END_OF_FUNC;
    }

    if (!QrResize(pQrCtx, (INT)dwW, (INT)dwH))
    {
        DBG("[!] Failed To Resize QR Context To %lux%lu For: %ws", dwW, dwH, pwszPng);
        goto _END_OF_FUNC;
    }

    // Feed image into decoder
    RtlCopyMemory(QrBegin(pQrCtx, NULL, NULL), pbPixels, (SIZE_T)dwW * dwH);
    
    // Run full detection pipeline (threshold, scan, group)
    QrEnd(pQrCtx);

    if (pQrCtx->nGrids == 0x00)
    {
        DBG("[!] Failed To Detect Any QR Codes In: %ws", pwszPng);
        goto _END_OF_FUNC;
    }

    QrExtract(pQrCtx, 0x00, pQrCode);

    if (!QrDecode(pQrCode, pQrData))
    {
        DBG("[!] Failed To Decode QR Code From: %ws", pwszPng);
        goto _END_OF_FUNC;
    }

    HEAP_ALLOC(pbPacket, (DWORD)pQrData->cbPayload);
    if (!pbPacket) goto _END_OF_FUNC;

    RtlCopyMemory(pbPacket, pQrData->abPayload, pQrData->cbPayload);
    
    *ppbPacket  = pbPacket;
    *pcbPacket  = (DWORD)pQrData->cbPayload;
    pbPacket    = NULL;
    bResult     = TRUE;

_END_OF_FUNC:
    if (pQrCtx) QrDestroy(pQrCtx);
    HEAP_FREE(pbPixels);
    HEAP_FREE(pbPacket);
    HEAP_FREE(pQrCtx);
    HEAP_FREE(pQrCode);
    HEAP_FREE(pQrData);
    return bResult;
}

// Scans pwszInDir for Qr*.png files, decodes each QR packet, reassembles the chunks in order and returns the raw output buffer.
BOOL CmdDecode(IN LPCWSTR pwszInDir, OUT PBYTE* ppbOutput, OUT PDWORD pdwOutput, OUT OPTIONAL LPWSTR* ppwszEmbeddedName)
{
    WIN32_FIND_DATAW    w32FindData                 = { 0 };
    HANDLE              hFindFile                   = INVALID_HANDLE_VALUE;
    PBYTE               pbPacket                    = NULL;
    PBYTE               pbOutput                    = NULL;
    PBYTE*              ppbChunks                   = NULL;
    PDWORD              pdwChunkSize                = NULL;
    DWORD               dwTotal                     = 0x00,
                        dwOrigSize                  = 0x00,
                        dwFound                     = 0x00,
                        dwPacketSize                = 0x00;
    WCHAR               wszPattern[MAX_PATH]        = { 0 };
    WCHAR               wszInDir[MAX_PATH]          = { 0 };
    CHAR                szExpectedName[MAX_PATH]    = { 0 };
    WORD                wExpectedNameLen            = 0x00;
    BOOL                bResult                     = FALSE;

    StringCchCopyW(wszInDir, MAX_PATH, pwszInDir);
    PathRemoveBackslashW(wszInDir);
    StringCchPrintfW(wszPattern, MAX_PATH, L"%s\\" QR_PNG_GLOB, wszInDir);

    // Enumerate (1): Probe First File For Chunk Count And Original Size
    if ((hFindFile = FindFirstFileW(wszPattern, &w32FindData)) == INVALID_HANDLE_VALUE)
    {
        DBG_LAST_ERROR("FindFirstFileW");
        goto _END_OF_FUNC;
    }

    {
        WCHAR       wszFirstFile[MAX_PATH]  = { 0 };
        DWORD       dwChunkIndex            = 0x00,
                    dwChunkOrigSize         = 0x00,
                    cbPayload               = 0x00,
                    cbFirstPacket           = 0x00;
        WORD        wNameLen                = 0x00;
        LPCSTR      pszName                 = NULL;
        CONST BYTE* pbPayload               = NULL;

        StringCchPrintfW(wszFirstFile, MAX_PATH, L"%s\\%s", wszInDir, w32FindData.cFileName);

        if (!QrDecodePng(wszFirstFile, &pbPacket, &cbFirstPacket))
        {
            DBG("[!] Failed To Probe First File: %ws", w32FindData.cFileName);
            goto _END_OF_FUNC;
        }

        if (!ParseChunkPacket(pbPacket, cbFirstPacket, &dwTotal, &dwChunkIndex, &dwChunkOrigSize, &pszName, &wNameLen, &pbPayload, &cbPayload))
        {
            DBG("[!] Failed To Parse First Chunk Packet");
            HEAP_FREE(pbPacket);
            goto _END_OF_FUNC;
        }

        dwOrigSize = dwChunkOrigSize;

        // Save expected name for validation
        if (wNameLen > 0 && wNameLen < MAX_PATH)
        {
            RtlCopyMemory(szExpectedName, pszName, wNameLen);
            szExpectedName[wNameLen] = '\0';
            wExpectedNameLen = wNameLen;
        }

        if (ppwszEmbeddedName && wNameLen > 0)
        {
            HEAP_ALLOC(*ppwszEmbeddedName, (wNameLen + 1) * sizeof(WCHAR));
            if (*ppwszEmbeddedName)
            {
                MultiByteToWideChar(CP_UTF8, 0x00, pszName, wNameLen, *ppwszEmbeddedName, wNameLen + 1);
                wprintf(L"[*] Decoding '%s' Into: %s\n", wszPattern, *ppwszEmbeddedName);
            }
        }

        wprintf(L"[i] Expecting %lu Chunk(s)\n[i] Original Size: %lu Bytes\n", dwTotal, dwOrigSize);

        HEAP_FREE(pbPacket);
    }

    FindClose(hFindFile);
    hFindFile = INVALID_HANDLE_VALUE;

    if (dwTotal == 0x00 || dwOrigSize == 0x00)
    {
        DBG("[!] Invalid Chunk Metadata (Total: %lu, OrigSize: %lu)", dwTotal, dwOrigSize);
        goto _END_OF_FUNC;
    }

    // Allocate Chunk Table
    HEAP_ALLOC(ppbChunks, dwTotal * sizeof(PBYTE));
    HEAP_ALLOC(pdwChunkSize, dwTotal * sizeof(DWORD));
    if (!ppbChunks || !pdwChunkSize) goto _END_OF_FUNC;

    // Enumerate (2): Decode All QR Chunks Into Table
    if ((hFindFile = FindFirstFileW(wszPattern, &w32FindData)) == INVALID_HANDLE_VALUE)
        goto _END_OF_FUNC;

    do
    {
        WCHAR       wszPngPath[MAX_PATH]    = { 0 };
        DWORD       dwParsedTotal           = 0x00,
                    dwChunkIndex            = 0x00,
                    dwChunkOrigSize         = 0x00,
                    cbPayload               = 0x00,
                    cbPngPacket             = 0x00;
        WORD        wNameLen                = 0x00;
        LPCSTR      pszName                 = NULL;
        PBYTE       pbPayCopy               = NULL;
        CONST BYTE* pbPayload               = NULL;

        StringCchPrintfW(wszPngPath, MAX_PATH, L"%s\\%s", wszInDir, w32FindData.cFileName);

        if (!QrDecodePng(wszPngPath, &pbPacket, &cbPngPacket))
        {
            DBG("[!] Failed To Decode QR From: %ws", w32FindData.cFileName);
            goto _END_OF_FUNC;
        }

        if (!ParseChunkPacket(pbPacket, cbPngPacket, &dwParsedTotal, &dwChunkIndex, &dwChunkOrigSize, &pszName, &wNameLen, &pbPayload, &cbPayload))
        {
            DBG("[!] Failed To Parse Chunk Packet From: %ws", w32FindData.cFileName);
            HEAP_FREE(pbPacket);
            goto _END_OF_FUNC;
        }


        // Skip chunks that belong to a different file
        if (wExpectedNameLen > 0 && (wNameLen != wExpectedNameLen || RtlCompareMemory(pszName, szExpectedName, wExpectedNameLen) != wExpectedNameLen))
        {
            wprintf(L"  [i] Skipping '%.*hs' Chunk From: %s (Expected '%hs')\n", (int)wNameLen, pszName, w32FindData.cFileName, szExpectedName);
            HEAP_FREE(pbPacket);
            continue;
        }

        // Validate metadata matches the first chunk
        if (dwParsedTotal != dwTotal || dwChunkIndex >= dwTotal)
        {
            wprintf(L"  [-] Metadata Mismatch From: %s (Total: %lu vs %lu, Index: %lu)", w32FindData.cFileName, dwParsedTotal, dwTotal, dwChunkIndex);
            HEAP_FREE(pbPacket);
            continue;
        }

        if (ppbChunks[dwChunkIndex] != NULL)
        {
            wprintf(L"  [-] Duplicate Chunk %lu From: %ws", dwChunkIndex, w32FindData.cFileName);
            HEAP_FREE(pbPacket);
            continue;
        }

        HEAP_ALLOC(pbPayCopy, cbPayload);

        if (pbPayCopy)
        {
            RtlCopyMemory(pbPayCopy, pbPayload, cbPayload);
            ppbChunks[dwChunkIndex]    = pbPayCopy;
            pdwChunkSize[dwChunkIndex] = cbPayload;
            dwFound++;
            wprintf(L"  [+] Chunk %lu/%lu Decoded From: %s\n", dwChunkIndex + 1, dwTotal, w32FindData.cFileName);
        }

        HEAP_FREE(pbPacket);

    } while (FindNextFileW(hFindFile, &w32FindData));

    FindClose(hFindFile);
    hFindFile = INVALID_HANDLE_VALUE;

    // Validate All Chunks Exist
    if (dwFound != dwTotal)
    {
        wprintf(L"[!] Missing Chunks: Found %lu Of %lu\n", dwFound, dwTotal);
        goto _END_OF_FUNC;
    }

    // Reassemble all chunks in order into the output buffer
    HEAP_ALLOC(pbOutput, dwOrigSize);
    if (!pbOutput) goto _END_OF_FUNC;

    {
        PBYTE pbCur = pbOutput;

        for (DWORD dwI = 0x00; dwI < dwTotal; dwI++)
        {
            DWORD dwOffset = (DWORD)(pbCur - pbOutput);
            DWORD cbWrite = pdwChunkSize[dwI];

            if (dwOffset + cbWrite > dwOrigSize)
                cbWrite = dwOrigSize - dwOffset;

            RtlCopyMemory(pbCur, ppbChunks[dwI], cbWrite);
            pbCur += cbWrite;
        }
    }

    wprintf(L"[+] Data Reassembled (%lu Bytes)\n", dwOrigSize);

    *ppbOutput = pbOutput;
    *pdwOutput = dwOrigSize;
    pbOutput   = NULL;
    bResult    = TRUE;

_END_OF_FUNC:
    if (hFindFile != INVALID_HANDLE_VALUE)
        FindClose(hFindFile);
    HEAP_FREE(pbPacket);
    HEAP_FREE(pbOutput);
    if (ppbChunks)
    {
        for (DWORD dwI = 0x00; dwI < dwTotal; dwI++)
            HEAP_FREE(ppbChunks[dwI]);
        HEAP_FREE(ppbChunks);
    }
    HEAP_FREE(pdwChunkSize);
    return bResult;
}

#pragma endregion