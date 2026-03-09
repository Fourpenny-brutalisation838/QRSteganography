// Based on:
// * https://github.com/nayuki/QR-Code-generator/tree/master/c   (encode)
// * https://github.com/dlbeer/quirc                             (decode)

#include "QrInternals.h"


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// GLOBAL VARIABLES
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region GLOBAL_TABLES

// QR version database: total codewords, alignment pattern positions, and ECC block layout per version (ISO 18004:2015).
// https://github.com/dlbeer/quirc/blob/master/lib/version_db.c
const QR_VER_INFO g_aQrVersionDb[QR_VERSION_MAX + 1] =
{
    { 0, { 0 }, { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } } },

    // Version 1
    {    26, { 0, 0, 0, 0, 0, 0, 0 },
        { {  26,  16,  1 }, {  26,  19,  1 }, {  26,   9,  1 }, {  26,  13,  1 } } },
    // Version 2
    {    44, { 6, 18, 0, 0, 0, 0, 0 },
        { {  44,  28,  1 }, {  44,  34,  1 }, {  44,  16,  1 }, {  44,  22,  1 } } },
    // Version 3
    {    70, { 6, 22, 0, 0, 0, 0, 0 },
        { {  70,  44,  1 }, {  70,  55,  1 }, {  35,  13,  2 }, {  35,  17,  2 } } },
    // Version 4
    {   100, { 6, 26, 0, 0, 0, 0, 0 },
        { {  50,  32,  2 }, { 100,  80,  1 }, {  25,   9,  4 }, {  50,  24,  2 } } },
    // Version 5
    {   134, { 6, 30, 0, 0, 0, 0, 0 },
        { {  67,  43,  2 }, { 134, 108,  1 }, {  33,  11,  2 }, {  33,  15,  2 } } },
    // Version 6
    {   172, { 6, 34, 0, 0, 0, 0, 0 },
        { {  43,  27,  4 }, {  86,  68,  2 }, {  43,  15,  4 }, {  43,  19,  4 } } },
    // Version 7
    {   196, { 6, 22, 38, 0, 0, 0, 0 },
        { {  49,  31,  4 }, {  98,  78,  2 }, {  39,  13,  4 }, {  32,  14,  2 } } },
    // Version 8
    {   242, { 6, 24, 42, 0, 0, 0, 0 },
        { {  60,  38,  2 }, { 121,  97,  2 }, {  40,  14,  4 }, {  40,  18,  4 } } },
    // Version 9
    {   292, { 6, 26, 46, 0, 0, 0, 0 },
        { {  58,  36,  3 }, { 146, 116,  2 }, {  36,  12,  4 }, {  36,  16,  4 } } },
    // Version 10
    {   346, { 6, 28, 50, 0, 0, 0, 0 },
        { {  69,  43,  4 }, {  86,  68,  2 }, {  43,  15,  6 }, {  43,  19,  6 } } },
    // Version 11
    {   404, { 6, 30, 54, 0, 0, 0, 0 },
        { {  80,  50,  1 }, { 101,  81,  4 }, {  36,  12,  3 }, {  50,  22,  4 } } },
    // Version 12
    {   466, { 6, 32, 58, 0, 0, 0, 0 },
        { {  58,  36,  6 }, { 116,  92,  2 }, {  42,  14,  7 }, {  46,  20,  4 } } },
    // Version 13
    {   532, { 6, 34, 62, 0, 0, 0, 0 },
        { {  59,  37,  8 }, { 133, 107,  4 }, {  33,  11, 12 }, {  44,  20,  8 } } },
    // Version 14
    {   581, { 6, 26, 46, 66, 0, 0, 0 },
        { {  64,  40,  4 }, { 145, 115,  3 }, {  36,  12, 11 }, {  36,  16, 11 } } },
    // Version 15
    {   655, { 6, 26, 48, 70, 0, 0, 0 },
        { {  65,  41,  5 }, { 109,  87,  5 }, {  36,  12, 11 }, {  54,  24,  5 } } },
    // Version 16
    {   733, { 6, 26, 50, 74, 0, 0, 0 },
        { {  73,  45,  7 }, { 122,  98,  5 }, {  45,  15,  3 }, {  43,  19, 15 } } },
    // Version 17
    {   815, { 6, 30, 54, 78, 0, 0, 0 },
        { {  74,  46, 10 }, { 135, 107,  1 }, {  42,  14,  2 }, {  50,  22,  1 } } },
    // Version 18
    {   901, { 6, 30, 56, 82, 0, 0, 0 },
        { {  69,  43,  9 }, { 150, 120,  5 }, {  42,  14,  2 }, {  50,  22, 17 } } },
    // Version 19
    {   991, { 6, 30, 58, 86, 0, 0, 0 },
        { {  70,  44,  3 }, { 141, 113,  3 }, {  39,  13,  9 }, {  47,  21, 17 } } },
    // Version 20
    {  1085, { 6, 34, 62, 90, 0, 0, 0 },
        { {  67,  41,  3 }, { 135, 107,  3 }, {  43,  15, 15 }, {  54,  24, 15 } } },
    // Version 21
    {  1156, {6, 28, 50, 72, 92, 0 },
        { {  68,  42, 17 }, { 144, 116,  4 }, {  46,  16, 19 }, {  50,  22, 17 } } },
    // Version 22
    {  1258, { 6, 26, 50, 74, 98, 0, 0 },
        { {  74,  46, 17 }, { 139, 111,  2 }, {  37,  13, 34 }, {  54,  24,  7 } } },
    // Version 23
    {  1364, { 6, 30, 54, 78, 102, 0, 0 },
        { {  75,  47,  4 }, { 151, 121,  4 }, {  45,  15, 16 }, {  54,  24, 11 } } },
    // Version 24
    {  1474, { 6, 28, 54, 80, 106, 0, 0 },
        { {  73,  45,  6 }, { 147, 117,  6 }, {  46,  16, 30 }, {  54,  24, 11 } } },
    // Version 25
    {  1588, { 6, 32, 58, 84, 110, 0, 0 },
        { {  75,  47,  8 }, { 132, 106,  8 }, {  45,  15, 22 }, {  54,  24,  7 } } },
    // Version 26
    {  1706, { 6, 30, 58, 86, 114, 0, 0 },
        { {  74,  46, 19 }, { 142, 114, 10 }, {  46,  16, 33 }, {  50,  22, 28 } } },
    // Version 27
    {  1828, { 6, 34, 62, 90, 118, 0, 0 },
        { {  73,  45, 22 }, { 152, 122,  8 }, {  45,  15, 12 }, {  53,  23,  8 } } },
    // Version 28
    {  1921, { 6, 26, 50, 74, 98, 122, 0 },
        { {  73,  45,  3 }, { 147, 117,  3 }, {  45,  15, 11 }, {  54,  24,  4 } } },
    // Version 29
    {  2051, { 6, 30, 54, 78, 102, 126, 0 },
        { {  73,  45, 21 }, { 146, 116,  7 }, {  45,  15, 19 }, {  53,  23,  1 } } },
    // Version 30
    {  2185, { 6, 26, 52, 78, 104, 130, 0 },
        { {  75,  47, 19 }, { 145, 115,  5 }, {  45,  15, 23 }, {  54,  24, 15 } } },
    // Version 31
    {  2323, { 6, 30, 56, 82, 108, 134, 0 },
        { {  74,  46,  2 }, { 145, 115, 13 }, {  45,  15, 23 }, {  54,  24, 42 } } },
    // Version 32
    {  2465, { 6, 34, 60, 86, 112, 138, 0 },
        { {  74,  46, 10 }, { 145, 115, 17 }, {  45,  15, 19 }, {  54,  24, 10 } } },
    // Version 33
    {  2611, { 6, 30, 58, 86, 114, 142, 0 },
        { {  74,  46, 14 }, { 145, 115, 17 }, {  45,  15, 11 }, {  54,  24, 29 } } },
    // Version 34
    {  2761, { 6, 34, 62, 90, 118, 146, 0 },
        { {  74,  46, 14 }, { 145, 115, 13 }, {  46,  16, 59 }, {  54,  24, 44 } } },
    // Version 35
    {  2876, { 6, 30, 54, 78, 102, 126, 150 },
        { {  75,  47, 12 }, { 151, 121, 12 }, {  45,  15, 22 }, {  54,  24, 39 } } },
    // Version 36
    {  3034, { 6, 24, 50, 76, 102, 128, 154 },
        { {  75,  47,  6 }, { 151, 121,  6 }, {  45,  15,  2 }, {  54,  24, 46 } } },
    // Version 37
    {  3196, { 6, 28, 54, 80, 106, 132, 158 },
        { {  74,  46, 29 }, { 152, 122, 17 }, {  45,  15, 24 }, {  54,  24, 49 } } },
    // Version 38
    {  3362, { 6, 32, 58, 84, 110, 136, 162 },
        { {  74,  46, 13 }, { 152, 122,  4 }, {  45,  15, 42 }, {  54,  24, 48 } } },
    // Version 39
    {  3532, { 6, 26, 54, 82, 110, 138, 166 },
        { {  75,  47, 40 }, { 147, 117, 20 }, {  45,  15, 10 }, {  54,  24, 43 } } },
    // Version 40
    {  3706, { 6, 30, 58, 86, 114, 142, 170 },
        { {  75,  47, 18 }, { 148, 118, 19 }, {  45,  15, 20 }, {  54,  24, 34 } } },
};

// Number of ECC codewords per block
static const INT8 g_anEccCodewordsPerBlock[QR_ECC_LEVEL_COUNT][QR_VERSION_MAX + 1] =
{
    // Low
    { -1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
    // Medium
    { -1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28 },
    // Quartile
    { -1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
    // High
    { -1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
};

// Number of ECC blocks
static const INT8 g_anNumEccBlocks[QR_ECC_LEVEL_COUNT][QR_VERSION_MAX + 1] =
{
    // Low
    { -1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25 },
    // Medium
    { -1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49 },
    // Quartile
    { -1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68 },
    // High
    { -1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81 },
};

// GF(2^4) exponentiation (antilog) table.
static const BYTE g_abGf16Exp[16] =
{
    0x01, 0x02, 0x04, 0x08, 0x03, 0x06, 0x0c, 0x0b,
    0x05, 0x0a, 0x07, 0x0e, 0x0f, 0x0d, 0x09, 0x01
};

// GF(2^4) discrete logarithm table.
static const BYTE g_abGf16Log[16] =
{
    0x00, 0x0f, 0x01, 0x04, 0x02, 0x08, 0x05, 0x0a,
    0x03, 0x0e, 0x09, 0x07, 0x06, 0x0d, 0x0b, 0x0c
};

// GF(2^8) exponentiation (antilog) table.
static const BYTE g_abGf256Exp[256] =
{
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    0x1d, 0x3a, 0x74, 0xe8, 0xcd, 0x87, 0x13, 0x26,
    0x4c, 0x98, 0x2d, 0x5a, 0xb4, 0x75, 0xea, 0xc9,
    0x8f, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0,
    0x9d, 0x27, 0x4e, 0x9c, 0x25, 0x4a, 0x94, 0x35,
    0x6a, 0xd4, 0xb5, 0x77, 0xee, 0xc1, 0x9f, 0x23,
    0x46, 0x8c, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0,
    0x5d, 0xba, 0x69, 0xd2, 0xb9, 0x6f, 0xde, 0xa1,
    0x5f, 0xbe, 0x61, 0xc2, 0x99, 0x2f, 0x5e, 0xbc,
    0x65, 0xca, 0x89, 0x0f, 0x1e, 0x3c, 0x78, 0xf0,
    0xfd, 0xe7, 0xd3, 0xbb, 0x6b, 0xd6, 0xb1, 0x7f,
    0xfe, 0xe1, 0xdf, 0xa3, 0x5b, 0xb6, 0x71, 0xe2,
    0xd9, 0xaf, 0x43, 0x86, 0x11, 0x22, 0x44, 0x88,
    0x0d, 0x1a, 0x34, 0x68, 0xd0, 0xbd, 0x67, 0xce,
    0x81, 0x1f, 0x3e, 0x7c, 0xf8, 0xed, 0xc7, 0x93,
    0x3b, 0x76, 0xec, 0xc5, 0x97, 0x33, 0x66, 0xcc,
    0x85, 0x17, 0x2e, 0x5c, 0xb8, 0x6d, 0xda, 0xa9,
    0x4f, 0x9e, 0x21, 0x42, 0x84, 0x15, 0x2a, 0x54,
    0xa8, 0x4d, 0x9a, 0x29, 0x52, 0xa4, 0x55, 0xaa,
    0x49, 0x92, 0x39, 0x72, 0xe4, 0xd5, 0xb7, 0x73,
    0xe6, 0xd1, 0xbf, 0x63, 0xc6, 0x91, 0x3f, 0x7e,
    0xfc, 0xe5, 0xd7, 0xb3, 0x7b, 0xf6, 0xf1, 0xff,
    0xe3, 0xdb, 0xab, 0x4b, 0x96, 0x31, 0x62, 0xc4,
    0x95, 0x37, 0x6e, 0xdc, 0xa5, 0x57, 0xae, 0x41,
    0x82, 0x19, 0x32, 0x64, 0xc8, 0x8d, 0x07, 0x0e,
    0x1c, 0x38, 0x70, 0xe0, 0xdd, 0xa7, 0x53, 0xa6,
    0x51, 0xa2, 0x59, 0xb2, 0x79, 0xf2, 0xf9, 0xef,
    0xc3, 0x9b, 0x2b, 0x56, 0xac, 0x45, 0x8a, 0x09,
    0x12, 0x24, 0x48, 0x90, 0x3d, 0x7a, 0xf4, 0xf5,
    0xf7, 0xf3, 0xfb, 0xeb, 0xcb, 0x8b, 0x0b, 0x16,
    0x2c, 0x58, 0xb0, 0x7d, 0xfa, 0xe9, 0xcf, 0x83,
    0x1b, 0x36, 0x6c, 0xd8, 0xad, 0x47, 0x8e, 0x01
};

// GF(2^8) discrete logarithm table.
static const BYTE g_abGf256Log[256] =
{
    0x00, 0xff, 0x01, 0x19, 0x02, 0x32, 0x1a, 0xc6,
    0x03, 0xdf, 0x33, 0xee, 0x1b, 0x68, 0xc7, 0x4b,
    0x04, 0x64, 0xe0, 0x0e, 0x34, 0x8d, 0xef, 0x81,
    0x1c, 0xc1, 0x69, 0xf8, 0xc8, 0x08, 0x4c, 0x71,
    0x05, 0x8a, 0x65, 0x2f, 0xe1, 0x24, 0x0f, 0x21,
    0x35, 0x93, 0x8e, 0xda, 0xf0, 0x12, 0x82, 0x45,
    0x1d, 0xb5, 0xc2, 0x7d, 0x6a, 0x27, 0xf9, 0xb9,
    0xc9, 0x9a, 0x09, 0x78, 0x4d, 0xe4, 0x72, 0xa6,
    0x06, 0xbf, 0x8b, 0x62, 0x66, 0xdd, 0x30, 0xfd,
    0xe2, 0x98, 0x25, 0xb3, 0x10, 0x91, 0x22, 0x88,
    0x36, 0xd0, 0x94, 0xce, 0x8f, 0x96, 0xdb, 0xbd,
    0xf1, 0xd2, 0x13, 0x5c, 0x83, 0x38, 0x46, 0x40,
    0x1e, 0x42, 0xb6, 0xa3, 0xc3, 0x48, 0x7e, 0x6e,
    0x6b, 0x3a, 0x28, 0x54, 0xfa, 0x85, 0xba, 0x3d,
    0xca, 0x5e, 0x9b, 0x9f, 0x0a, 0x15, 0x79, 0x2b,
    0x4e, 0xd4, 0xe5, 0xac, 0x73, 0xf3, 0xa7, 0x57,
    0x07, 0x70, 0xc0, 0xf7, 0x8c, 0x80, 0x63, 0x0d,
    0x67, 0x4a, 0xde, 0xed, 0x31, 0xc5, 0xfe, 0x18,
    0xe3, 0xa5, 0x99, 0x77, 0x26, 0xb8, 0xb4, 0x7c,
    0x11, 0x44, 0x92, 0xd9, 0x23, 0x20, 0x89, 0x2e,
    0x37, 0x3f, 0xd1, 0x5b, 0x95, 0xbc, 0xcf, 0xcd,
    0x90, 0x87, 0x97, 0xb2, 0xdc, 0xfc, 0xbe, 0x61,
    0xf2, 0x56, 0xd3, 0xab, 0x14, 0x2a, 0x5d, 0x9e,
    0x84, 0x3c, 0x39, 0x53, 0x47, 0x6d, 0x41, 0xa2,
    0x1f, 0x2d, 0x43, 0xd8, 0xb7, 0x7b, 0xa4, 0x76,
    0xc4, 0x17, 0x49, 0xec, 0x7f, 0x0c, 0x6f, 0xf6,
    0x6c, 0xa1, 0x3b, 0x52, 0x29, 0x9d, 0x55, 0xaa,
    0xfb, 0x60, 0x86, 0xb1, 0xbb, 0xcc, 0x3e, 0x5a,
    0xcb, 0x59, 0x5f, 0xb0, 0x9c, 0xa9, 0xa0, 0x51,
    0x0b, 0xf5, 0x16, 0xeb, 0x7a, 0x75, 0x2c, 0xd7,
    0x4f, 0xae, 0xd5, 0xe9, 0xe6, 0xe7, 0xad, 0xe8,
    0x74, 0xd6, 0xf4, 0xea, 0xa8, 0x50, 0x58, 0xaf
};

// GF(2^4) field descriptor: order 15, used for format string error correction.
static const GF_FIELD g_Gf16  = { QR_GF16_ORDER,  g_abGf16Log,  g_abGf16Exp  };

// GF(2^8) field descriptor: order 255, used for codeword-level Reed-Solomon ECC.
static const GF_FIELD g_Gf256 = { QR_GF256_ORDER, g_abGf256Log, g_abGf256Exp };

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// QR ENCODING HELPERS
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region ENCODE_HELPERS

// Returns the value of bit nI from integer nX.
static inline BOOL  QriGetBit(IN INT nX, IN INT nI)
{
    return ((nX >> nI) & 1) != 0;
}

// Returns the module value at (nX, nY) from the packed QR grid buffer.
static inline BOOL  QriGetModuleBounded(IN const BYTE* pbQr, IN INT nX, IN INT nY)
{
    INT nQrSize = pbQr[0];
    INT nIndex  = nY * nQrSize + nX;
    return QriGetBit(pbQr[(nIndex >> 3) + 1], nIndex & 7);
}

// Sets or clears the module at (nX, nY) in the packed QR grid buffer.
static inline VOID  QriSetModuleBounded(IN OUT PBYTE pbQr, IN INT nX, IN INT nY, IN BOOL bDark)
{
    INT nQrSize     = pbQr[0];
    INT nIndex      = nY * nQrSize + nX;
    INT nBitIndex   = nIndex & 7;
    INT nByteIndex  = (nIndex >> 3) + 1;
    if (bDark)  pbQr[nByteIndex] |=   (BYTE)(1 << nBitIndex);
    else        pbQr[nByteIndex] &= ~((BYTE)(1 << nBitIndex));
}

// Sets or clears the module at (nX, nY), silently ignoring out-of-bounds coordinates.
static inline VOID  QriSetModuleUnbounded(IN OUT PBYTE pbQr, IN INT nX, IN INT nY, IN BOOL bDark)
{
    INT nQrSize = pbQr[0];
    if (nX >= 0 && nX < nQrSize && nY >= 0 && nY < nQrSize)
        QriSetModuleBounded(pbQr, nX, nY, bDark);
}

// Returns the number of raw data modules available for a given QR version.
static INT  QriGetNumRawDataModules(IN INT nVer)
{
    // total modules = (4v+17)^2, minus finders, separators, timing, format, and dark module
    INT nResult = (16 * nVer + 128) * nVer + 64;
    if (nVer >= QR_MIN_ALIGN_VERSION)
    {
        INT nNumAlign = nVer / 7 + 2;
        nResult -= (25 * nNumAlign - 10) * nNumAlign - 55;
        if (nVer >= QR_MIN_VERSION_INFO)
            nResult -= QR_VERSION_INFO_MODULES;
    }
    return nResult;
}

// Returns the number of usable data codewords after subtracting ECC overhead.
static INT  QriGetNumDataCodewords(IN INT nVer, IN QR_ECC eEcc)
{
    // Convert data modules(bits) to codewords(bytes)
    return QriGetNumRawDataModules(nVer) / 8
        - g_anEccCodewordsPerBlock[(INT)eEcc][nVer]
        * g_anNumEccBlocks[(INT)eEcc][nVer];
}

// Returns the character count indicator bit length for byte mode at a given version.
static inline INT  QriByteModeCcBits(IN INT nVer)
{
    return (nVer < 10) ? 8 : 16;
}

// Calculates the total bit length needed to encode cbData bytes in byte mode, or returns QR_LENGTH_OVERFLOW.
static INT  QriCalcByteBitLen(IN DWORD cbData, IN INT nVer)
{
    if (cbData > QR_MAX_BIT_LEN)
        return QR_LENGTH_OVERFLOW;
    INT nResult = QR_MODE_INDICATOR_BITS + QriByteModeCcBits(nVer) + (INT)cbData * 8;
    if (nResult > QR_MAX_BIT_LEN)
        return QR_LENGTH_OVERFLOW;
    return nResult;
}

// Appends nNumBits bits of nVal to pbBuf at the current bit offset, MSB first.
static VOID  QriAppendBits(IN UINT nVal, IN INT nNumBits, IN OUT PBYTE pbBuf, IN OUT PINT pnBitLen)
{
    for (INT nI = nNumBits - 1; nI >= 0; nI--, (*pnBitLen)++)
        pbBuf[*pnBitLen >> 3] |= (BYTE)(((nVal >> nI) & 1) << (7 - (*pnBitLen & 7)));
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Reed-Solomon Error Correction (Encode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region REED_SOLOMON

// Multiplies two bytes in GF(2^8) using the QR code polynomial.
static BYTE  QriRsMultiply(IN BYTE bX, IN BYTE bY)
{
    BYTE bZ = 0x00;
    for (INT nI = 7; nI >= 0; nI--)
    {
        bZ = (BYTE)((bZ << 1) ^ ((bZ >> 7) * QR_GF256_POLY));   
        bZ ^= ((bY >> nI) & 1) * bX;
    }
    return bZ;
}

// Computes the Reed-Solomon generator polynomial of the given degree.
static VOID  QriRsComputeDivisor(IN INT nDegree, OUT PBYTE pbResult)
{
    RtlZeroMemory(pbResult, (SIZE_T)nDegree);
    // Start with the monomial x^0
    pbResult[nDegree - 1] = 1;
    BYTE bRoot = 1;
    for (INT nI = 0; nI < nDegree; nI++)
    {
        // Multiply running polynomial by (x - alpha^nI)
        for (INT nJ = 0; nJ < nDegree; nJ++)
        {
            pbResult[nJ] = QriRsMultiply(pbResult[nJ], bRoot);
            if (nJ + 1 < nDegree)
                pbResult[nJ] ^= pbResult[nJ + 1];
        }

        // Next root: alpha^(nI+1)
        bRoot = QriRsMultiply(bRoot, QR_GF256_ALPHA);
    }
}

// Divides pbData by the generator polynomial and stores the remainder (ECC codewords) in pbResult.
static VOID  QriRsComputeRemainder(IN const BYTE* pbData, IN INT nDataLen, IN const BYTE* pbGenerator, IN INT nDegree, OUT PBYTE pbResult)
{
    RtlZeroMemory(pbResult, (SIZE_T)nDegree);
    
    for (INT nI = 0; nI < nDataLen; nI++)
    {
        // Leading term XOR with data byte
        BYTE bFactor = pbData[nI] ^ pbResult[0];
        // Shift remainder left by one
        RtlMoveMemory(&pbResult[0], &pbResult[1], (SIZE_T)(nDegree - 1));
        pbResult[nDegree - 1] = 0x00;
        for (INT nJ = 0; nJ < nDegree; nJ++)
            // Subtract generator * factor
            pbResult[nJ] ^= QriRsMultiply(pbGenerator[nJ], bFactor);
    }
}

// Splits data into blocks, appends Reed-Solomon ECC to each, and interleaves the result.
static VOID  QriAddEccAndInterleave(IN OUT PBYTE pbData, IN INT nVer, IN QR_ECC eEcc, OUT PBYTE pbResult)
{
    INT         nNumBlocks                      = g_anNumEccBlocks[(INT)eEcc][nVer];
    INT         nBlockEccLen                    = g_anEccCodewordsPerBlock[(INT)eEcc][nVer];
    INT         nRawCodewords                   = QriGetNumRawDataModules(nVer) / 8;
    INT         nDataLen                        = QriGetNumDataCodewords(nVer, eEcc);
    INT         nNumShortBlocks                 = nNumBlocks - nRawCodewords % nNumBlocks;
    INT         nShortBlockDataLen              = nRawCodewords / nNumBlocks - nBlockEccLen;
    BYTE        abRsDivisor[QR_RS_DEGREE_MAX]   = { 0 };
    CONST BYTE* pDat                            = pbData;

    QriRsComputeDivisor(nBlockEccLen, abRsDivisor);

    for (INT nI = 0; nI < nNumBlocks; nI++)
    {
        INT   nDatLen = nShortBlockDataLen + (nI < nNumShortBlocks ? 0 : 1);    // long blocks get +1 byte
        PBYTE pbEcc   = &pbData[nDataLen];                                      // temporary ECC storage past the data region

        QriRsComputeRemainder(pDat, nDatLen, abRsDivisor, nBlockEccLen, pbEcc);

        // interleave data codewords across blocks
        for (INT nJ = 0, nK = nI; nJ < nDatLen; nJ++, nK += nNumBlocks)
        {
            if (nJ == nShortBlockDataLen) nK -= nNumShortBlocks;                // skip short-block gap
            pbResult[nK] = pDat[nJ];
        }

        // interleave ECC codewords across blocks
        for (INT nJ = 0, nK = nDataLen + nI; nJ < nBlockEccLen; nJ++, nK += nNumBlocks)
        {
            pbResult[nK] = pbEcc[nJ];
        }

        pDat += nDatLen;
    }
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// QR Module Drawing & Masking (Encode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region ENCODE_DRAWING

// Returns the alignment pattern center positions for a given version.
static INT  QriGetAlignmentPatternPositions(IN INT nVer, OUT BYTE abResult[7])
{
    // No alignment patterns
    if (nVer == QR_VERSION_MIN) return 0;

    INT nNumAlign   = nVer / 7 + 2;
    INT nStep       = (nVer * 8 + nNumAlign * 3 + 5) / (nNumAlign * 4 - 4) * 2; // even spacing
    
    // fill positions from last to first, stepping backwards from bottom-right
    for (INT nI = nNumAlign - 1, nPos = nVer * 4 + 10; nI >= 1; nI--, nPos -= nStep)
        abResult[nI] = (BYTE)nPos;
    
    // first position is always at coordinate QR_TIMING_POS (6) 
    abResult[0] = QR_TIMING_POS;
    return nNumAlign;
}

// Fills a rectangle of dark modules in the QR grid.
static VOID  QriFillRect(IN OUT PBYTE pbQr, IN INT nLeft, IN INT nTop, IN INT nW, IN INT nH)
{
    for (INT nDy = 0; nDy < nH; nDy++)
        for (INT nDx = 0; nDx < nW; nDx++)
            QriSetModuleBounded(pbQr, nLeft + nDx, nTop + nDy, TRUE);
}

// Marks all function module regions (finders, timing, alignment, version) as occupied.
static VOID  QriInitFunctionModules(IN INT nVer, IN OUT PBYTE pbQr)
{
    INT     nQrSize         = nVer * QR_VER_INCREMENT + QR_BASE_SIZE;
    BYTE    abAlignPos[7]   = { 0 };
    INT     nNumAlign       = 0x00;

    RtlZeroMemory(pbQr, (SIZE_T)((nQrSize * nQrSize + 7) / 8 + 1));
    pbQr[0] = (BYTE)nQrSize; // first byte stores the grid dimension

    // Timing patterns
    QriFillRect(pbQr, QR_TIMING_POS, 0, 1, nQrSize);
    QriFillRect(pbQr, 0, QR_TIMING_POS, nQrSize, 1);

    // Finder patterns + format bit areas (three corners)
    QriFillRect(pbQr, 0, 0, QR_FINDER_TOTAL, QR_FINDER_TOTAL);
    QriFillRect(pbQr, nQrSize - (QR_FINDER_TOTAL - 1), 0, QR_FINDER_TOTAL - 1, QR_FINDER_TOTAL);
    QriFillRect(pbQr, 0, nQrSize - (QR_FINDER_TOTAL - 1), QR_FINDER_TOTAL, QR_FINDER_TOTAL - 1);

    // Alignment patterns (skip positions that overlap finder patterns)
    nNumAlign = QriGetAlignmentPatternPositions(nVer, abAlignPos);
    for (INT nI = 0; nI < nNumAlign; nI++)
    {
        for (INT nJ = 0; nJ < nNumAlign; nJ++)
        {
            if ((nI == 0 && nJ == 0) || (nI == 0 && nJ == nNumAlign - 1) || (nI == nNumAlign - 1 && nJ == 0))
                continue;
            QriFillRect(pbQr, abAlignPos[nI] - QR_ALIGN_MARGIN, abAlignPos[nJ] - QR_ALIGN_MARGIN, QR_ALIGN_SIZE, QR_ALIGN_SIZE);
        }
    }

    // Version blocks (v7+)
    if (nVer >= QR_MIN_VERSION_INFO)
    {
        QriFillRect(pbQr, nQrSize - QR_VERINFO_OFFSET, 0, QR_VERINFO_WIDTH, QR_TIMING_POS);
        QriFillRect(pbQr, 0, nQrSize - QR_VERINFO_OFFSET, QR_TIMING_POS, QR_VERINFO_WIDTH);
    }
}

// Draws the light modules within finder, alignment, timing, and version patterns.
static VOID  QriDrawLightFunctionModules(IN OUT PBYTE pbQr, IN INT nVer)
{
    INT     nQrSize         = QrGetSize(pbQr);
    BYTE    abAlignPos[7]   = { 0 };
    INT     nNumAlign       = 0x00;

    // Timing patterns: clear every other module to form alternating pattern
    for (INT nI = QR_FINDER_SIZE; nI < nQrSize - QR_FINDER_SIZE; nI += 2)
    {
        QriSetModuleBounded(pbQr, QR_TIMING_POS, nI, FALSE);
        QriSetModuleBounded(pbQr, nI, QR_TIMING_POS, FALSE);
    }

    // Finder pattern rings: clear modules at Chebyshev distance 2 and 4 from center
    for (INT nDy = -(QR_FINDER_SIZE / 2 + 1); nDy <= (QR_FINDER_SIZE / 2 + 1); nDy++)
    {
        for (INT nDx = -(QR_FINDER_SIZE / 2 + 1); nDx <= (QR_FINDER_SIZE / 2 + 1); nDx++)
        {
            INT nDist = abs(nDx);
            if (abs(nDy) > nDist) nDist = abs(nDy);
            if (nDist == QR_ALIGN_MARGIN || nDist == (QR_FINDER_SIZE / 2 + 1))
            {
                QriSetModuleUnbounded(pbQr, (QR_FINDER_SIZE / 2) + nDx, (QR_FINDER_SIZE / 2) + nDy, FALSE);
                QriSetModuleUnbounded(pbQr, nQrSize - (QR_FINDER_SIZE / 2 + 1) + nDx, (QR_FINDER_SIZE / 2) + nDy, FALSE);
                QriSetModuleUnbounded(pbQr, (QR_FINDER_SIZE / 2) + nDx, nQrSize - (QR_FINDER_SIZE / 2 + 1) + nDy, FALSE);
            }
        }
    }

    // Alignment pattern centers: 3x3 ring with only center module dark
    nNumAlign = QriGetAlignmentPatternPositions(nVer, abAlignPos);
    for (INT nI = 0; nI < nNumAlign; nI++)
    {
        for (INT nJ = 0; nJ < nNumAlign; nJ++)
        {
            if ((nI == 0 && nJ == 0) || (nI == 0 && nJ == nNumAlign - 1) || (nI == nNumAlign - 1 && nJ == 0))
                continue;
            for (INT nDy = -1; nDy <= 1; nDy++)
                for (INT nDx = -1; nDx <= 1; nDx++)
                    QriSetModuleBounded(pbQr, abAlignPos[nI] + nDx, abAlignPos[nJ] + nDy, (nDx == 0 && nDy == 0));
        }
    }

    // Version info bits (v7+): BCH-encoded version placed in two 6x3 blocks
    if (nVer >= QR_MIN_VERSION_INFO)
    {
        INT  nRem  = nVer;
        for (INT nI = 0; nI < QR_BCH_VERSION_BITS; nI++)
            nRem = (nRem << 1) ^ ((nRem >> (QR_BCH_VERSION_BITS - 1)) * QR_BCH_VERSION_GEN);
        LONG lBits = (LONG)nVer << QR_BCH_VERSION_BITS | nRem;
        for (INT nI = 0; nI < QR_TIMING_POS; nI++)
        {
            for (INT nJ = 0; nJ < QR_VERINFO_WIDTH; nJ++)
            {
                INT nK = nQrSize - QR_VERINFO_OFFSET + nJ;
                QriSetModuleBounded(pbQr, nK, nI, (lBits & 1) != 0);
                QriSetModuleBounded(pbQr, nI, nK, (lBits & 1) != 0);
                lBits >>= 1;
            }
        }
    }
}

// Encodes and writes the 15-bit format information into both copies on the grid.
static VOID  QriDrawFormatBits(IN QR_ECC eEcc, IN INT nMask, IN OUT PBYTE pbQr)
{
    static const INT anEccTable[] = { 1, 0, 3, 2 };
    INT nData   = anEccTable[(INT)eEcc] << 3 | nMask;
    INT nRem    = nData;
    
    for (INT nI = 0; nI < QR_BCH_FORMAT_BITS; nI++)
        nRem = (nRem << 1) ^ ((nRem >> (QR_BCH_FORMAT_BITS - 1)) * QR_BCH_FORMAT_GEN);
    
    INT nBits = (nData << QR_BCH_FORMAT_BITS | nRem) ^ QR_FORMAT_XOR_MASK;

    // First copy: adjacent to top-left finder
    for (INT nI = 0; nI <= 5; nI++)
        QriSetModuleBounded(pbQr, 8, nI, QriGetBit(nBits, nI));
    QriSetModuleBounded(pbQr, 8, 7, QriGetBit(nBits, 6));
    QriSetModuleBounded(pbQr, 8, 8, QriGetBit(nBits, 7));
    QriSetModuleBounded(pbQr, 7, 8, QriGetBit(nBits, 8));
    for (INT nI = 9; nI < FMT_BITS; nI++)
        QriSetModuleBounded(pbQr, (FMT_BITS - 1) - nI, 8, QriGetBit(nBits, nI));

    // Second copy: right + bottom edges
    INT nQrSize = QrGetSize(pbQr);
    for (INT nI = 0; nI < 8; nI++)
        QriSetModuleBounded(pbQr, nQrSize - 1 - nI, 8, QriGetBit(nBits, nI));
    for (INT nI = 8; nI < FMT_BITS; nI++)
        QriSetModuleBounded(pbQr, 8, nQrSize - FMT_BITS + nI, QriGetBit(nBits, nI));
    QriSetModuleBounded(pbQr, 8, nQrSize - 8, TRUE);    // Always dark
}

// Places interleaved data and ECC codewords into the available data modules.
static VOID  QriDrawCodewords(IN const BYTE* pbData, IN INT nDataLen, IN OUT PBYTE pbQr)
{
    INT nQrSize = QrGetSize(pbQr);
    INT nI      = 0;

    for (INT nRight = nQrSize - 1; nRight >= 1; nRight -= 2)
    {
        if (nRight == QR_TIMING_POS) nRight = QR_TIMING_POS - 1;
        for (INT nVert = 0; nVert < nQrSize; nVert++)
        {
            for (INT nJ = 0; nJ < 2; nJ++)
            {
                INT  nX     = nRight - nJ;
                BOOL bUp    = ((nRight + 1) & 2) == 0;
                INT  nY     = bUp ? nQrSize - 1 - nVert : nVert;
                if (!QriGetModuleBounded(pbQr, nX, nY) && nI < nDataLen * 8)
                {
                    BOOL bDark = QriGetBit(pbData[nI >> 3], 7 - (nI & 7));
                    QriSetModuleBounded(pbQr, nX, nY, bDark);
                    nI++;
                }
            }
        }
    }
}

// Applies one of the eight QR data mask patterns, toggling non-function modules.
static VOID  QriApplyMask(IN const BYTE* pbFuncModules, IN OUT PBYTE pbQr, IN INT nMask)
{
    INT nQrSize = QrGetSize(pbQr);
    for (INT nY = 0; nY < nQrSize; nY++)
    {
        for (INT nX = 0; nX < nQrSize; nX++)
        {
            if (QriGetModuleBounded(pbFuncModules, nX, nY))
                continue;
            
            BOOL bInvert = FALSE;
            switch (nMask)
            {
            case 0: bInvert = ((nX + nY) % 2 == 0);                       break;
            case 1: bInvert = (nY % 2 == 0);                              break;
            case 2: bInvert = (nX % 3 == 0);                              break;
            case 3: bInvert = ((nX + nY) % 3 == 0);                       break;
            case 4: bInvert = ((nX / 3 + nY / 2) % 2 == 0);               break;
            case 5: bInvert = (nX * nY % 2 + nX * nY % 3 == 0);           break;
            case 6: bInvert = ((nX * nY % 2 + nX * nY % 3) % 2 == 0);     break;
            case 7: bInvert = (((nX + nY) % 2 + nX * nY % 3) % 2 == 0);   break;
            }
            BOOL bVal = QriGetModuleBounded(pbQr, nX, nY);
            QriSetModuleBounded(pbQr, nX, nY, bVal ^ bInvert);
        }
    }
}

// Pushes a run length onto the penalty history ring buffer, padding with nQrSize if first entry.
static VOID  QriPenaltyAddHistory(IN INT nRunLen, IN OUT INT anHistory[QR_PENALTY_HISTORY], IN INT nQrSize)
{
    if (anHistory[0] == 0) nRunLen += nQrSize;
    RtlMoveMemory(&anHistory[1], &anHistory[0], (QR_PENALTY_HISTORY - 1) * sizeof(anHistory[0]));
    anHistory[0] = nRunLen;
}

// Checks the penalty history for finder-like 1:1:3:1:1 patterns with sufficient quiet zone.
static INT  QriPenaltyCountPatterns(IN const INT anHistory[QR_PENALTY_HISTORY], IN INT nQrSize)
{
    INT  nN     = anHistory[1];
    // check for 1:1:3:1:1 finder-like pattern in the run history
    BOOL bCore  = (nN > 0 && anHistory[2] == nN && anHistory[3] == nN * 3 && anHistory[4] == nN && anHistory[5] == nN);
    // require 4-unit quiet zone on at least one side of the pattern
    return (bCore && anHistory[0] >= nN * 4 && anHistory[6] >= nN ? 1 : 0) + (bCore && anHistory[6] >= nN * 4 && anHistory[0] >= nN ? 1 : 0);
}

// Flushes the final run at end-of-line into the penalty history and returns the pattern count.
static INT  QriPenaltyTerminateAndCount(IN BOOL bDarkRun, IN INT nRunLen, IN OUT INT anHistory[QR_PENALTY_HISTORY], IN INT nQrSize)
{
    if (bDarkRun)
    {
        QriPenaltyAddHistory(nRunLen, anHistory, nQrSize);
        nRunLen = 0;
    }
    nRunLen += nQrSize;
    QriPenaltyAddHistory(nRunLen, anHistory, nQrSize);
    return QriPenaltyCountPatterns(anHistory, nQrSize);
}

// Evaluates the total mask penalty score across all four QR penalty rules.
static LONG  QriGetPenaltyScore(IN const BYTE* pbQr)
{
    INT  nQrSize    = QrGetSize(pbQr);
    LONG lResult    = 0;

    // Rule 1: penalty for runs of 5+ same-colored modules — rows
    for (INT nY = 0; nY < nQrSize; nY++)
    {
        BOOL bRunColor  = FALSE;
        INT  nRunX      = 0;
        INT  anHist[QR_PENALTY_HISTORY]  = { 0 };
        for (INT nX = 0; nX < nQrSize; nX++)
        {
            if (QriGetModuleBounded(pbQr, nX, nY) == bRunColor)
            {
                nRunX++;
                if (nRunX == QR_PENALTY_RUN_MIN)      lResult += QR_PENALTY_N1;
                else if (nRunX > QR_PENALTY_RUN_MIN)   lResult++;
            }
            else
            {
                QriPenaltyAddHistory(nRunX, anHist, nQrSize);
                if (!bRunColor) lResult += QriPenaltyCountPatterns(anHist, nQrSize) * QR_PENALTY_N3;
                bRunColor   = QriGetModuleBounded(pbQr, nX, nY);
                nRunX       = 1;
            }
        }
        lResult += QriPenaltyTerminateAndCount(bRunColor, nRunX, anHist, nQrSize) * QR_PENALTY_N3;
    }

    // Rule 1: same check but for columns
    for (INT nX = 0; nX < nQrSize; nX++)
    {
        BOOL bRunColor  = FALSE;
        INT  nRunY      = 0;
        INT  anHist[QR_PENALTY_HISTORY]  = { 0 };
        for (INT nY = 0; nY < nQrSize; nY++)
        {
            if (QriGetModuleBounded(pbQr, nX, nY) == bRunColor)
            {
                nRunY++;
                if (nRunY == QR_PENALTY_RUN_MIN)      lResult += QR_PENALTY_N1;
                else if (nRunY > QR_PENALTY_RUN_MIN)   lResult++;
            }
            else
            {
                QriPenaltyAddHistory(nRunY, anHist, nQrSize);
                if (!bRunColor) lResult += QriPenaltyCountPatterns(anHist, nQrSize) * QR_PENALTY_N3;
                bRunColor   = QriGetModuleBounded(pbQr, nX, nY);
                nRunY       = 1;
            }
        }
        lResult += QriPenaltyTerminateAndCount(bRunColor, nRunY, anHist, nQrSize) * QR_PENALTY_N3;
    }

    // Rule 2: 2x2 same-color blocks
    for (INT nY = 0; nY < nQrSize - 1; nY++)
    {
        for (INT nX = 0; nX < nQrSize - 1; nX++)
        {
            BOOL bColor = QriGetModuleBounded(pbQr, nX, nY);
            if (bColor == QriGetModuleBounded(pbQr, nX + 1, nY) &&
                bColor == QriGetModuleBounded(pbQr, nX,     nY + 1) &&
                bColor == QriGetModuleBounded(pbQr, nX + 1, nY + 1))
                lResult += QR_PENALTY_N2;
        }
    }

    // Rule 4: penalty if dark/light ratio deviates from 50%
    INT nDark   = 0;
    INT nTotal  = nQrSize * nQrSize;
    for (INT nY = 0; nY < nQrSize; nY++)
        for (INT nX = 0; nX < nQrSize; nX++)
            if (QriGetModuleBounded(pbQr, nX, nY)) nDark++;
    INT nK = (INT)((labs(nDark * 20L - nTotal * 10L) + nTotal - 1) / nTotal) - 1;
    if (nK > 0) lResult += nK * QR_PENALTY_N4;

    return lResult;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Galois Field Error Correction (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_GF

// Adds polynomial pbSrc scaled by bC and shifted by nShift into pbDst over the given Galois field.
static VOID  GfPolyAdd(IN OUT PBYTE pbDst, IN PBYTE pbSrc, IN BYTE bC, IN INT nShift, IN const GF_FIELD* pGf)
{
    INT nLogC = pGf->pbLog[bC];
    if (!bC) return;
    
    for (INT nI = 0; nI < GF_POLY_MAX; nI++)
    {
        INT     nP  = nI + nShift;
        BYTE    bV  = pbSrc[nI];
        if (nP < 0 || nP >= GF_POLY_MAX)    continue;
        if (!bV)                             continue;
        pbDst[nP] ^= pGf->pbExp[(pGf->pbLog[bV] + nLogC) % pGf->nP];
    }
}

// Evaluates polynomial pbS at point bX over the given Galois field.
static BYTE  GfPolyEval(IN PBYTE pbS, IN BYTE bX, IN const GF_FIELD* pGf)
{
    if (!bX) return pbS[0];

    BYTE bSum   = 0x00;
    BYTE bLogX  = pGf->pbLog[bX];
    for (INT nI = 0; nI < GF_POLY_MAX; nI++)
    {
        BYTE bC = pbS[nI];
        if (!bC) continue;
        bSum ^= pGf->pbExp[(pGf->pbLog[bC] + bLogX * nI) % pGf->nP];
    }
    return bSum;
}

// Computes the error locator polynomial via Berlekamp-Massey for nN syndromes.
static VOID  GfBerlekampMassey(IN PBYTE pbS, IN INT nN, IN const GF_FIELD* pGf, OUT PBYTE pbSigma)
{
    BYTE    abC[GF_POLY_MAX]    = { 0 };    // current error locator polynomial
    BYTE    abB[GF_POLY_MAX]    = { 0 };    // previous locator for update step
    INT     nL                  = 0;        // current number of assumed errors
    INT     nM                  = 1;        // shift counter
    BYTE    bB                  = 1;        // previous discrepancy

    abB[0] = 1;
    abC[0] = 1;

    for (INT nN2 = 0; nN2 < nN; nN2++)
    {
        BYTE bD     = pbS[nN2]; // compute discrepancy
        BYTE bMult  = 0x00;

        for (INT nI = 1; nI <= nL; nI++)
        {
            if (!(abC[nI] && pbS[nN2 - nI])) continue;
            bD ^= pGf->pbExp[(pGf->pbLog[abC[nI]] + pGf->pbLog[pbS[nN2 - nI]]) % pGf->nP];
        }
        
        bMult = pGf->pbExp[(pGf->nP - pGf->pbLog[bB] + pGf->pbLog[bD]) % pGf->nP];
        
        // discrepancy is zero, just shift
        if (!bD)
        {
            nM++; 
        }
        // need to increase error count: swap polynomials
        else if (nL * 2 <= nN2)
        {
            BYTE abT[GF_POLY_MAX] = { 0 };
            RtlCopyMemory(abT, abC, sizeof(abT));
            GfPolyAdd(abC, abB, bMult, nM, pGf);
            RtlCopyMemory(abB, abT, sizeof(abB));
            nL  = nN2 + 1 - nL;
            bB  = bD;
            nM  = 1;
        }
        // adjust locator, no length change
        else
        {
            GfPolyAdd(abC, abB, bMult, nM, pGf);
            nM++;
        }
    }
    RtlCopyMemory(pbSigma, abC, GF_POLY_MAX);
}

// Computes syndromes for a Reed-Solomon block; returns TRUE if any are non-zero.
static BOOL  GfBlockSyndromes(IN PBYTE pbData, IN INT nBs, IN INT nPar, OUT PBYTE pbS)
{
    BOOL bNonZero = FALSE;
    RtlZeroMemory(pbS, GF_POLY_MAX);
    for (INT nI = 0; nI < nPar; nI++)
    {
        for (INT nJ = 0; nJ < nBs; nJ++)
        {
            BYTE bC = pbData[nBs - nJ - 1];
            if (!bC) continue;
            pbS[nI] ^= g_abGf256Exp[((INT)g_abGf256Log[bC] + nI * nJ) % QR_GF256_ORDER];
        }
        if (pbS[nI])
            bNonZero = TRUE;
    }
    return bNonZero;
}

// Computes the error evaluator polynomial from the syndromes and error locator.
static VOID  GfElocPoly(OUT PBYTE pbOmega, IN PBYTE pbS, IN PBYTE pbSigma, IN INT nPar)
{
    RtlZeroMemory(pbOmega, GF_POLY_MAX);
    for (INT nI = 0; nI < nPar; nI++)
    {
        const BYTE  bA      = pbSigma[nI];
        const BYTE  bLogA   = g_abGf256Log[bA];
        if (!bA) continue;
        for (INT nJ = 0; nJ + 1 < GF_POLY_MAX; nJ++)
        {
            const BYTE bB = pbS[nJ + 1];
            if (nI + nJ >= nPar)    break;
            if (!bB)                continue;
            pbOmega[nI + nJ] ^= g_abGf256Exp[(bLogA + g_abGf256Log[bB]) % QR_GF256_ORDER];
        }
    }
}

// Corrects a single Reed-Solomon block in place using Berlekamp-Massey and Forney's algorithm.
static BOOL  GfCorrectBlock(IN OUT PBYTE pbData, IN const QR_RS_PARAMS* pEcc)
{
    INT     nPar                        = pEcc->nBs - pEcc->nDw;        // number of parity symbols
    BYTE    abS[GF_POLY_MAX]            = { 0x00 };
    BYTE    abSigma[GF_POLY_MAX]        = { 0x00 };
    BYTE    abSigmaDeriv[GF_POLY_MAX]   = { 0x00 };
    BYTE    abOmega[GF_POLY_MAX]        = { 0x00 };

    // all syndromes zero means no errors
    if (!GfBlockSyndromes(pbData, pEcc->nBs, nPar, abS))
        return TRUE;

    // find error locator polynomial
    GfBerlekampMassey(abS, nPar, &g_Gf256, abSigma);

    // formal derivative of sigma (odd-indexed coefficients shifted down)
    for (INT nI = 0; nI + 1 < GF_POLY_MAX; nI += 2)
        abSigmaDeriv[nI] = abSigma[nI + 1];

    // error evaluator polynomial
    GfElocPoly(abOmega, abS, abSigma, nPar - 1);

    // Forney's algorithm: find error positions and magnitudes
    for (INT nI = 0; nI < pEcc->nBs; nI++)
    {
        BYTE bXInv = g_abGf256Exp[QR_GF256_ORDER - nI];

        // sigma(X^-1)==0 means error at position nI
        if (!GfPolyEval(abSigma, bXInv, &g_Gf256))
        {
            BYTE bSdX       = GfPolyEval(abSigmaDeriv, bXInv, &g_Gf256);
            BYTE bOmegaX    = GfPolyEval(abOmega,      bXInv, &g_Gf256);
            BYTE bError     = g_abGf256Exp[(QR_GF256_ORDER - g_abGf256Log[bSdX] + g_abGf256Log[bOmegaX]) % QR_GF256_ORDER];
            // correct the byte in-place:
            pbData[pEcc->nBs - nI - 1] ^= bError;
        }
    }

    // verify correction succeeded
    if (GfBlockSyndromes(pbData, pEcc->nBs, nPar, abS))
    {
        DBG("[!] Uncorrectable Errors Remain After Reed-Solomon Recovery (BlockSize: %d, DataWords: %d, Parity: %d)", pEcc->nBs, pEcc->nDw, nPar);
        return FALSE;
    }
    return TRUE;
}

// Computes syndromes for the 15-bit format word over GF(2^4); returns TRUE if any are non-zero.
static BOOL  GfFormatSyndromes(IN WORD wU, OUT PBYTE pbS)
{
    BOOL bNonZero = FALSE;
    RtlZeroMemory(pbS, GF_POLY_MAX);
    for (INT nI = 0; nI < FMT_SYNDROMES; nI++)
    {
        pbS[nI] = 0;
        for (INT nJ = 0; nJ < FMT_BITS; nJ++)
            if (wU & (1 << nJ))
                pbS[nI] ^= g_abGf16Exp[((nI + 1) * nJ) % QR_GF16_ORDER];
        if (pbS[nI])
            bNonZero = TRUE;
    }
    return bNonZero;
}

// Corrects the 15-bit format word in place using GF(2^4) Berlekamp-Massey.
static BOOL  GfCorrectFormat(IN OUT PWORD pwFmt)
{
    WORD    wU                      = *pwFmt;
    BYTE    abS[GF_POLY_MAX]        = { 0 };
    BYTE    abSigma[GF_POLY_MAX]    = { 0 };

    // no errors detected
    if (!GfFormatSyndromes(wU, abS))
        return TRUE;

    GfBerlekampMassey(abS, FMT_SYNDROMES, &g_Gf16, abSigma);

    // Flip each bit position where sigma evaluates to zero (error location)
    for (INT nI = 0; nI < QR_GF16_ORDER; nI++)
        if (!GfPolyEval(abSigma, g_abGf16Exp[QR_GF16_ORDER - nI], &g_Gf16))
            wU ^= (1 << nI);

    // verify correction
    if (GfFormatSyndromes(wU, abS))
    {
        DBG("[!] Format Info Unrecoverable After ECC Correction (Raw: 0x%04X, Corrected: 0x%04X)", *pwFmt, wU);
        return FALSE;
    }
    *pwFmt = wU;
    return TRUE;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// QR DATA EXTRACTION & ECC (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_DATASTREAM

// Reads a single bit from the decoded QR grid at (nX, nY), LSB-first packed.
static inline INT QrGridBit(IN const QR_CODE* pCode, IN INT nX, IN INT nY)
{
    INT nP = nY * pCode->nSize + nX;
    return (pCode->abCells[nP >> 3] >> (nP & 7)) & 1;
}

// Reads and error-corrects the 15-bit format information from copy nWhich (0 = top-left, 1 = edges).
static BOOL  QrReadFormat(IN const QR_CODE* pCode, IN OUT PQR_DATA pData, IN INT nWhich)
{
    WORD wFmt   = 0x00;
    WORD wFData = 0x00;

    if (nWhich)
    {
        // read from bottom-left and top-right edge copies
        for (INT nI = 0; nI < 7; nI++)
            wFmt = (WORD)((wFmt << 1) | QrGridBit(pCode, 8, pCode->nSize - 1 - nI));
        for (INT nI = 0; nI < 8; nI++)
            wFmt = (WORD)((wFmt << 1) | QrGridBit(pCode, pCode->nSize - 8 + nI, 8));
    }
    else
    {
        // read from top-left copy using fixed coordinate tables
        static const INT anXs[FMT_BITS] = { 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 4, 3, 2, 1, 0 };
        static const INT anYs[FMT_BITS] = { 0, 1, 2, 3, 4, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8 };
        for (INT nI = FMT_BITS - 1; nI >= 0; nI--)
            wFmt = (WORD)((wFmt << 1) | QrGridBit(pCode, anXs[nI], anYs[nI]));
    }

    wFmt ^= QR_FORMAT_XOR_MASK; // unmask the format info

    if (!GfCorrectFormat(&wFmt))
        return FALSE;

    wFData              = wFmt >> QR_BCH_FORMAT_BITS; // upper 5 bits hold ECC level + mask
    pData->nEccLevel    = wFData >> 3;
    pData->nMask        = wFData & 7;
    return TRUE;
}

// Returns whether the data mask flips the module at row nI, column nJ for the given mask pattern.
static INT  QrMaskBit(IN INT nMask, IN INT nI, IN INT nJ)
{
    switch (nMask)
    {
    case 0: return !((nI + nJ) % 2);
    case 1: return !(nI % 2);
    case 2: return !(nJ % 3);
    case 3: return !((nI + nJ) % 3);
    case 4: return !(((nI / 2) + (nJ / 3)) % 2);
    case 5: return !((nI * nJ) % 2 + (nI * nJ) % 3);
    case 6: return !(((nI * nJ) % 2 + (nI * nJ) % 3) % 2);
    case 7: return !(((nI * nJ) % 3 + (nI + nJ) % 2) % 2);
    }
    return 0;
}

// Returns whether the cell at (nI, nJ) is reserved by a function pattern (finders, timing, alignment, version).
static INT  QrReservedCell(IN INT nVersion, IN INT nI, IN INT nJ)
{
    const QR_VER_INFO   *pVer   = &g_aQrVersionDb[nVersion];
    INT                 nSize   = nVersion * QR_VER_INCREMENT + QR_BASE_SIZE;
    INT                 nAi     = -1; // alignment pattern row index, -1 if none
    INT                 nAj     = -1; // alignment pattern column index
    INT                 a       = 0x00;

    // finder pattern + format info regions (three corners)
    if (nI < QR_FINDER_TOTAL && nJ < QR_FINDER_TOTAL)                          return 1;
    if (nI + (QR_FINDER_TOTAL - 1) >= nSize && nJ < QR_FINDER_TOTAL)           return 1;
    if (nI < QR_FINDER_TOTAL && nJ + (QR_FINDER_TOTAL - 1) >= nSize)           return 1;
    if (nI == QR_TIMING_POS || nJ == QR_TIMING_POS)                            return 1; // timing patterns

    // version info blocks (v7+)
    if (nVersion >= QR_MIN_VERSION_INFO)
    {
        if (nI < QR_TIMING_POS && nJ + QR_VERINFO_OFFSET >= nSize)    return 1;
        if (nI + QR_VERINFO_OFFSET >= nSize && nJ < QR_TIMING_POS)    return 1;
    }

    // check proximity to alignment pattern positions
    for (a = 0; a < QR_MAX_ALIGNMENT && pVer->anAPat[a]; a++)
    {
        INT nP = pVer->anAPat[a];
        if (abs(nP - nI) < (QR_ALIGN_MARGIN + 1)) nAi = a;
        if (abs(nP - nJ) < (QR_ALIGN_MARGIN + 1)) nAj = a;
    }

    if (nAi >= 0 && nAj >= 0)
    {
        a--; // a is now the last valid alignment index
        if (nAi > 0 && nAi < a)    return 1;
        if (nAj > 0 && nAj < a)    return 1;
        if (nAj == a && nAi == a)   return 1;
    }

    return 0;
}

// Reads a single data bit from the grid, applies the mask, and appends it to the datastream.
static VOID  QrReadBit(IN const QR_CODE* pCode, IN PQR_DATA pData, IN OUT PQR_DATASTREAM pDs, IN INT nI, IN INT nJ)
{
    INT nBitPos     = pDs->nDataBits & 7;
    INT nBytePos    = pDs->nDataBits >> 3;
    INT nV          = QrGridBit(pCode, nJ, nI);
    if (QrMaskBit(pData->nMask, nI, nJ))
        nV ^= 1;
    if (nV)
        pDs->pbRaw[nBytePos] |= (0x80 >> nBitPos);
    pDs->nDataBits++;
}

// Reads all data bits from the grid in the QR zigzag traversal order, skipping reserved cells.
static VOID  QrReadData(IN const QR_CODE* pCode, IN PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    INT nY      = pCode->nSize - 1; // start at bottom-right
    INT nX      = pCode->nSize - 1;
    INT nDir    = -1;              // initial direction is upward
    while (nX > 0)
    {
        if (nX == QR_TIMING_POS) nX--; // skip vertical timing column
        if (!QrReservedCell(pData->nVersion, nY, nX))
            QrReadBit(pCode, pData, pDs, nY, nX);
        if (!QrReservedCell(pData->nVersion, nY, nX - 1))
            QrReadBit(pCode, pData, pDs, nY, nX - 1); // read the left column of the pair
        nY += nDir;
        if (nY < 0 || nY >= pCode->nSize) // hit top or bottom edge, reverse direction
        {
            nDir    = -nDir;
            nX      -= 2; // move to the next two-column strip
            nY      += nDir;
        }
    }
}

// De-interleaves the raw datastream into RS blocks, corrects each block, and concatenates the data codewords.
static BOOL  QrCodestreamEcc(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    const QR_VER_INFO   *pVer       = &g_aQrVersionDb[pData->nVersion];
    const QR_RS_PARAMS  *pSbEcc     = &pVer->aEcc[pData->nEccLevel]; // short block ECC params
    QR_RS_PARAMS        LbEcc       = { 0x00 };                      // long block ECC params
    const INT           nLbCount    = (pVer->nDataBytes - pSbEcc->nBs * pSbEcc->nNs) / (pSbEcc->nBs + 1);
    const INT           nBc         = nLbCount + pSbEcc->nNs; // total block count
    const INT           nEccOffset  = pSbEcc->nDw * nBc + nLbCount; // where ECC codewords start
    INT                 nDstOffset  = 0x00;

    RtlCopyMemory(&LbEcc, pSbEcc, sizeof(LbEcc));
    LbEcc.nDw++; // long blocks have one extra data codeword
    LbEcc.nBs++;

    for (INT nI = 0; nI < nBc; nI++)
    {
        PBYTE               pbDst   = pDs->abData + nDstOffset;
        const QR_RS_PARAMS  *pEcc   = (nI < pSbEcc->nNs) ? pSbEcc : &LbEcc; // pick short or long block params
        const INT           nNumEc  = pEcc->nBs - pEcc->nDw;

        // de-interleave data codewords for this block
        for (INT nJ = 0; nJ < pEcc->nDw; nJ++)
            pbDst[nJ] = pDs->pbRaw[nJ * nBc + nI];
        // de-interleave ECC codewords for this block
        for (INT nJ = 0; nJ < nNumEc; nJ++)
            pbDst[pEcc->nDw + nJ] = pDs->pbRaw[nEccOffset + nJ * nBc + nI];

        if (!GfCorrectBlock(pbDst, pEcc))
        {
            DBG("[!] Reed-Solomon Correction Failed On Block %d/%d (BlockSize: %d, DataWords: %d, ECC: %d)", nI, nBc, pEcc->nBs, pEcc->nDw, nNumEc);
            return FALSE;
        }
        nDstOffset += pEcc->nDw;
    }

    pDs->nDataBits = nDstOffset * 8;
    return TRUE;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// QR Payload Decoding
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_PAYLOAD

// Returns the number of unread bits remaining in the datastream.
static inline INT QrBitsRemaining(IN const QR_DATASTREAM* pDs)
{
    return pDs->nDataBits - pDs->nPtr;
}

// Consumes and returns nLen bits from the datastream, MSB first.
static INT  QrTakeBits(IN OUT PQR_DATASTREAM pDs, IN INT nLen)
{
    INT nRet = 0x00;

    while (nLen && (pDs->nPtr < pDs->nDataBits))
    {
        BYTE    bB      = pDs->abData[pDs->nPtr >> 3];
        INT     nBitPos = pDs->nPtr & 7;
        nRet <<= 1;
        if ((bB << nBitPos) & 0x80)
            nRet |= 1;
        pDs->nPtr++;
        nLen--;
    }
    return nRet;
}

// Decodes a numeric mode segment (digits packed as 10/7/4-bit groups).
static BOOL  QrDecodeNumeric(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    INT nBits   = 14; // character count indicator length depends on version
    INT nCount  = 0x00;
    
    if (pData->nVersion < QR_VER_MEDIUM_THRESHOLD) 
        nBits = 10;
    else if (pData->nVersion < QR_VER_LARGE_THRESHOLD)  
        nBits = 12;
    
    nCount = QrTakeBits(pDs, nBits); // read the digit count
    if (pData->cbPayload + nCount + 1 > QR_MAX_PAYLOAD)
        return FALSE;
    
    while (nCount >= 3) // decode 3 digits packed into 10 bits
    {
        if (QrBitsRemaining(pDs) < 10) return FALSE;
        INT nTuple = QrTakeBits(pDs, 10);
        for (INT nI = 2; nI >= 0; nI--)
        {
            pData->abPayload[pData->cbPayload + nI] = (BYTE)(nTuple % 10 + '0');
            nTuple /= 10;
        }
        pData->cbPayload += 3;
        nCount           -= 3;
    }
    if (nCount >= 2) // remaining 2 digits packed into 7 bits
    {
        if (QrBitsRemaining(pDs) < 7) return FALSE;
        INT nTuple = QrTakeBits(pDs, 7);
        for (INT nI = 1; nI >= 0; nI--)
        {
            pData->abPayload[pData->cbPayload + nI] = (BYTE)(nTuple % 10 + '0');
            nTuple /= 10;
        }
        pData->cbPayload += 2;
        nCount           -= 2;
    }
    if (nCount) // remaining single digit in 4 bits
    {
        if (QrBitsRemaining(pDs) < 4) return FALSE;
        pData->abPayload[pData->cbPayload++] = (BYTE)(QrTakeBits(pDs, 4) + '0');
    }
    return TRUE;
}

// Decodes an alphanumeric mode segment (character pairs packed as 11-bit values).
static BOOL  QrDecodeAlpha(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    INT nBits   = 13; // version-dependent character count indicator size
    INT nCount  = 0x00;

static const CHAR szAlphaMap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

    if (pData->nVersion < QR_VER_MEDIUM_THRESHOLD)
        nBits = 9;
    else if (pData->nVersion < QR_VER_LARGE_THRESHOLD)
        nBits = 11;
    
    nCount = QrTakeBits(pDs, nBits);
    if (pData->cbPayload + nCount + 1 > QR_MAX_PAYLOAD)
        return FALSE;
    
    while (nCount >= 2) // two characters packed into 11 bits (45*c1 + c2)
    {
        if (QrBitsRemaining(pDs) < 11) return FALSE;
        INT nTuple = QrTakeBits(pDs, 11);
        pData->abPayload[pData->cbPayload + 1]  = (BYTE)szAlphaMap[nTuple % QR_ALPHA_CHARSET_SIZE];
        pData->abPayload[pData->cbPayload]      = (BYTE)szAlphaMap[nTuple / QR_ALPHA_CHARSET_SIZE];
        pData->cbPayload += 2;
        nCount           -= 2;
    }
    if (nCount) // odd trailing character in 6 bits
    {
        if (QrBitsRemaining(pDs) < 6) return FALSE;
        pData->abPayload[pData->cbPayload++] = (BYTE)szAlphaMap[QrTakeBits(pDs, 6)];
    }
    return TRUE;
}

// Decodes a byte mode segment (raw 8-bit values).
static BOOL  QrDecodeByte(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    INT nBits   = 16;
    INT nCount  = 0x00;
    
    if (pData->nVersion < QR_VER_MEDIUM_THRESHOLD) nBits = 8;
    
    nCount = QrTakeBits(pDs, nBits);
    if (pData->cbPayload + nCount + 1 > QR_MAX_PAYLOAD)
        return FALSE;
    
    if (QrBitsRemaining(pDs) < nCount * 8)
        return FALSE;
    
    for (INT nI = 0; nI < nCount; nI++)
        pData->abPayload[pData->cbPayload++] = (BYTE)QrTakeBits(pDs, 8);
    
    return TRUE;
}

// Decodes a Kanji mode segment (Shift-JIS double-byte characters packed as 13-bit values).
static BOOL  QrDecodeKanji(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    INT nBits   = 12;
    INT nCount  = 0x00;
    
    if (pData->nVersion < QR_VER_MEDIUM_THRESHOLD)
        nBits = 8;
    else if (pData->nVersion < QR_VER_LARGE_THRESHOLD) 
        nBits = 10;
    
    nCount = QrTakeBits(pDs, nBits);
    if (pData->cbPayload + nCount * 2 + 1 > QR_MAX_PAYLOAD)
        return FALSE;
    
    if (QrBitsRemaining(pDs) < nCount * 13)
        return FALSE;
    
    for (INT nI = 0; nI < nCount; nI++)
    {
        INT     nD      = QrTakeBits(pDs, 13);
        INT     nMsB    = nD / QR_KANJI_DIVISOR;   // high byte of the Shift-JIS pair
        INT     nLsB    = nD % QR_KANJI_DIVISOR;   // low byte
        INT     nInter  = (nMsB << 8) | nLsB;
        WORD    wSjw    = (WORD)((nInter + QR_SJIS_LOW_BASE <= QR_SJIS_LOW_LIMIT) ? nInter + QR_SJIS_LOW_BASE : nInter + QR_SJIS_HIGH_BASE); // restore Shift-JIS range
        pData->abPayload[pData->cbPayload++] = (BYTE)(wSjw >> 8);
        pData->abPayload[pData->cbPayload++] = (BYTE)(wSjw & 0xff);
    }
    return TRUE;
}

// Decodes an ECI mode indicator (1, 2, or 3 byte assignment number).
static BOOL  QrDecodeEci(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    if (QrBitsRemaining(pDs) < 8)
        return FALSE;
    
    pData->dwEci = (DWORD)QrTakeBits(pDs, 8);
    
    if ((pData->dwEci & QR_ECI_2BYTE_MASK) == QR_ECI_2BYTE_TAG)
    {
        if (QrBitsRemaining(pDs) < 8) return FALSE;
        pData->dwEci = (pData->dwEci << 8) | (DWORD)QrTakeBits(pDs, 8);
    }
    else if ((pData->dwEci & QR_ECI_3BYTE_MASK) == QR_ECI_3BYTE_TAG)
    {
        if (QrBitsRemaining(pDs) < 16) return FALSE;
        pData->dwEci = (pData->dwEci << 16) | (DWORD)QrTakeBits(pDs, 16);
    }
    
    return TRUE;
}

// Dispatches mode segments in a loop until terminator or exhaustion, building the final payload.
static BOOL  QrDecodePayload(IN OUT PQR_DATA pData, IN OUT PQR_DATASTREAM pDs)
{
    while (QrBitsRemaining(pDs) >= QR_MODE_INDICATOR_BITS) // need at least 4 bits for a mode indicator
    {
        BOOL    bOk     = TRUE;
        INT     nType   = QrTakeBits(pDs, QR_MODE_INDICATOR_BITS); // read 4-bit mode indicator
        
        switch (nType)
        {
            case QR_DATA_TYPE_NUMERIC:  bOk = QrDecodeNumeric(pData, pDs);     break;
            case QR_DATA_TYPE_ALPHA:    bOk = QrDecodeAlpha(pData, pDs);       break;
            case QR_DATA_TYPE_BYTE:     bOk = QrDecodeByte(pData, pDs);        break;
            case QR_DATA_TYPE_KANJI:    bOk = QrDecodeKanji(pData, pDs);       break;
            case QR_DATA_TYPE_ECI:      bOk = QrDecodeEci(pData, pDs);         break;
            default:                    goto _END_OF_FUNC; // terminator or unknown mode
        }
        if (!bOk)
        {
            DBG("[!] Segment Decode Failed (Mode: %d, PayloadSoFar: %d Bytes, BitsRemaining: %d)", nType, pData->cbPayload, QrBitsRemaining(pDs));
            return FALSE;
        }
        
        // track the highest data type seen (power-of-two check filters ECI)
        if (!(nType & (nType - 1)) && (nType > pData->nDataType))
            pData->nDataType = nType;
    }

_END_OF_FUNC:
    if (pData->cbPayload >= (INT)sizeof(pData->abPayload))
        pData->cbPayload--;
    pData->abPayload[pData->cbPayload] = 0x00; // null-terminate
    return TRUE;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Geometric Transforms (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_GEOMETRY

// Computes the intersection point of two lines (P0-P1 and Q0-Q1); returns FALSE if parallel.
static BOOL  QrLineIntersect(IN const QR_POINT* pP0, IN const QR_POINT* pP1, IN const QR_POINT* pQ0, IN const QR_POINT* pQ1, OUT PQR_POINT pR)
{
    INT nA      = -(pP1->nY - pP0->nY);
    INT nB      =   pP1->nX - pP0->nX;
    INT nC      = -(pQ1->nY - pQ0->nY);
    INT nD      =   pQ1->nX - pQ0->nX;
    INT nE      = nA * pP1->nX + nB * pP1->nY;
    INT nF      = nC * pQ1->nX + nD * pQ1->nY;
    INT nDet    = (nA * nD) - (nB * nC);
    if (!nDet)
        return FALSE;
    pR->nX = (nD * nE - nB * nF) / nDet;
    pR->nY = (-nC * nE + nA * nF) / nDet;
    return TRUE;
}

// Builds the 8-coefficient perspective transform from a quadrilateral to a W x H rectangle.
static VOID  QrPerspectiveSetup(OUT QR_FLOAT* adC, IN const QR_POINT* aptRect, IN QR_FLOAT dW, IN QR_FLOAT dH)
{
    QR_FLOAT dX0    = aptRect[0].nX,    dY0 = aptRect[0].nY;
    QR_FLOAT dX1    = aptRect[1].nX,    dY1 = aptRect[1].nY;
    QR_FLOAT dX2    = aptRect[2].nX,    dY2 = aptRect[2].nY;
    QR_FLOAT dX3    = aptRect[3].nX,    dY3 = aptRect[3].nY;
    QR_FLOAT dWDen  = (QR_FLOAT)1 / (dW * (dX2*dY3 - dX3*dY2 + (dX3-dX2)*dY1 + dX1*(dY2-dY3)));
    QR_FLOAT dHDen  = (QR_FLOAT)1 / (dH * (dX2*dY3 + dX1*(dY2-dY3) - dX3*dY2 + (dX3-dX2)*dY1));
    adC[0] = (dX1*(dX2*dY3-dX3*dY2) + dX0*(-dX2*dY3+dX3*dY2+(dX2-dX3)*dY1) + dX1*(dX3-dX2)*dY0) * dWDen;
    adC[1] = -(dX0*(dX2*dY3+dX1*(dY2-dY3)-dX2*dY1) - dX1*dX3*dY2 + dX2*dX3*dY1 + (dX1*dX3-dX2*dX3)*dY0) * dHDen;
    adC[2] = dX0;
    adC[3] = (dY0*(dX1*(dY3-dY2)-dX2*dY3+dX3*dY2) + dY1*(dX2*dY3-dX3*dY2) + dX0*dY1*(dY2-dY3)) * dWDen;
    adC[4] = (dX0*(dY1*dY3-dY2*dY3) + dX1*dY2*dY3 - dX2*dY1*dY3 + dY0*(dX3*dY2-dX1*dY2+(dX2-dX3)*dY1)) * dHDen;
    adC[5] = dY0;
    adC[6] = (dX1*(dY3-dY2) + dX0*(dY2-dY3) + (dX2-dX3)*dY1 + (dX3-dX2)*dY0) * dWDen;
    adC[7] = (-dX2*dY3 + dX1*dY3 + dX3*dY2 + dX0*(dY1-dY2) - dX3*dY1 + (dX2-dX1)*dY0) * dHDen;
}

// Maps a grid point back through the perspective transform to get normalised (U, V) coordinates.
static VOID  QrPerspectiveUnmap(IN const QR_FLOAT* adC, IN const QR_POINT* pIn, OUT QR_FLOAT* pdU, OUT QR_FLOAT* pdV)
{
    QR_FLOAT dX     = pIn->nX;
    QR_FLOAT dY     = pIn->nY;
    QR_FLOAT dDen   = (QR_FLOAT)1 / (-adC[0]*adC[7]*dY + adC[1]*adC[6]*dY + (adC[3]*adC[7]-adC[4]*adC[6])*dX + adC[0]*adC[4] - adC[1]*adC[3]);
    *pdU = -(adC[1]*(dY-adC[5]) - adC[2]*adC[7]*dY + (adC[5]*adC[7]-adC[4])*dX + adC[2]*adC[4]) * dDen;
    *pdV =  (adC[0]*(dY-adC[5]) - adC[2]*adC[6]*dY + (adC[5]*adC[6]-adC[3])*dX + adC[2]*adC[3]) * dDen;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Flood Fill & Image Thresholding (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_FLOODFILL

// Fills a horizontal span of matching pixels at (nX, nY), returns the span bounds via pnLeft/pnRight.
static VOID  QrFloodFillLine(IN OUT PQR_CTX pCtx, IN INT nX, IN INT nY, IN INT nFrom, IN INT nTo, IN OPTIONAL QR_SPAN_FUNC pfnSpan, IN OPTIONAL PVOID pvUserData, OUT PINT pnLeft, OUT PINT pnRight)
{
    QR_PIXEL_T  *pRow   = pCtx->pbPixels + nY * pCtx->nWidth;
    INT         nLeft   = nX;
    INT         nRight  = nX;

    QR_ASSERT(pRow[nX] == nFrom);

    while (nLeft > 0 && pRow[nLeft - 1] == nFrom)
        nLeft--;
    while (nRight < pCtx->nWidth - 1 && pRow[nRight + 1] == nFrom)
        nRight++;

    for (INT nI = nLeft; nI <= nRight; nI++)
        pRow[nI] = (QR_PIXEL_T)nTo;

    *pnLeft     = nLeft;
    *pnRight    = nRight;

    if (pfnSpan)
        pfnSpan(pvUserData, nY, nLeft, nRight);
}

// Scans an adjacent row for the next unfilled span and pushes it onto the flood fill stack.
static PQR_FLOOD_VARS  QrFloodFillNext(IN OUT PQR_CTX pCtx, IN QR_PIXEL_T* pRow, IN INT nFrom, IN INT nTo, IN OPTIONAL QR_SPAN_FUNC pfnSpan, IN OPTIONAL PVOID pvUserData, IN OUT PQR_FLOOD_VARS pVars, IN INT nDir)
{
    PINT pnLeft = (nDir < 0) ? &pVars->nLeftUp : &pVars->nLeftDown;
    while (*pnLeft <= pVars->nRight)
    {
        if (pRow[*pnLeft] == nFrom)
        {
            PQR_FLOOD_VARS  pNext   = pVars + 1;
            INT             nLeft   = 0x00;
            pNext->nY = pVars->nY + nDir;
            QrFloodFillLine(pCtx, *pnLeft, pNext->nY, nFrom, nTo, pfnSpan, pvUserData, &nLeft, &pNext->nRight);
            pNext->nLeftDown    = nLeft;
            pNext->nLeftUp      = nLeft;
            return pNext;
        }
        (*pnLeft)++;
    }
    return NULL;
}

// Performs a stack-based flood fill from seed (nX0, nY0), replacing nFrom with nTo, with optional span callback.
static VOID  QrFloodFillSeed(IN OUT PQR_CTX pCtx, IN INT nX0, IN INT nY0, IN INT nFrom, IN INT nTo, IN OPTIONAL QR_SPAN_FUNC pfnSpan, IN OPTIONAL PVOID pvUserData)
{
    PQR_FLOOD_VARS      pStack  = pCtx->pFloodVars;
    PQR_FLOOD_VARS      pLast   = &pStack[pCtx->cbFloodVars - 1];
    PQR_FLOOD_VARS      pNext   = NULL;
    INT                 nLeft   = 0x00;

    QR_ASSERT(nFrom != nTo);
    QR_ASSERT(pCtx->pbPixels[nY0 * pCtx->nWidth + nX0] == nFrom);

    pNext       = pStack;
    pNext->nY   = nY0;
    QrFloodFillLine(pCtx, nX0, pNext->nY, nFrom, nTo, pfnSpan, pvUserData, &nLeft, &pNext->nRight);
    pNext->nLeftDown    = nLeft;
    pNext->nLeftUp      = nLeft;

    while (1)
    {
        PQR_FLOOD_VARS const    pVars   = pNext;
        QR_PIXEL_T              *pRow;

        if (pVars == pLast)
            break;

        if (pVars->nY > 0)
        {
            pRow  = pCtx->pbPixels + (pVars->nY - 1) * pCtx->nWidth;
            pNext = QrFloodFillNext(pCtx, pRow, nFrom, nTo, pfnSpan, pvUserData, pVars, -1);
            if (pNext) continue;
        }
        if (pVars->nY < pCtx->nHeight - 1)
        {
            pRow  = pCtx->pbPixels + (pVars->nY + 1) * pCtx->nWidth;
            pNext = QrFloodFillNext(pCtx, pRow, nFrom, nTo, pfnSpan, pvUserData, pVars, 1);
            if (pNext) continue;
        }
        if (pVars > pStack)
        {
            pNext = pVars - 1;
            continue;
        }
        break;
    }
}

// Computes the optimal binarisation threshold for the grayscale image using Otsu's method.
static BYTE  QrOtsuThreshold(IN const QR_CTX* pCtx)
{
    DWORD       dwNumPixels         = (DWORD)(pCtx->nWidth * pCtx->nHeight);
    DWORD       adwHistogram[256]   = { 0x00 };
    PBYTE       pbPtr               = pCtx->pbImage;
    DWORD       dwLength            = dwNumPixels;
    QR_FLOAT    dSum                = 0.0;
    QR_FLOAT    dSumB               = 0.0;
    DWORD       dwQ1                = 0x00;
    QR_FLOAT    dMax                = 0.0;
    BYTE        bThreshold          = 0x00;

    while (dwLength--)
        adwHistogram[*pbPtr++]++;

    for (DWORD dwI = 0; dwI <= 255; dwI++)
        dSum += dwI * adwHistogram[dwI];

    for (DWORD dwI = 0; dwI <= 255; dwI++)
    {
        DWORD       dwQ2;
        QR_FLOAT    dM1, dM2, dM1M2, dVariance;

        dwQ1 += adwHistogram[dwI];
        if (dwQ1 == 0x00) continue;
        dwQ2 = dwNumPixels - dwQ1;
        if (dwQ2 == 0x00) break;

        dSumB   += dwI * adwHistogram[dwI];
        dM1      = dSumB / dwQ1;
        dM2      = (dSum - dSumB) / dwQ2;
        dM1M2    = dM1 - dM2;
        dVariance = dM1M2 * dM1M2 * dwQ1 * dwQ2;

        if (dVariance >= dMax)
        {
            bThreshold  = (BYTE)dwI;
            dMax        = dVariance;
        }
    }

    return bThreshold;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Finder Pattern & Capstone Detection (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_CAPSTONE

// Span callback that accumulates the pixel count for a region.
static VOID  QrAreaCount(IN PVOID pvUserData, IN INT nY, IN INT nLeft, IN INT nRight)
{
    ((PQR_REGION)pvUserData)->nCount += nRight - nLeft + 1;
}

// Flood-fills a connected component at (nX, nY) and assigns it a region index.
static INT  QrRegionCode(IN OUT PQR_CTX pCtx, IN INT nX, IN INT nY)
{
    INT         nPixel;
    PQR_REGION  pBox;
    INT         nRegion;

    if (nX < 0 || nY < 0 || nX >= pCtx->nWidth || nY >= pCtx->nHeight)
        return -1;

    nPixel = pCtx->pbPixels[nY * pCtx->nWidth + nX];
    if (nPixel >= QR_PIXEL_REGION)  return nPixel;
    if (nPixel == QR_PIXEL_WHITE)   return -1;

    if (pCtx->nRegions >= QR_MAX_REGIONS)
    {
        // DBG("[-] Region Limit Reached (%d/%d) At Pixel (%d, %d)", pCtx->nRegions, QR_MAX_REGIONS, nX, nY);
        return -1;
    }

    nRegion = pCtx->nRegions;
    pBox    = &pCtx->aRegions[pCtx->nRegions++];
    RtlZeroMemory(pBox, sizeof(*pBox));
    pBox->ptSeed.nX = nX;
    pBox->ptSeed.nY = nY;
    pBox->nCapstone = -1;

    QrFloodFillSeed(pCtx, nX, nY, nPixel, nRegion, QrAreaCount, pBox);
    return nRegion;
}

// Span callback that finds the corner farthest from the reference point.
static VOID  QrFindOneCorner(IN PVOID pvUserData, IN INT nY, IN INT nLeft, IN INT nRight)
{
    PQR_POLY_SCORE  pPsd    = (PQR_POLY_SCORE)pvUserData;
    INT             anXs[2] = { nLeft, nRight };
    INT             nDy     = nY - pPsd->ptRef.nY;
    for (INT nI = 0; nI < 2; nI++)
    {
        INT nDx = anXs[nI] - pPsd->ptRef.nX;
        INT nD  = nDx * nDx + nDy * nDy;
        if (nD > pPsd->anScores[0])
        {
            pPsd->anScores[0]       = nD;
            pPsd->pCorners[0].nX    = anXs[nI];
            pPsd->pCorners[0].nY    = nY;
        }
    }
}

// Span callback that finds four extreme corners by projecting onto the reference axis and its perpendicular.
static VOID  QrFindOtherCorners(IN PVOID pvUserData, IN INT nY, IN INT nLeft, IN INT nRight)
{
    PQR_POLY_SCORE  pPsd    = (PQR_POLY_SCORE)pvUserData;
    INT             anXs[2] = { nLeft, nRight };
    for (INT nI = 0; nI < 2; nI++)
    {
        INT nUp     = anXs[nI] *  pPsd->ptRef.nX + nY *  pPsd->ptRef.nY;
        INT nRight  = anXs[nI] * -pPsd->ptRef.nY + nY *  pPsd->ptRef.nX;
        INT anS[4]  = { nUp, nRight, -nUp, -nRight };
        for (INT nJ = 0; nJ < 4; nJ++)
        {
            if (anS[nJ] > pPsd->anScores[nJ])
            {
                pPsd->anScores[nJ]      = anS[nJ];
                pPsd->pCorners[nJ].nX   = anXs[nI];
                pPsd->pCorners[nJ].nY   = nY;
            }
        }
    }
}

// Locates the four corners of a region by two-pass flood fill from the seed.
static VOID  QrFindRegionCorners(IN OUT PQR_CTX pCtx, IN INT nRCode, IN const QR_POINT* pRef, OUT PQR_POINT pCorners)
{
    PQR_REGION      pRegion = &pCtx->aRegions[nRCode];
    QR_POLY_SCORE   Psd     = { 0x00 };

    Psd.pCorners    = pCorners;
    RtlCopyMemory(&Psd.ptRef, pRef, sizeof(Psd.ptRef));
    Psd.anScores[0] = -1;

    QrFloodFillSeed(pCtx, pRegion->ptSeed.nX, pRegion->ptSeed.nY, nRCode, QR_PIXEL_BLACK, QrFindOneCorner, &Psd);

    Psd.ptRef.nX = Psd.pCorners[0].nX - Psd.ptRef.nX;
    Psd.ptRef.nY = Psd.pCorners[0].nY - Psd.ptRef.nY;

    for (INT nI = 0; nI < 4; nI++)
        RtlCopyMemory(&Psd.pCorners[nI], &pRegion->ptSeed, sizeof(Psd.pCorners[nI]));

    INT nScore          = pRegion->ptSeed.nX *  Psd.ptRef.nX + pRegion->ptSeed.nY *  Psd.ptRef.nY;
    Psd.anScores[0]     =  nScore;
    Psd.anScores[2]     = -nScore;
    nScore              = pRegion->ptSeed.nX * -Psd.ptRef.nY + pRegion->ptSeed.nY *  Psd.ptRef.nX;
    Psd.anScores[1]     =  nScore;
    Psd.anScores[3]     = -nScore;

    QrFloodFillSeed(pCtx, pRegion->ptSeed.nX, pRegion->ptSeed.nY, QR_PIXEL_BLACK, nRCode, QrFindOtherCorners, &Psd);
}

// Maps normalised grid coordinates (dU, dV) to image pixel coordinates via the perspective transform.
static VOID QrPerspectiveMap(IN const QR_FLOAT* adC, IN QR_FLOAT dU, IN QR_FLOAT dV, OUT PQR_POINT pPt)
{
    QR_FLOAT dDen = (QR_FLOAT)1 / (adC[6] * dU + adC[7] * dV + (QR_FLOAT)1.0);
    QR_FLOAT dX = (adC[0] * dU + adC[1] * dV + adC[2]) * dDen;
    QR_FLOAT dY = (adC[3] * dU + adC[4] * dV + adC[5]) * dDen;
    pPt->nX = (INT)rint(dX);
    pPt->nY = (INT)rint(dY);
}

// Registers a validated capstone (ring + stone pair) and computes its perspective transform.
static VOID  QrRecordCapstone(IN OUT PQR_CTX pCtx, IN INT nRing, IN INT nStone)
{
    PQR_CAPSTONE    pCap;
    INT             nCsIdx;

    if (pCtx->nCapstones >= QR_MAX_CAPSTONES)
        return;

    nCsIdx  = pCtx->nCapstones;
    pCap    = &pCtx->aCapstones[pCtx->nCapstones++];
    RtlZeroMemory(pCap, sizeof(*pCap));

    pCap->nGrid     = -1;
    pCap->nRing     = nRing;
    pCap->nStone    = nStone;

    pCtx->aRegions[nStone].nCapstone    = nCsIdx;
    pCtx->aRegions[nRing].nCapstone     = nCsIdx;

    QrFindRegionCorners(pCtx, nRing, &pCtx->aRegions[nStone].ptSeed, pCap->aptCorners);
    QrPerspectiveSetup(pCap->adPerspective, pCap->aptCorners, (QR_FLOAT)QR_FINDER_SIZE, (QR_FLOAT)QR_FINDER_SIZE);
    QrPerspectiveMap(pCap->adPerspective, QR_FINDER_CENTER, QR_FINDER_CENTER, &pCap->ptCenter);
}

// Validates a candidate capstone by checking region identity and area ratio.
static VOID  QrTestCapstone(IN OUT PQR_CTX pCtx, IN DWORD dwX, IN DWORD dwY, IN DWORD* pdwPb)
{
    // sample three points along the scanline to get region codes for ring and stone
    INT         nRingRight  = QrRegionCode(pCtx, dwX - pdwPb[4], dwY);
    INT         nStone      = QrRegionCode(pCtx, dwX - pdwPb[4] - pdwPb[3] - pdwPb[2], dwY);
    INT         nRingLeft   = QrRegionCode(pCtx, dwX - pdwPb[4] - pdwPb[3] - pdwPb[2] - pdwPb[1] - pdwPb[0], dwY);
    PQR_REGION  pStoneReg;
    PQR_REGION  pRingReg;
    DWORD       dwRatio;

    if (nRingLeft < 0 || nRingRight < 0 || nStone < 0)      return;
    if (nRingLeft != nRingRight)                            return; // left and right ring must be same region
    if (nRingLeft == nStone)                                return; // stone must be a different region

    pStoneReg   = &pCtx->aRegions[nStone];
    pRingReg    = &pCtx->aRegions[nRingLeft];

    // already claimed by another capstone
    if (pStoneReg->nCapstone >= 0 || pRingReg->nCapstone >= 0)
        return;

    // stone-to-ring area ratio
    dwRatio = (DWORD)pStoneReg->nCount * 100 / (DWORD)pRingReg->nCount;

    // ideal stone/ring area ratio is 9/24 ≈ 37%, bounds allow for perspective distortion and noise
    if (dwRatio < QR_CAPSTONE_RATIO_MIN || dwRatio > QR_CAPSTONE_RATIO_MAX)
        return;

    QrRecordCapstone(pCtx, nRingLeft, nStone);
}

// Scans a single row for 1:1:3:1:1 dark/light finder patterns and tests each candidate.
static VOID  QrFinderScan(IN OUT PQR_CTX pCtx, IN DWORD dwY)
{
    QR_PIXEL_T  *pRow                       = pCtx->pbPixels + dwY * pCtx->nWidth;
    DWORD       dwX                         = 0x00;
    INT         nLastColor                  = 0x00;
    DWORD       dwRunLen                    = 0x00;
    DWORD       dwRunCount                  = 0x00;
    DWORD       adwPb[QR_FINDER_SEGMENTS]   = { 0 };

    for (dwX = 0; dwX < (DWORD)pCtx->nWidth; dwX++)
    {
        INT nColor = pRow[dwX] ? 1 : 0;

        // colour transition detected
        if (dwX && nColor != nLastColor)
        {
            // shift run history left
            RtlMoveMemory(adwPb, adwPb + 1, sizeof(adwPb[0]) * (QR_FINDER_SEGMENTS - 1));
            adwPb[4]    = dwRunLen;
            dwRunLen    = 0x00;
            dwRunCount++;

            // after at least 5 runs, check if the pattern matches 1:1:3:1:1
            if (!nColor && dwRunCount >= QR_FINDER_SEGMENTS)
            {
                static const DWORD  adwCheck[QR_FINDER_SEGMENTS]    = { 1, 1, 3, 1, 1 };
                const INT           nScale                          = 16;
                DWORD               dwAvg                           = 0x00,
                                    dwErr                           = 0x00;
                INT                 nOk                             = 0x01;

                // average of the 1-width runs
                dwAvg = (adwPb[0] + adwPb[1] + adwPb[3] + adwPb[4]) * nScale / 4;
                // tolerance window
                dwErr = dwAvg * 3 / 4;

                for (DWORD dwI = 0; dwI < QR_FINDER_SEGMENTS; dwI++)
                    if (adwPb[dwI] * nScale < adwCheck[dwI] * dwAvg - dwErr || adwPb[dwI] * nScale > adwCheck[dwI] * dwAvg + dwErr)
                        nOk = 0;

                if (nOk)
                    QrTestCapstone(pCtx, dwX, dwY, adwPb);
            }
        }
        dwRunLen++;
        nLastColor = nColor;
    }
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Perspective Fitness & Refinement (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_FITNESS

// Samples a single grid cell through the perspective transform, returns +1 (dark) or -1 (light).
static INT  QrCellValue(IN const PQR_CTX pCtx, IN INT nIdx, IN INT nX, IN INT nY)
{
    const QR_GRID   *pGrid  = &pCtx->aGrids[nIdx];
    QR_POINT        pt      = { 0x00 };

    QrPerspectiveMap(pGrid->adPerspective, nX + 0.5, nY + 0.5, &pt);
    if (pt.nY < 0 || pt.nY >= pCtx->nHeight || pt.nX < 0 || pt.nX >= pCtx->nWidth)
        return 0;
    return pCtx->pbPixels[pt.nY * pCtx->nWidth + pt.nX] ? 1 : -1;
}

// Samples a cell at 3x3 sub-positions and returns a weighted dark/light fitness score.
static INT  QrFitnessCell(IN const PQR_CTX pCtx, IN INT nIdx, IN INT nX, IN INT nY)
{
    static const QR_FLOAT   adOff[] = { 0.3, 0.5, 0.7 };
    const QR_GRID           *pGrid  = &pCtx->aGrids[nIdx];
    INT                     nScore  = 0x00;

    for (INT v = 0; v < 3; v++)
    {
        for (INT u = 0; u < 3; u++)
        {
            QR_POINT pt = { 0 };
            QrPerspectiveMap(pGrid->adPerspective, nX + adOff[u], nY + adOff[v], &pt);
            if (pt.nY < 0 || pt.nY >= pCtx->nHeight || pt.nX < 0 || pt.nX >= pCtx->nWidth)
                continue;
            if (pCtx->pbPixels[pt.nY * pCtx->nWidth + pt.nX])  nScore++;
            else                                                nScore--;
        }
    }
    return nScore;
}

// Sums the fitness of all cells on the perimeter of a square ring of radius nR.
static INT  QrFitnessRing(IN const PQR_CTX pCtx, IN INT nIdx, IN INT nCx, IN INT nCy, IN INT nR)
{
    INT nScore = 0x00;
    for (INT nI = 0; nI < nR * 2; nI++)
    {
        nScore += QrFitnessCell(pCtx, nIdx, nCx - nR + nI, nCy - nR);
        nScore += QrFitnessCell(pCtx, nIdx, nCx - nR,      nCy + nR - nI);
        nScore += QrFitnessCell(pCtx, nIdx, nCx + nR,      nCy - nR + nI);
        nScore += QrFitnessCell(pCtx, nIdx, nCx + nR - nI, nCy + nR);
    }
    return nScore;
}

// Scores an alignment pattern candidate: dark center, light inner ring, dark outer ring.
static INT  QrFitnessApat(IN const PQR_CTX pCtx, IN INT nIdx, IN INT nCx, IN INT nCy)
{
    return  QrFitnessCell(pCtx, nIdx, nCx, nCy)
          - QrFitnessRing(pCtx, nIdx, nCx, nCy, 1)
          + QrFitnessRing(pCtx, nIdx, nCx, nCy, 2);
}

// Scores a finder pattern candidate: dark center, dark inner, light middle, dark outer rings.
static INT  QrFitnessCapstone(IN const PQR_CTX pCtx, IN INT nIdx, IN INT nX, IN INT nY)
{
    nX += QR_FINDER_SIZE / 2;
    nY += QR_FINDER_SIZE / 2;
    return  QrFitnessCell(pCtx, nIdx, nX, nY)
        + QrFitnessRing(pCtx, nIdx, nX, nY, 1)
        - QrFitnessRing(pCtx, nIdx, nX, nY, 2)
        + QrFitnessRing(pCtx, nIdx, nX, nY, 3);
}

// Computes the total fitness score for a grid by evaluating timing, finders, and alignment patterns.
static INT  QrFitnessAll(IN const PQR_CTX pCtx, IN INT nIdx)
{
    const QR_GRID       *pGrid      = &pCtx->aGrids[nIdx];
    INT                 nVersion    = (pGrid->nSize - QR_BASE_SIZE) / QR_VER_INCREMENT;
    const QR_VER_INFO   *pInfo      = &g_aQrVersionDb[nVersion];
    INT                 nScore      = 0x00;
    INT                 nApCount    = 0x00;

    for (INT nI = 0; nI < pGrid->nSize - QR_FINDER_SIZE * 2; nI++)
    {
        INT nExpect = (nI & 1) ? 1 : -1;
        nScore += QrFitnessCell(pCtx, nIdx, nI + QR_FINDER_SIZE, QR_TIMING_POS) * nExpect;
        nScore += QrFitnessCell(pCtx, nIdx, QR_TIMING_POS, nI + QR_FINDER_SIZE) * nExpect;
    }

    nScore += QrFitnessCapstone(pCtx, nIdx, 0, 0);
    nScore += QrFitnessCapstone(pCtx, nIdx, pGrid->nSize - QR_FINDER_SIZE, 0);
    nScore += QrFitnessCapstone(pCtx, nIdx, 0, pGrid->nSize - QR_FINDER_SIZE);

    if (nVersion < 0 || nVersion > QR_VERSION_MAX)
        return nScore;

    while (nApCount < QR_MAX_ALIGNMENT && pInfo->anAPat[nApCount])
        nApCount++;

    for (INT nI = 1; nI + 1 < nApCount; nI++)
    {
        nScore += QrFitnessApat(pCtx, nIdx, QR_TIMING_POS, pInfo->anAPat[nI]);
        nScore += QrFitnessApat(pCtx, nIdx, pInfo->anAPat[nI], QR_TIMING_POS);
    }

    for (INT nI = 1; nI < nApCount; nI++)
        for (INT nJ = 1; nJ < nApCount; nJ++)
            nScore += QrFitnessApat(pCtx, nIdx, pInfo->anAPat[nI], pInfo->anAPat[nJ]);

    return nScore;
}

// Iteratively nudges the perspective coefficients to maximise the overall fitness score.
static VOID  QrJigglePerspective(IN OUT PQR_CTX pCtx, IN INT nIdx)
{
    PQR_GRID    pGrid                           = &pCtx->aGrids[nIdx];
    INT         nBest                           = QrFitnessAll(pCtx, nIdx);
    QR_FLOAT    adAdj[QR_PERSPECTIVE_PARAMS]    = { 0.0 };

    for (INT nI = 0; nI < QR_PERSPECTIVE_PARAMS; nI++)
        adAdj[nI] = pGrid->adPerspective[nI] * QR_JIGGLE_INIT_STEP; 

    for (INT nPass = 0; nPass < QR_JIGGLE_PASSES; nPass++)
    {
        for (INT nI = 0; nI < QR_PERSPECTIVE_PARAMS * 2; nI++) // try +/- for each coefficient
        {
            INT         nJ      = nI >> 1;
            INT         nTest   = 0x00;
            QR_FLOAT    dOld    = pGrid->adPerspective[nJ];
            QR_FLOAT    dNew    = (nI & 1) ? dOld + adAdj[nJ] : dOld - adAdj[nJ];

            pGrid->adPerspective[nJ] = dNew;
            nTest = QrFitnessAll(pCtx, nIdx);
            if (nTest > nBest)  nBest = nTest;                   // keep improvement
            else                pGrid->adPerspective[nJ] = dOld; // revert
        }
        for (INT nI = 0; nI < QR_PERSPECTIVE_PARAMS; nI++)
            adAdj[nI] *= QR_JIGGLE_DECAY; // halve the step for next pass
    }
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Grid Assembly & Alignment (Decode)
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region DECODE_GRID

// Returns the Euclidean distance between two points.
static QR_FLOAT  QrLength(IN QR_POINT a, IN QR_POINT b)
{
    QR_FLOAT dX = a.nX - b.nX;
    QR_FLOAT dY = a.nY - b.nY;
    return sqrt(dX * dX + dY * dY);
}

// Estimates the grid module count from the capstone-to-capstone distances.
static VOID  QrMeasureGridSize(IN OUT PQR_CTX pCtx, IN INT nIdx)
{
    PQR_GRID        pGrid   = &pCtx->aGrids[nIdx];
    PQR_CAPSTONE    pA      = &pCtx->aCapstones[pGrid->anCaps[0]];
    PQR_CAPSTONE    pB      = &pCtx->aCapstones[pGrid->anCaps[1]];
    PQR_CAPSTONE    pC      = &pCtx->aCapstones[pGrid->anCaps[2]];
    QR_FLOAT        dAb     = QrLength(pB->aptCorners[0], pA->aptCorners[3]);
    QR_FLOAT        dCapAb  = (QrLength(pB->aptCorners[0], pB->aptCorners[3]) + QrLength(pA->aptCorners[0], pA->aptCorners[3])) / 2.0;
    QR_FLOAT        dBc     = QrLength(pB->aptCorners[0], pC->aptCorners[1]);
    QR_FLOAT        dCapBc  = (QrLength(pB->aptCorners[0], pB->aptCorners[1]) + QrLength(pC->aptCorners[0], pC->aptCorners[1])) / 2.0;
    QR_FLOAT        dEst    = ((QR_FLOAT)QR_FINDER_SIZE * dAb / dCapAb + (QR_FLOAT)QR_FINDER_SIZE * dBc / dCapBc) * 0.5;
    INT             nVer    = (INT)((dEst - (QR_FLOAT)QR_BASE_SIZE) * (1.0 / QR_VER_INCREMENT));

    pGrid->nSize            = QR_VER_INCREMENT * nVer + QR_BASE_SIZE;
}

// Rotates a capstone's corners so corner[0] is closest to the reference line.
static VOID  QrRotateCapstone(IN OUT PQR_CAPSTONE pCap, IN const QR_POINT* pH0, IN const QR_POINT* pHd)
{
    QR_POINT    aCopy[4]    = { 0 };
    INT         nBest       = 0x00;
    INT         nBestScore  = 0x7FFFFFFF;

    for (INT nJ = 0; nJ < 4; nJ++)
    {
        INT nScore = (pCap->aptCorners[nJ].nX - pH0->nX) * -pHd->nY
                   + (pCap->aptCorners[nJ].nY - pH0->nY) *  pHd->nX;
        if (!nJ || nScore < nBestScore)
        {
            nBest       = nJ;
            nBestScore  = nScore;
        }
    }

    for (INT nJ = 0; nJ < 4; nJ++)
        RtlCopyMemory(&aCopy[nJ], &pCap->aptCorners[(nJ + nBest) % 4], sizeof(aCopy[nJ]));

    RtlCopyMemory(pCap->aptCorners, aCopy, sizeof(pCap->aptCorners));
    QrPerspectiveSetup(pCap->adPerspective, pCap->aptCorners, (QR_FLOAT)QR_FINDER_SIZE, (QR_FLOAT)QR_FINDER_SIZE);
}

// Spiral-searches around the estimated position for the bottom-right alignment pattern.
static VOID  QrFindAlignmentPattern(IN OUT PQR_CTX pCtx, IN INT nIdx)
{
    PQR_GRID        pGrid       = &pCtx->aGrids[nIdx];
    PQR_CAPSTONE    pC0         = &pCtx->aCapstones[pGrid->anCaps[0]];
    PQR_CAPSTONE    pC2         = &pCtx->aCapstones[pGrid->anCaps[2]];
    QR_POINT        ptA, ptB, ptC;
    INT             nSizeEst    = 0x00;
    INT             nStepSize   = 1;
    INT             nDir        = 0x00;
    QR_FLOAT        dU, dV;

    RtlCopyMemory(&ptB, &pGrid->ptAlign, sizeof(ptB));

    QrPerspectiveUnmap(pC0->adPerspective, &ptB, &dU, &dV);
    QrPerspectiveMap(pC0->adPerspective, dU, dV + 1.0, &ptA);
    QrPerspectiveUnmap(pC2->adPerspective, &ptB, &dU, &dV);
    QrPerspectiveMap(pC2->adPerspective, dU + 1.0, dV, &ptC);

    nSizeEst = abs((ptA.nX - ptB.nX) * -(ptC.nY - ptB.nY)
                 + (ptA.nY - ptB.nY) *  (ptC.nX - ptB.nX));

    while (nStepSize * nStepSize < nSizeEst * 100)
    {
        static const INT    anDx[] = { 1, 0, -1, 0 };
        static const INT    anDy[] = { 0, -1, 0, 1 };

        for (INT nI = 0; nI < nStepSize; nI++)
        {
            INT nCode = QrRegionCode(pCtx, ptB.nX, ptB.nY);
            if (nCode >= 0)
            {
                PQR_REGION pReg = &pCtx->aRegions[nCode];
                if (pReg->nCount >= nSizeEst / 2 && pReg->nCount <= nSizeEst * 2)
                {
                    pGrid->nAlignRegion = nCode;
                    return;
                }
            }
            ptB.nX += anDx[nDir];
            ptB.nY += anDy[nDir];
        }
        nDir = (nDir + 1) % 4;
        if (!(nDir & 1))
            nStepSize++;
    }
}

// Span callback that finds the point leftmost relative to the reference direction.
static VOID  QrFindLeftmostToLine(IN PVOID pvUserData, IN INT nY, IN INT nLeft, IN INT nRight)
{
    PQR_POLY_SCORE  pPsd    = (PQR_POLY_SCORE)pvUserData;
    INT             anXs[2] = { nLeft, nRight };
    for (INT nI = 0; nI < 2; nI++)
    {
        INT nD = -pPsd->ptRef.nY * anXs[nI] + pPsd->ptRef.nX * nY;
        if (nD < pPsd->anScores[0])
        {
            pPsd->anScores[0]       = nD;
            pPsd->pCorners[0].nX    = anXs[nI];
            pPsd->pCorners[0].nY    = nY;
        }
    }
}

// Assembles a full QR grid from three capstones, locates alignment, and computes the final perspective.
static VOID  QrRecordGrid(IN OUT PQR_CTX pCtx, IN INT a, IN INT b, IN INT c)
{
    QR_POINT    ptH0    = { 0 }; // reference origin (capstone A center)
    QR_POINT    ptHd    = { 0 }; // direction vector from A to C
    INT         nIdx    = 0x00;
    PQR_GRID    pGrid   = NULL;
    INT         nSwap   = 0x00;

    if (pCtx->nGrids >= QR_MAX_GRIDS)
        return;

    RtlCopyMemory(&ptH0, &pCtx->aCapstones[a].ptCenter, sizeof(ptH0));
    ptHd.nX = pCtx->aCapstones[c].ptCenter.nX - pCtx->aCapstones[a].ptCenter.nX;
    ptHd.nY = pCtx->aCapstones[c].ptCenter.nY - pCtx->aCapstones[a].ptCenter.nY;

    // ensure B is on the correct side (cross product sign check); swap A/C if needed
    if ((pCtx->aCapstones[b].ptCenter.nX - ptH0.nX) * -ptHd.nY +
        (pCtx->aCapstones[b].ptCenter.nY - ptH0.nY) *  ptHd.nX > 0)
    {
        nSwap = a; a = c; c = nSwap;
        ptHd.nX = -ptHd.nX;
        ptHd.nY = -ptHd.nY;
    }

    nIdx    = pCtx->nGrids;
    pGrid   = &pCtx->aGrids[pCtx->nGrids++];
    
    RtlZeroMemory(pGrid, sizeof(*pGrid));
    
    pGrid->anCaps[0]    = a; // top-left finder
    pGrid->anCaps[1]    = b; // top-right finder
    pGrid->anCaps[2]    = c; // bottom-left finder
    pGrid->nAlignRegion = -1;

    // rotate each capstone's corners to canonical orientation
    for (INT nI = 0; nI < 3; nI++)
    {
        PQR_CAPSTONE pCap = &pCtx->aCapstones[pGrid->anCaps[nI]];
        QrRotateCapstone(pCap, &ptH0, &ptHd);
        pCap->nGrid = nIdx;
    }

    QrMeasureGridSize(pCtx, nIdx);

    // estimate bottom-right corner by intersecting edges of capstones A and C
    if (!QrLineIntersect(&pCtx->aCapstones[a].aptCorners[0], &pCtx->aCapstones[a].aptCorners[1],
                         &pCtx->aCapstones[c].aptCorners[0], &pCtx->aCapstones[c].aptCorners[3],
                         &pGrid->ptAlign))
        goto _FAIL_FUNC_PATH;

    // for larger codes, refine with the actual alignment pattern
    if (pGrid->nSize > (QR_VERSION_MIN * QR_VER_INCREMENT + QR_BASE_SIZE))
    {
        QrFindAlignmentPattern(pCtx, nIdx);
        if (pGrid->nAlignRegion >= 0)
        {
            // snap to the leftmost point of the alignment region relative to ptHd
            QR_POLY_SCORE   Psd     = { 0x00 };
            PQR_REGION      pReg    = &pCtx->aRegions[pGrid->nAlignRegion];
            RtlCopyMemory(&pGrid->ptAlign, &pReg->ptSeed, sizeof(pGrid->ptAlign));
            RtlCopyMemory(&Psd.ptRef, &ptHd, sizeof(Psd.ptRef));
            Psd.pCorners        = &pGrid->ptAlign;
            Psd.anScores[0]     = -ptHd.nY * pGrid->ptAlign.nX + ptHd.nX * pGrid->ptAlign.nY;
            QrFloodFillSeed(pCtx, pReg->ptSeed.nX, pReg->ptSeed.nY, pGrid->nAlignRegion, QR_PIXEL_BLACK, NULL, NULL);
            QrFloodFillSeed(pCtx, pReg->ptSeed.nX, pReg->ptSeed.nY, QR_PIXEL_BLACK, pGrid->nAlignRegion, QrFindLeftmostToLine, &Psd);
        }
    }

    // build perspective transform from the four corner points
    QrPerspectiveSetup(pGrid->adPerspective,
        (QR_POINT[]) {
            pCtx->aCapstones[pGrid->anCaps[1]].aptCorners[0],
            pCtx->aCapstones[pGrid->anCaps[2]].aptCorners[0],
            pGrid->ptAlign,
            pCtx->aCapstones[pGrid->anCaps[0]].aptCorners[0]
        },
        pGrid->nSize - QR_FINDER_SIZE, pGrid->nSize - QR_FINDER_SIZE);

    QrJigglePerspective(pCtx, nIdx);
    return;

_FAIL_FUNC_PATH:
    for (INT nI = 0; nI < 3; nI++)
        pCtx->aCapstones[pGrid->anCaps[nI]].nGrid = -1;
    pCtx->nGrids--;
}

// Tests all horizontal/vertical neighbour pairs for squareness and records valid grids.
static VOID  QrTestNeighbours(IN OUT PQR_CTX pCtx, IN INT nI, IN const QR_NEIGHBOUR_LIST* pHList, IN const QR_NEIGHBOUR_LIST* pVList)
{
    for (INT nJ = 0; nJ < pHList->nCount; nJ++)
    {
        const QR_NEIGHBOUR* pHN = &pHList->aN[nJ];
        for (INT nK = 0; nK < pVList->nCount; nK++)
        {
            const QR_NEIGHBOUR* pVN         = &pVList->aN[nK];
            QR_FLOAT            dSquareness = fabs((QR_FLOAT)1.0 - pHN->dDistance / pVN->dDistance);
            if (dSquareness < QR_SQUARENESS_THRESHOLD)
                QrRecordGrid(pCtx, pHN->nIndex, nI, pVN->nIndex);
        }
    }
}

#pragma endregion


// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Public Encode APIs
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region PUBLIC_ENCODE_API

// Returns the side length (in modules) of the encoded QR code.
INT  QrGetSize(IN const BYTE* pbQrCode)
{
    if (!pbQrCode) return 0;
    return (INT)pbQrCode[0];
}

// Returns the module value at (nX, nY), with bounds checking.
BOOL  QrGetModule(IN const BYTE* pbQrCode, IN INT nX, IN INT nY)
{
    if (!pbQrCode) return FALSE;
    INT nQrSize = (INT)pbQrCode[0];
    if (nX < 0 || nX >= nQrSize || nY < 0 || nY >= nQrSize)
        return FALSE;
    return QriGetModuleBounded(pbQrCode, nX, nY);
}

// Encodes pbData into a QR code, auto-selecting the best version and mask.
BOOL  QrEncode(IN const BYTE* pbData, IN DWORD cbData, OUT PBYTE pbQrCode, IN QR_ECC eEcc, IN INT nMinVersion, IN INT nMaxVersion)
{
    BOOL    bResult     = FALSE;
    PBYTE   pbScratch   = NULL;
    SIZE_T  cbScratch   = (SIZE_T)QR_BUFFER_LEN_FOR_VERSION(nMaxVersion);

    if (!pbData || !pbQrCode)
        goto _END_OF_FUNC;
    if (nMinVersion < QR_VERSION_MIN || nMinVersion > nMaxVersion || nMaxVersion > QR_VERSION_MAX)
        goto _END_OF_FUNC;
    if ((INT)eEcc < 0 || (INT)eEcc > 3)
        goto _END_OF_FUNC;


    HEAP_ALLOC(pbScratch, cbScratch);
    if (!pbScratch) goto _END_OF_FUNC;

    RtlCopyMemory(pbScratch, pbData, cbData);

    // 1. Find minimum version that fits the data
    INT nVersion        = nMinVersion;
    INT nDataUsedBits   = 0;
    for (nVersion = nMinVersion; ; nVersion++)
    {
        INT nCapBits  = QriGetNumDataCodewords(nVersion, eEcc) * 8;
        nDataUsedBits = QriCalcByteBitLen(cbData, nVersion);
        if (nDataUsedBits != QR_LENGTH_OVERFLOW && nDataUsedBits <= nCapBits)
            break;
        if (nVersion >= nMaxVersion)
        {
            DBG("[!] Data Too Large For Version Range %d-%d At ECC Level %d (%lu Bytes, %d Bits Needed)", nMinVersion, nMaxVersion, (INT)eEcc, cbData, nDataUsedBits);
            pbQrCode[0] = 0;
            goto _END_OF_FUNC;
        }
    }

    // 2. Optionally boost ECC level if data still fits
    for (INT nI = (INT)QR_ECC_MEDIUM; nI <= (INT)QR_ECC_HIGH; nI++)
        if (nDataUsedBits <= QriGetNumDataCodewords(nVersion, (QR_ECC)nI) * 8)
            eEcc = (QR_ECC)nI;

    // 3. Build bit stream into pbQrCode (used as temp here)
    {
        INT nCapBits    = QriGetNumDataCodewords(nVersion, eEcc) * 8;
        INT nCcBits     = QriByteModeCcBits(nVersion);
        INT nBitLen     = 0;

        RtlZeroMemory(pbQrCode, cbScratch);
        QriAppendBits(QR_MODE_BYTE, QR_MODE_INDICATOR_BITS, pbQrCode, &nBitLen);    // Mode indicator (BYTE)
        QriAppendBits((UINT)cbData, nCcBits, pbQrCode, &nBitLen);                   // Character count
        for (DWORD nI = 0; nI < cbData; nI++)                                       // Data bits
            QriAppendBits(pbData[nI], 8, pbQrCode, &nBitLen);

        INT nTermBits = nCapBits - nBitLen;
        if (nTermBits > QR_MODE_INDICATOR_BITS) nTermBits = QR_MODE_INDICATOR_BITS;
        QriAppendBits(0, nTermBits, pbQrCode, &nBitLen);                            // Terminator
        QriAppendBits(0, (8 - nBitLen % 8) % 8, pbQrCode, &nBitLen);               // Byte-align

        for (BYTE bPad = QR_PAD_BYTE_A; nBitLen < nCapBits; bPad ^= (QR_PAD_BYTE_A ^ QR_PAD_BYTE_B))
            QriAppendBits(bPad, 8, pbQrCode, &nBitLen);                             // Pad codewords

        // 4. Interleave + add ECC
        QriAddEccAndInterleave(pbQrCode, nVersion, eEcc, pbScratch);
    }

    // 5. Draw function modules, then codewords, then light modules
    QriInitFunctionModules(nVersion, pbQrCode);
    QriDrawCodewords(pbScratch, QriGetNumRawDataModules(nVersion) / 8, pbQrCode);
    QriDrawLightFunctionModules(pbQrCode, nVersion);

    // 6. Save function module map into scratch for masking
    QriInitFunctionModules(nVersion, pbScratch);

    // 7. Auto-select best mask
    {
        INT  nBestMask      = 0;
        LONG lMinPenalty    = LONG_MAX;
        for (INT nI = 0; nI < QR_NUM_MASKS; nI++)
        {
            QriApplyMask(pbScratch, pbQrCode, nI);
            QriDrawFormatBits(eEcc, nI, pbQrCode);
            LONG lPenalty = QriGetPenaltyScore(pbQrCode);
            if (lPenalty < lMinPenalty)
            {
                nBestMask   = nI;
                lMinPenalty = lPenalty;
            }
            QriApplyMask(pbScratch, pbQrCode, nI);      // Undo (XOR is its own inverse)
        }
        QriApplyMask(pbScratch, pbQrCode, nBestMask);   // apply the winning mask
        QriDrawFormatBits(eEcc, nBestMask, pbQrCode);    // write final format info
    }

    bResult = TRUE;

_END_OF_FUNC:
    if (pbScratch) HEAP_FREE(pbScratch);
    return bResult;
}

#pragma endregion

// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==
// Public Decode APIs
// ==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==-==

#pragma region PUBLIC_DECODE_API

// Resizes the decode context buffers (image, pixel map, flood fill stack) to the new dimensions.
BOOL QrResize(IN OUT PQR_CTX pCtx, IN INT nWidth, IN INT nHeight)
{
    PBYTE           pbImage     = NULL;
    QR_PIXEL_T      *pbPixels   = NULL;
    PQR_FLOOD_VARS  pFloodVars  = NULL;
    SIZE_T          cbOld, cbNew, cbMin, cbVars, cVars;
    BOOL            bResult     = FALSE;

    if (!pCtx || nWidth <= 0 || nHeight <= 0)
        goto _END_OF_FUNC;

    HEAP_ALLOC(pbImage, ((SIZE_T)nWidth * nHeight));
    if (!pbImage) goto _END_OF_FUNC;

    cbOld = (SIZE_T)pCtx->nWidth * pCtx->nHeight;
    cbNew = (SIZE_T)nWidth * nHeight;
    cbMin = (cbOld < cbNew) ? cbOld : cbNew;
    
    if (cbMin) RtlCopyMemory(pbImage, pCtx->pbImage, cbMin);

    HEAP_ALLOC(pbPixels, cbNew * sizeof(QR_PIXEL_T));
    if (!pbPixels) goto _END_OF_FUNC;
    
    if (nHeight > (SIZE_T_MAX / 2))
        goto _END_OF_FUNC;
    
    cVars = (SIZE_T)nHeight * 2;

    if (cVars == 0x00) cVars = 1;
    
    cbVars = sizeof(QR_FLOOD_VARS) * cVars;
    
    if (cbVars / sizeof(QR_FLOOD_VARS) != cVars)
        goto _END_OF_FUNC;

    HEAP_ALLOC(pFloodVars, cbVars);
    if (!pFloodVars) goto _END_OF_FUNC;

    pCtx->nWidth    = nWidth;
    pCtx->nHeight   = nHeight;

    if (pCtx->pbImage)      HEAP_FREE(pCtx->pbImage);
    if (pCtx->pbPixels)     HEAP_FREE(pCtx->pbPixels);
    if (pCtx->pFloodVars)   HEAP_FREE(pCtx->pFloodVars);
    
    pCtx->pbImage       = pbImage;      pbImage     = NULL;
    pCtx->pbPixels      = pbPixels;     pbPixels    = NULL;
    pCtx->pFloodVars    = pFloodVars;   pFloodVars  = NULL;
    pCtx->cbFloodVars   = cVars;
    bResult = TRUE;

_END_OF_FUNC:
    if (pbImage)    HEAP_FREE(pbImage);
    if (pbPixels)   HEAP_FREE(pbPixels);
    if (pFloodVars) HEAP_FREE(pFloodVars);
    return bResult;
}

// Frees all heap allocations owned by the decode context.
VOID QrDestroy(IN OUT PQR_CTX pCtx)
{
    if (!pCtx)
        return;
    if (pCtx->pbImage)      HEAP_FREE(pCtx->pbImage);
    if (pCtx->pbPixels)     HEAP_FREE(pCtx->pbPixels);
    if (pCtx->pFloodVars)   HEAP_FREE(pCtx->pFloodVars);
    pCtx->pbImage       = NULL;
    pCtx->pbPixels      = NULL;
    pCtx->pFloodVars    = NULL;
}

// Resets the decode context for a new frame and returns a pointer to the grayscale image buffer.
PBYTE QrBegin(IN OUT PQR_CTX pCtx, OUT OPTIONAL PINT pnWidth, OUT OPTIONAL PINT pnHeight)
{
    if (!pCtx)
        return NULL;
    pCtx->nRegions      = QR_PIXEL_REGION;
    pCtx->nCapstones    = 0x00;
    pCtx->nGrids        = 0x00;
    if (pnWidth)    *pnWidth    = pCtx->nWidth;
    if (pnHeight)   *pnHeight   = pCtx->nHeight;
    return pCtx->pbImage;
}

// Binarises the grayscale image into black/white pixel values using the given threshold.
static VOID  QrPixelsSetup(IN OUT PQR_CTX pCtx, IN BYTE bThreshold)
{
    PBYTE       pbSrc   = pCtx->pbImage;
    QR_PIXEL_T  *pbDst  = pCtx->pbPixels;
    INT         nLen    = pCtx->nWidth * pCtx->nHeight;
    while (nLen--)
    {
        BYTE bVal   = *pbSrc++;
        *pbDst++    = (bVal < bThreshold) ? QR_PIXEL_BLACK : QR_PIXEL_WHITE;

    }
}

// Groups a capstone with its horizontal and vertical neighbours to form candidate grids.
static VOID QrTestGrouping(IN OUT PQR_CTX pCtx, IN INT nI)
{
    PQR_CAPSTONE        pC1     = &pCtx->aCapstones[nI];
    QR_NEIGHBOUR_LIST   HList   = { 0x00 };
    QR_NEIGHBOUR_LIST   VList   = { 0x00 };

    for (INT nJ = 0; nJ < pCtx->nCapstones; nJ++)
    {
        PQR_CAPSTONE    pC2 = &pCtx->aCapstones[nJ];
        QR_FLOAT        dU, dV;

        if (nI == nJ)
            continue;

        QrPerspectiveUnmap(pC1->adPerspective, &pC2->ptCenter, &dU, &dV);
        dU = fabs(dU - QR_FINDER_CENTER);
        dV = fabs(dV - QR_FINDER_CENTER);

        if (dU < QR_SQUARENESS_THRESHOLD * dV)
        {
            PQR_NEIGHBOUR pN    = &HList.aN[HList.nCount++];
            pN->nIndex          = nJ;
            pN->dDistance       = dV;
        }
        if (dV < QR_SQUARENESS_THRESHOLD * dU)
        {
            PQR_NEIGHBOUR pN    = &VList.aN[VList.nCount++];
            pN->nIndex          = nJ;
            pN->dDistance       = dU;
        }
    }

    if (!(HList.nCount && VList.nCount))
        return;

    QrTestNeighbours(pCtx, nI, &HList, &VList);
}

// Runs the full decode pipeline: threshold, finder scan, and capstone grouping.
VOID QrEnd(IN OUT PQR_CTX pCtx)
{
    BYTE bThreshold = QrOtsuThreshold(pCtx);
    QrPixelsSetup(pCtx, bThreshold);
    for (INT nI = 0; nI < pCtx->nHeight; nI++)
        QrFinderScan(pCtx, nI);
    for (INT nI = 0; nI < pCtx->nCapstones; nI++)
        QrTestGrouping(pCtx, nI);
}


// Samples a single cell through the grid's perspective and sets the corresponding bit.
static VOID QrReadCell(IN PQR_CTX pCtx, IN INT nIdx, IN INT nX, IN INT nY, IN OUT PQR_CODE pCode, IN OUT PINT pnBit)
{
    if (QrCellValue(pCtx, nIdx, nX, nY) > 0)
        pCode->abCells[*pnBit >> 3] |= (1 << (*pnBit & 7));
    (*pnBit)++;
}

// Extracts the raw bit grid for the QR code at nIndex via perspective sampling.
VOID QrExtract(IN PQR_CTX pCtx, IN INT nIndex, OUT PQR_CODE pCode)
{
    const PQR_GRID  pGrid   = &pCtx->aGrids[nIndex];
    INT             nBit    = 0x00;

    RtlZeroMemory(pCode, sizeof(*pCode));
    if (nIndex < 0 || nIndex >= pCtx->nGrids)
        return;

    QrPerspectiveMap(pGrid->adPerspective, 0.0,          0.0,          &pCode->aptCorners[0]);
    QrPerspectiveMap(pGrid->adPerspective, pGrid->nSize, 0.0,          &pCode->aptCorners[1]);
    QrPerspectiveMap(pGrid->adPerspective, pGrid->nSize, pGrid->nSize, &pCode->aptCorners[2]);
    QrPerspectiveMap(pGrid->adPerspective, 0.0,          pGrid->nSize, &pCode->aptCorners[3]);
    pCode->nSize = pGrid->nSize;

    if (pCode->nSize > QR_MAX_GRID_SIZE)
        return;

    for (INT nY = 0; nY < pGrid->nSize; nY++)
        for (INT nX = 0; nX < pGrid->nSize; nX++)
            QrReadCell(pCtx, nIndex, nX, nY, pCode, &nBit);
}

/*
// Mirrors the QR grid along the diagonal for retry after a failed decode.
VOID QrFlip(IN OUT PQR_CODE pCode)
{
    QR_CODE     Flipped     = { 0 };
    DWORD       dwOffset    = 0x00;

    for (INT nY = 0; nY < pCode->nSize; nY++)
        for (INT nX = 0; nX < pCode->nSize; nX++)
        {
            if (QrGridBit(pCode, nY, pCode->nSize - 1 - nX))
                Flipped.abCells[dwOffset >> 3u] |= (1u << (dwOffset & 7u));
            dwOffset++;
        }

    RtlCopyMemory(&pCode->abCells, &Flipped.abCells, sizeof(Flipped.abCells));
}
*/

// Decodes a QR_CODE grid into payload data: format, ECC, and data segment parsing.
BOOL QrDecode(IN PQR_CODE pCode, OUT PQR_DATA pData)
{
    QR_DATASTREAM Ds = { 0 };

    if (!pCode || !pData)
        return FALSE;
    if (pCode->nSize > QR_MAX_GRID_SIZE)
    {
        DBG("[!] Grid Size %d Exceeds Maximum (%d)", pCode->nSize, QR_MAX_GRID_SIZE);
        return FALSE;
    }
    if ((pCode->nSize - QR_BASE_SIZE) % QR_VER_INCREMENT)
    {
        DBG("[!] Grid Size %d Is Not QR-Aligned (Must Be %d + %d*N)", pCode->nSize, QR_BASE_SIZE, QR_VER_INCREMENT);
        return FALSE;
    }

    RtlZeroMemory(pData, sizeof(*pData));
    pData->nVersion = (pCode->nSize - QR_BASE_SIZE) / QR_VER_INCREMENT;

    if (pData->nVersion < QR_VERSION_MIN || pData->nVersion > QR_VERSION_MAX)
    {
        DBG("[!] Derived Version %d Out Of Range (%d-%d)", pData->nVersion, QR_VERSION_MIN, QR_VERSION_MAX);
        return FALSE;
    }

    if (!QrReadFormat(pCode, pData, 0))
    {
        if (!QrReadFormat(pCode, pData, 1))
        {
            DBG("[!] Both Format Info Copies Are Unreadable (Version: %d, GridSize: %d)", pData->nVersion, pCode->nSize);
            return FALSE;
        }
    }

    Ds.pbRaw = pData->abPayload;
    QrReadData(pCode, pData, &Ds);
    if (!QrCodestreamEcc(pData, &Ds))
        return FALSE;

    Ds.pbRaw = NULL;
    return QrDecodePayload(pData, &Ds);
}

#pragma endregion
