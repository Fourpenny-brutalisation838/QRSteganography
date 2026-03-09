// Based on:
// * https://github.com/nayuki/QR-Code-generator/tree/master/c   (encode)
// * https://github.com/dlbeer/quirc                             (decode)

#pragma once
#ifndef QRINTERNALS_H
#define QRINTERNALS_H

#include <Windows.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <initguid.h>
#include <wincodec.h>
#include <ole2.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#define _DBG_FORCE
#define _DBG_USE_DEBUGSTR
#include <DebugMacros.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Windowscodecs.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Crypt32.lib")

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Tunable
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_ECC_LEVEL                    0                       // Active ECC level for chunk encoding. 0=Low, 1=Medium, 2=Quartile, 3=High

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Encode types
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// Error correction level 
typedef enum _QR_ECC
{
    QR_ECC_LOW      = 0,    //  ~7%  error correction
    QR_ECC_MEDIUM   = 1,    // ~15%  error correction
    QR_ECC_QUARTILE = 2,    // ~25%  error correction
    QR_ECC_HIGH     = 3,    // ~30%  error correction

} QR_ECC;

#define QR_ECC_LEVEL_COUNT              4                              // number of ECC levels (L, M, Q, H)

// V40 byte-mode capacity per ECC level
// https://www.qrcode.com/en/about/version.html
#define _QR_V40_CAP_0                   2953                           // Low       (~7%)  
#define _QR_V40_CAP_1                   2331                           // Medium    (~15%)
#define _QR_V40_CAP_2                   1663                           // Quartile  (~25%)
#define _QR_V40_CAP_3                   1273                           // High      (~30%)

#define _QR_V40_CAP_PASTE(x)           _QR_V40_CAP_##x
#define _QR_V40_CAP(x)                 _QR_V40_CAP_PASTE(x)
#define QR_V40_CAPACITY                _QR_V40_CAP(QR_ECC_LEVEL)       // auto-selected from QR_ECC_LEVEL


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Chunk packet format
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

/*
    Packet layout: [magic:4][total:4][index:4][origSize:4][nameLen:2][name:nameLen][payload:...]

    typedef struct _CHUNK_HDR
    {
        BYTE    abMagic[4];
        DWORD   dwTotalChunks;
        DWORD   dwChunkIndex;
        DWORD   dwOrigSize;
        WORD    wNameLen;
        // variable: abName[wNameLen] then abPayload[...]

    } CHUNK_HDR, *PCHUNK_HDR;
*/


#define CHUNK_MAGIC                     "QRFL"                                                          // 4-byte signature at the start of every chunk packet
#define CHUNK_MAGIC_LEN                 (sizeof(CHUNK_MAGIC) - 1)                                       // length of the magic signature (excludes null terminator)

// [magic:4] + [total:4] + [index:4] + [origSize:4] + [nameLen:2]
#define CHUNK_HDR_FIXED_SIZE            (CHUNK_MAGIC_LEN + sizeof(DWORD) * 3 + sizeof(WORD))            // bytes before the variable-length name field - 18

#define CHUNK_NAME_MAX                  64                                                              // max assumed filename length in chunk packets
#define CHUNK_PAYLOAD_MAX               (QR_V40_CAPACITY - CHUNK_HDR_FIXED_SIZE - CHUNK_NAME_MAX)       // max raw data bytes per chunk (after header overhead) - 2871

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  QR version and grid geometry
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_VERSION_MIN                  1                              // smallest QR version
#define QR_VERSION_MAX                  40                             // largest QR version
#define QR_VER_INCREMENT                4                              // modules added to grid size per version step
#define QR_BASE_SIZE                    17                             // grid size at version 0 (version 1 = 21 modules)
#define QR_MAX_GRID_SIZE                (QR_VERSION_MAX * QR_VER_INCREMENT + QR_BASE_SIZE)  // 177 modules at version 40

#define QR_MIN_ALIGN_VERSION            2                              // first version with alignment patterns
#define QR_MIN_VERSION_INFO             7                              // first version with version info blocks
#define QR_VER_MEDIUM_THRESHOLD         10                             // versions below this use shorter character count indicators
#define QR_VER_LARGE_THRESHOLD          27                             // versions at or above this use the longest character count indicators

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  QR structural dimensions (finders, timing, alignment, version info)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_FINDER_SIZE                  7                              // finder pattern width/height in modules
#define QR_FINDER_CENTER                ((QR_FLOAT)QR_FINDER_SIZE / 2) // 3.5 — capstone center coordinate
#define QR_FINDER_TOTAL                 9                              // finder + separator + format area (7 + 1 + 1)
#define QR_FINDER_SEGMENTS              5                              // number of runs in the 1:1:3:1:1 finder pattern

#define QR_TIMING_POS                   6                              // row/column coordinate of the timing patterns

#define QR_ALIGN_SIZE                   5                              // alignment pattern diameter in modules
#define QR_ALIGN_MARGIN                 2                              // alignment pattern radius from center

#define QR_VERINFO_OFFSET               11                             // version info block distance from grid edge
#define QR_VERINFO_WIDTH                3                              // version info block width in modules
#define QR_VERSION_INFO_MODULES         36                             // total modules consumed by both version info blocks (2 x 3 x 6)

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  BCH error correction polynomials and masks
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_BCH_FORMAT_BITS              10                             // number of ECC bits for format info
#define QR_BCH_FORMAT_GEN               0x537                          // BCH(15,5) generator polynomial for format info
#define QR_FORMAT_XOR_MASK              0x5412                         // fixed XOR mask applied to format info bits

#define QR_BCH_VERSION_BITS             12                             // number of ECC bits for version info
#define QR_BCH_VERSION_GEN              0x1F25                         // BCH(18,6) generator polynomial for version info

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Galois field constants
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_GF256_POLY                   0x11D                          // GF(2^8) reducing polynomial (x^8 + x^4 + x^3 + x^2 + 1)
#define QR_GF256_ALPHA                  0x02                           // GF(2^8) primitive element (generator root)
#define QR_GF256_ORDER                  255                            // GF(2^8) field order (2^8 - 1)
#define QR_GF16_ORDER                   15                             // GF(2^4) field order (2^4 - 1)

#define GF_POLY_MAX                     64                             // max polynomial coefficient array size for GF operations

#define FMT_MAX_ERROR                   3                              // max correctable errors in format info
#define FMT_SYNDROMES                   (FMT_MAX_ERROR * 2)            // number of syndromes for format ECC (6)
#define FMT_BITS                        15                             // total bits in the format info word

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Reed-Solomon and encode bitstream constants
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_RS_DEGREE_MAX                30                             // max Reed-Solomon generator polynomial degree
#define QR_LENGTH_OVERFLOW              (-1)                           // sentinel returned when bit length exceeds capacity
#define QR_MAX_BIT_LEN                  0x7FFF                         // maximum representable bit length before overflow

#define QR_MODE_BYTE                    0x4                            // byte mode indicator value (4-bit field)
#define QR_MODE_INDICATOR_BITS          4                              // mode indicator bit count (always 4 bits in QR)
#define QR_PAD_BYTE_A                   0xEC                           // first alternating pad codeword
#define QR_PAD_BYTE_B                   0x11                           // second alternating pad codeword

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Data mode types and character sets
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_DATA_TYPE_NUMERIC            1                              // numeric mode (0-9 only)
#define QR_DATA_TYPE_ALPHA              2                              // alphanumeric mode (0-9, A-Z, space, $%*+-./:)
#define QR_DATA_TYPE_BYTE               4                              // byte mode (raw 8-bit data)
#define QR_DATA_TYPE_KANJI              8                              // Kanji mode (Shift-JIS double-byte)
#define QR_DATA_TYPE_ECI                7                              // ECI mode indicator

#define QR_ALPHA_CHARSET_SIZE           45                             // number of characters in the alphanumeric mode set

#define QR_KANJI_DIVISOR                0xC0                           // Shift-JIS byte pair modular divisor (192)
#define QR_SJIS_LOW_BASE                0x8140                         // Shift-JIS low range base offset
#define QR_SJIS_LOW_LIMIT               0x9FFC                         // Shift-JIS low range upper bound
#define QR_SJIS_HIGH_BASE               0xC140                         // Shift-JIS high range base offset

#define QR_ECI_2BYTE_MASK               0xC0                           // ECI 2-byte prefix test mask
#define QR_ECI_2BYTE_TAG                0x80                           // ECI 2-byte prefix expected tag
#define QR_ECI_3BYTE_MASK               0xE0                           // ECI 3-byte prefix test mask
#define QR_ECI_3BYTE_TAG                0xC0                           // ECI 3-byte prefix expected tag

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Mask selection and penalty scoring
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_NUM_MASKS                    8                              // total QR data mask patterns (0-7)
#define QR_MASK_AUTO                    (-1)                           // sentinel for automatic mask selection

#define QR_PENALTY_N1                   3                              // penalty weight: run of 5+ same-color modules
#define QR_PENALTY_N2                   3                              // penalty weight: 2x2 same-color block
#define QR_PENALTY_N3                   40                             // penalty weight: finder-like 1:1:3:1:1 pattern
#define QR_PENALTY_N4                   10                             // penalty weight: dark/light ratio deviation from 50%
#define QR_PENALTY_HISTORY              7                              // run-length history buffer size for penalty rule 3
#define QR_PENALTY_RUN_MIN              5                              // minimum same-color run length to trigger penalty rule 1

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Encode output and buffer sizing
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_BUFFER_LEN_FOR_VERSION(n)    ((((n) * QR_VER_INCREMENT + QR_BASE_SIZE) * ((n) * QR_VER_INCREMENT + QR_BASE_SIZE) + 7) / 8 + 1) // packed bit buffer size for version n
#define QR_BUFFER_LEN_MAX               QR_BUFFER_LEN_FOR_VERSION(QR_VERSION_MAX)         

#define QR_MODULE_PX                    8                              // pixels per QR module in output PNG
#define QR_QUIET_ZONE_SIDES             2                              // quiet zone on both edges (left+right or top+bottom)

#define QR_PNG_FMT                      L"Qr%04lu.png"                 // output PNG filename format string
#define QR_PNG_GLOB                     L"Qr*.png"                     // glob pattern for finding QR PNG files

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Decode context limits
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_ASSERT(x)                    assert(x)

#define QR_PIXEL_WHITE                  0                              // binarised pixel value: light module
#define QR_PIXEL_BLACK                  1                              // binarised pixel value: dark module
#define QR_PIXEL_REGION                 2                              // first valid region label (0 and 1 are reserved for pixels)

#define QR_MAX_REGIONS                  4096                           // max connected component regions per frame
#define QR_MAX_CAPSTONES                32                             // max finder pattern capstones per frame
#define QR_MAX_GRIDS                    (QR_MAX_CAPSTONES * 2)         // max QR grid candidates per frame
#define QR_MAX_ALIGNMENT                7                              // max alignment pattern positions per version
#define QR_MAX_PAYLOAD                  8896                           // max decoded payload bytes (version 40, low ECC)

#define QR_PERSPECTIVE_PARAMS           8                              // number of coefficients in the perspective transform

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Decode tuning thresholds
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define QR_SQUARENESS_THRESHOLD         ((QR_FLOAT)0.2)                // max H/V distance ratio deviation for neighbour/grid candidates
#define QR_CAPSTONE_RATIO_MIN           10                             // min stone/ring area percentage for capstone validation
#define QR_CAPSTONE_RATIO_MAX           70                             // max stone/ring area percentage for capstone validation

#define QR_JIGGLE_INIT_STEP             0.02                           // initial perspective nudge step (2% of coefficient value)
#define QR_JIGGLE_DECAY                 0.5                            // step size halving factor per refinement pass
#define QR_JIGGLE_PASSES                5                              // number of perspective refinement passes

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  BMP constants (used by alternate output path)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#define BMP_SIGNATURE                   0x4D42                         // 'BM' file header magic
#define BMP_BYTES_PER_PIXEL             3                              // 24-bit RGB: B, G, R
#define BMP_STRIDE_ALIGN_MASK           3                              // BMP rows padded to 4-byte boundary (4 - 1)


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Decode types
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

typedef WORD    QR_PIXEL_T;     // pixel/region label type (must hold values up to QR_MAX_REGIONS)
typedef double  QR_FLOAT;       // floating-point type used throughout the decoder

// Galois field descriptor — log/exp tables and field order for GF(2^n) arithmetic.
typedef struct _GF_FIELD
{
    INT         nP;             // field order (2^n - 1)
    const BYTE* pbLog;          // discrete logarithm table
    const BYTE* pbExp;          // exponentiation (antilog) table

} GF_FIELD, *PGF_FIELD;

// Integer 2D point used for pixel coordinates and grid positions.
typedef struct _QR_POINT
{
    INT nX;
    INT nY;

} QR_POINT, *PQR_POINT;

// Connected component region found during flood fill.
typedef struct _QR_REGION
{
    QR_POINT    ptSeed;         // seed pixel that started the flood fill
    INT         nCount;         // total pixel count in this region
    INT         nCapstone;      // index into aCapstones[], or -1 if unclaimed

} QR_REGION, *PQR_REGION;

// Finder pattern capstone — a validated ring + stone pair with its perspective transform.
typedef struct _QR_CAPSTONE
{
    INT         nRing;          // region index of the outer ring
    INT         nStone;         // region index of the inner stone
    QR_POINT    aptCorners[4];  // four corners in image coordinates (clockwise from top-left)
    QR_POINT    ptCenter;       // center point in image coordinates
    QR_FLOAT    adPerspective[QR_PERSPECTIVE_PARAMS]; // 8-coeff perspective transform for this capstone
    INT         nGrid;          // index into aGrids[], or -1 if not yet assigned

} QR_CAPSTONE, *PQR_CAPSTONE;

// QR grid candidate — three capstones plus alignment info and perspective transform.
typedef struct _QR_GRID
{
    INT         anCaps[3];      // capstone indices: [0]=top-left, [1]=top-right, [2]=bottom-left
    INT         nAlignRegion;   // region index of the bottom-right alignment pattern, or -1
    QR_POINT    ptAlign;        // estimated or detected alignment pattern position
    INT         nSize;          // grid side length in modules
    QR_FLOAT    adPerspective[QR_PERSPECTIVE_PARAMS]; // 8-coeff perspective transform for the full grid

} QR_GRID, *PQR_GRID;

// Stack frame for the iterative flood fill algorithm.
typedef struct _QR_FLOOD_VARS
{
    INT nY;                     // row of the current span
    INT nRight;                 // rightmost column of the current span
    INT nLeftUp;                // scan cursor for the row above
    INT nLeftDown;              // scan cursor for the row below

} QR_FLOOD_VARS, *PQR_FLOOD_VARS;

// Top-level decode context — holds the image, pixel map, regions, capstones, and grids.
typedef struct _QR_CTX
{
    PBYTE           pbImage;                        // raw grayscale image buffer (8bpp)
    QR_PIXEL_T      *pbPixels;                      // binarised pixel/region label map
    INT             nWidth;                         // image width in pixels
    INT             nHeight;                        // image height in pixels
    INT             nRegions;                       // current region count (starts at QR_PIXEL_REGION)
    QR_REGION       aRegions[QR_MAX_REGIONS];       // connected component region table
    INT             nCapstones;                     // number of detected capstones
    QR_CAPSTONE     aCapstones[QR_MAX_CAPSTONES];   // detected capstone table
    INT             nGrids;                         // number of assembled grid candidates
    QR_GRID         aGrids[QR_MAX_GRIDS];           // assembled grid table
    SIZE_T          cbFloodVars;                    // number of entries in the flood fill stack
    PQR_FLOOD_VARS  pFloodVars;                     // heap-allocated flood fill stack

} QR_CTX, *PQR_CTX;

// Reed-Solomon block parameters — block size, data codewords, and block count.
typedef struct _QR_RS_PARAMS
{
    INT nBs;                    // total block size (data + ECC codewords)
    INT nDw;                    // data codewords per block
    INT nNs;                    // number of blocks with this configuration

} QR_RS_PARAMS, *PQR_RS_PARAMS;

// Per-version QR database entry — total capacity, alignment positions, and ECC layout.
typedef struct _QR_VER_INFO
{
    INT             nDataBytes;                     // total data codewords for this version
    INT             anAPat[QR_MAX_ALIGNMENT];       // alignment pattern center coordinates (0-terminated)
    QR_RS_PARAMS    aEcc[QR_ECC_LEVEL_COUNT];       // ECC block params indexed by QR_ECC level

} QR_VER_INFO, *PQR_VER_INFO;

// Global version database (indexed by version 0..40, where 0 is unused).
extern const QR_VER_INFO g_aQrVersionDb[QR_VERSION_MAX + 1];

// Decoded QR grid — raw cell bits extracted via perspective sampling.
typedef struct _QR_CODE
{
    QR_POINT    aptCorners[4];  // four corners in image coordinates
    INT         nSize;          // grid side length in modules
    BYTE        abCells[(QR_MAX_GRID_SIZE * QR_MAX_GRID_SIZE + 7) / 8]; // packed bit array of module values

} QR_CODE, *PQR_CODE;

// Final decoded QR payload — version, ECC level, mask, data type, and the output bytes.
typedef struct _QR_DATA
{
    INT     nVersion;           // detected QR version (1-40)
    INT     nEccLevel;          // detected ECC level (0-3, maps to QR_ECC)
    INT     nMask;              // detected data mask pattern (0-7)
    INT     nDataType;          // highest data mode encountered (numeric/alpha/byte/kanji)
    BYTE    abPayload[QR_MAX_PAYLOAD]; // decoded payload bytes (null-terminated)
    INT     cbPayload;          // number of valid bytes in abPayload
    DWORD   dwEci;              // ECI assignment number, if present

} QR_DATA, *PQR_DATA;

// Intermediate bitstream used during data extraction and ECC correction.
typedef struct _QR_DATASTREAM
{
    PBYTE   pbRaw;              // pointer to raw (pre-ECC) bit buffer, or NULL after correction
    INT     nDataBits;          // total number of data bits available
    INT     nPtr;               // current read position in bits
    BYTE    abData[QR_MAX_PAYLOAD]; // corrected data codewords (post-ECC)

} QR_DATASTREAM, *PQR_DATASTREAM;

// Scoring state for region corner detection via flood fill callbacks.
typedef struct _QR_POLY_SCORE
{
    QR_POINT    ptRef;          // reference point or direction vector
    INT         anScores[4];    // projection scores for up to 4 corner candidates
    PQR_POINT   pCorners;       // output corner point array

} QR_POLY_SCORE, *PQR_POLY_SCORE;

// A neighbouring capstone candidate with its distance from the reference.
typedef struct _QR_NEIGHBOUR
{
    INT         nIndex;         // capstone index in aCapstones[]
    QR_FLOAT    dDistance;      // distance in perspective-normalised units

} QR_NEIGHBOUR, *PQR_NEIGHBOUR;

// List of neighbouring capstones (horizontal or vertical) for grid assembly.
typedef struct _QR_NEIGHBOUR_LIST
{
    QR_NEIGHBOUR    aN[QR_MAX_CAPSTONES]; // neighbour entries
    INT             nCount;               // number of valid entries

} QR_NEIGHBOUR_LIST, *PQR_NEIGHBOUR_LIST;

// Callback invoked for each horizontal span during flood fill.
typedef VOID(* QR_SPAN_FUNC)(IN PVOID pvUserData, IN INT nY, IN INT nLeft, IN INT nRight);

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Encode API
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// Encodes raw data into a QR code, auto-selecting the best version and mask within the given range.
BOOL  QrEncode(IN CONST BYTE* pbData, IN DWORD cbData, OUT PBYTE pbQrCode, IN QR_ECC eEcc, IN INT nMinVersion, IN INT nMaxVersion);

// Returns the side length (in modules) of an encoded QR code buffer.
INT  QrGetSize(IN const BYTE* pbQrCode);

// Returns the module value (dark/light) at (nX, nY) with bounds checking.
BOOL  QrGetModule(IN const BYTE* pbQrCode, IN INT nX, IN INT nY);

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  Decode API
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// Allocates or resizes decode context buffers (image, pixel map, flood fill stack) for the given dimensions.
BOOL QrResize(IN OUT PQR_CTX pCtx, IN INT nWidth, IN INT nHeight);

// Frees all heap allocations owned by the decode context.
VOID QrDestroy(IN OUT PQR_CTX pCtx);

// Resets the decode context for a new frame and returns a pointer to the grayscale image buffer.
PBYTE QrBegin(IN OUT PQR_CTX pCtx, OUT OPTIONAL PINT pnWidth, OUT OPTIONAL PINT pnHeight);

// Runs the full decode pipeline: Otsu threshold, finder scan, and capstone grouping.
VOID QrEnd(IN OUT PQR_CTX pCtx);

// Extracts the raw bit grid for the QR code at nIndex via perspective sampling.
VOID QrExtract(IN PQR_CTX pCtx, IN INT nIndex, OUT PQR_CODE pCode);

// Decodes a QR_CODE grid into payload data: reads format info, corrects ECC, and parses data segments.
BOOL QrDecode(IN PQR_CODE pCode, OUT PQR_DATA pData);

// Mirrors the QR grid along the diagonal for retry after a failed decode.
// VOID QrFlip(IN OUT PQR_CODE pCode);

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
//  High-level encode/decode commands
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

// Splits input data into chunks, encodes each as a QR code, and writes numbered PNGs to pwszOutDir.
BOOL CmdEncode(IN PBYTE pbData, IN DWORD cbData, IN LPCWSTR pwszName, IN LPCWSTR pwszOutDir);

// Scans pwszInDir for QR PNG files, decodes each, and reassembles the original data buffer.
BOOL CmdDecode(IN LPCWSTR pwszInDir, OUT PBYTE* ppbOutput, OUT PDWORD pdwOutput, OUT OPTIONAL LPWSTR* ppwszEmbeddedName);


#endif // !QRINTERNALS_H