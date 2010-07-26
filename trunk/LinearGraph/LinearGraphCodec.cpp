#include "LinearGraph.h"
#include "LinearGraphCodec.h"
using namespace Gdiplus;

bool CLinearGraphCodec::GetEncoderClsid(const WCHAR* extName, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if( !size || !num )
    {
        return false;
    }

    char* buff = new char[size];
    pImageCodecInfo = (ImageCodecInfo*)buff;
    if( !pImageCodecInfo )
    {
        return false;
    }

    GetImageEncoders(num, size, pImageCodecInfo);
    
    CString ext(extName);
    ext.MakeLower();
    
    for(size_t i = 0; i < num; ++i)
    {
        CString extSupported(pImageCodecInfo[i].FilenameExtension);
        extSupported.MakeLower();

        if( -1 != extSupported.Find(ext) )
        {
            *pClsid = pImageCodecInfo[i].Clsid;
            delete[] buff;
            return true;
        }    
    }

    delete[] buff;
    return false;
}

bool CLinearGraphCodec::SaveImage(Bitmap* pBitmap, const WCHAR* fileName)
{
    if( !pBitmap || !fileName )
    {
        return false;
    }

    const WCHAR* pExt = 0;
    size_t len = wcslen(fileName);
    for(size_t i = len-1; i > 0; --i)
    {
        if( fileName[i] == L'.' )
        {
            pExt = fileName + i;
            break;
        }
    }

    if( !pExt )
    {
        return false;
    }
    
    CLSID clsid;
    if( !GetEncoderClsid(pExt, &clsid) )
    {
        return false;
    }
    return Gdiplus::Ok == pBitmap->Save(fileName, &clsid);
}
