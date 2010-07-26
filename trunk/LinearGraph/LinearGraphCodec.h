#pragma once

struct CLinearGraphCodec
{
    // Determine the encoder by file name extension
    // 
    static bool GetEncoderClsid(const WCHAR* fileNameExtension, CLSID* pClsid);

    static bool SaveImage(Gdiplus::Bitmap* pBitmap, const WCHAR* fileName);
};