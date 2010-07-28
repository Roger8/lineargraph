#include "LinearGraphDoc.h"

CDataObject::CDataObject(LONG* pData, LONG len): data(pData), length(len)
{
    refCount  = 1;
    timeStamp = 0;
    freqBase  = freqMulti = 0;
    indexInDoc = 0;
}

CDataObject::~CDataObject()
{
    delete[] data;
}

CDataObjectPtr CDataObject::CreateObject(LONG nLen)
{
    LONG* pData = 0;
    if( nLen > 0 )
    {
        pData = new LONG[nLen];
    }
    if( !pData )
    {
        LG_TRACE("CDataObject::CreateObject Failed [length = %d]", nLen);
        return CDataObjectPtr();
    }
    return CDataObjectPtr(new CDataObject(pData, nLen));
}

CDataObjectPtr::CDataObjectPtr() : m_pObj(0)
{
}

CDataObjectPtr::CDataObjectPtr(CDataObject* pImpl) : m_pObj(pImpl)
{
    if( m_pObj )
    {
        m_pObj->refCount = 1;
        m_pObj->indexInDoc = -1;
        m_pObj->timeStamp = 0;
        m_pObj->freqBase = m_pObj->freqMulti = 1;
    }
}

CDataObjectPtr::CDataObjectPtr(const CDataObjectPtr& data) : m_pObj(data.m_pObj)
{
    if( m_pObj )
    {
        ::InterlockedIncrement(&(m_pObj->refCount));
    }
}

CDataObjectPtr::~CDataObjectPtr()
{
    release();
}

const CDataObjectPtr& CDataObjectPtr::operator=(const CDataObjectPtr& r)
{
    release();
    m_pObj = r.m_pObj;

    if( m_pObj )
    {
        ::InterlockedIncrement(&(m_pObj->refCount));
    }
    return *this;
}

void CDataObjectPtr::release()
{
    if( m_pObj && !::InterlockedDecrement(&(m_pObj->refCount)) )
    {
        delete m_pObj;
    }
    m_pObj = 0;
}

CTextSampleFile::CTextSampleFile()
{
}

CTextSampleFile::~CTextSampleFile()
{
    Close();
}

BOOL CTextSampleFile::Open(PCWSTR szFileName)
{
    DWORD fileSize  = 0;
    char*  pFileData = loadFile(szFileName, fileSize);
    if( !pFileData )
    {
        LG_TRACE("CTextSampleFile::Open::loadFile Failed [pFileData = 0]");
        return FALSE;
    }

    m_vSamples.clear();

    DWORD nLines   = countLine(pFileData, fileSize);
    DWORD nColumns = countColumn(pFileData, fileSize);
    DWORD nWords   = countWord(pFileData, fileSize);

    if( nLines >= 0x10000000 || nColumns >= 0x10000000 || !nLines || !nColumns )
    {
        LG_ERROR("CTextSampleFile::Open Failed [nLines = %d, nColumns = %d]",
            nLines, nColumns);
        unloadFile(pFileData);
        return FALSE;
    }

    if( nWords != nLines*nColumns && nWords != nLines*(nColumns+1) )
    {
        LG_ERROR("CTextSampleFile::Open Failed [data corrupted]");
        unloadFile(pFileData);
        return FALSE;
    }

    CDataObjectPtr pObj;
    for(size_t i = 0; i < nColumns; ++i)
    {
        pObj = CDataObject::CreateObject(nLines);
        if( !pObj )
        {
            return FALSE;
        }

        pObj->indexInDoc = i;
        pObj->ownerFile = szFileName;
        
        GetName(pObj->ownerFile, pObj->name);
        if( nColumns > 1 )
        {
            pObj->name.AppendFormat(L"[%d]", i);
        }
        m_vSamples.push_back(pObj);
    }

    //  Convert text data to binary data
    //
    const char* gp = pFileData;
    if( isFirstColumnTimeStamp(pFileData) )
    {
        for(size_t i = 0; i < nLines; ++i)
        {
            readWord(gp); // Skip time stamp

            for (size_t j = 0; j < nColumns; ++j)
            {
                m_vSamples[j]->data[i] = atoi(readWord(gp));
            }
        }
        getTimeFreq(pFileData, fileSize);
    }
    else
    {
        for(size_t i = 0; i < nLines; ++i)
        {
            for (size_t j = 0; j < nColumns; ++j)
            {
                m_vSamples[j]->data[i] = atoi(readWord(gp));
            }
        }
    }

    unloadFile(pFileData);
    return TRUE;
}

BOOL CTextSampleFile::Close()
{
    m_vSamples.clear();
    return TRUE;
}

LONG CTextSampleFile::GetSampleCount()
{
    return (LONG)m_vSamples.size();
}

CDataObjectPtr& CTextSampleFile::GetSample(LONG i)
{
    return m_vSamples[i];
}

void CTextSampleFile::getTimeFreq(const char* pData, DWORD cbSize)
{
    LONGLONG tBegin = _atoi64(pData);
    LONGLONG tEnd = tBegin;
    
    DWORD fbase = 0, fmulti = 0;
    for(DWORD i = 1, im = cbSize - 1; i < im; )
    {
        if( '\n' == pData[i++] )
        {
            ++fmulti;
            if( (tEnd = _atoi64(pData+i)) != tBegin )
            {
                break;
            }
        }
    }

    if( !tEnd )
    {
        tEnd = tBegin;
    }

    asciiTimeTrim(tBegin);
    asciiTimeTrim(tEnd);
    asciiTimeToFileTime(tBegin);
    asciiTimeToFileTime(tEnd);

    for(size_t i = 0; i < m_vSamples.size(); ++i)
    {
        m_vSamples[i]->timeStamp = tBegin;
        m_vSamples[i]->freqBase  = (DWORD)((tEnd - tBegin) / 10000);
        m_vSamples[i]->freqMulti = (DWORD)fmulti * 1000;
        m_vSamples[i]->trimFrequency();
    }
}

void CTextSampleFile::asciiTimeTrim(LONGLONG& asciit)
{
    // If asciit lacks precision, just append zeros
    //
    if( asciit < 19001010120000000 )
    {
        if( asciit > 19001010120000 )     // Accurate to second
        {
            asciit *= 1000;
        }
        else if( asciit > 190010101200 )  // Accurate to minute
        {
            asciit *= 100000;
        }
        else if( asciit > 1900101012 )    // Accurate to hour
        {
            asciit *= 10000000;
        }
    }
}

void CTextSampleFile::asciiTimeToFileTime(LONGLONG& t)
{
    SYSTEMTIME syst = {0};
    syst.wYear   = (WORD)(t / 10000000000000);
    syst.wMonth  = (WORD)((t % 10000000000000) / 100000000000);
    syst.wDay    = (WORD)((t % 100000000000) / 1000000000);
    syst.wHour   = (WORD)((t % 1000000000)/ 10000000);
    syst.wMinute = (WORD)((t % 10000000) / 100000);
    syst.wSecond = (WORD)((t % 100000) / 1000);
    syst.wMilliseconds = (WORD)(t % 1000);

    SystemTimeToFileTime(&syst, (FILETIME*)&t);
}

bool CTextSampleFile::isFirstColumnTimeStamp(const char* pData)
{
    // Compare with 1900010100, which is the minimum value accepted as a decimal
    //string represented timestamp
    //
    return _atoi64(pData) > 0x0713FDA74;
}

DWORD CTextSampleFile::countLine(const char* pData, DWORD cbSize)
{
    DWORD nLines = 0;
    for(DWORD i = 0; i < cbSize; )
    {
        if( !isLegalCharacter(pData[i]) )
        {
            LG_ERROR("Illegal character encountered at position [%d]", i);
            return -1;
        }
        else if( '\n' == pData[i++] )
        {
            while( (i<cbSize) && isWhitespace(pData[i]) )
            {
                ++i;
            }
            ++nLines;
        }
    }
    if( '\n' != pData[cbSize-1] )
    {
        ++nLines;
    }
    return nLines;
}

DWORD CTextSampleFile::countColumn(const char* pData, DWORD cbSize)
{
    DWORD nColumns = 0;
    for(DWORD i = 0; i < cbSize; ++i)
    {
        if( '\n' == pData[i] )
        {
            nColumns = countWord(pData, i);
            break;
        }
    }

    if( !nColumns ){ return nColumns; }

    if( isFirstColumnTimeStamp(pData) )
    {
        --nColumns;
    }
    return nColumns;
}

DWORD CTextSampleFile::countWord(const char* pData, DWORD cbSize)
{
    DWORD wordCount = 0;
    DWORD dfaState  = 0;

    for(DWORD i = 0; i < cbSize; ++i)
    {
        if( isWhitespace(pData[i]) )
        {
            dfaState = 0;
        }
        else if( dfaState == 0 )
        {
            dfaState = 1;
            ++wordCount;
        }
    }

    return wordCount;
}

char* CTextSampleFile::loadFile(PCWSTR fileName, DWORD& fileSize)
{
    HANDLE hFile;
    hFile = ::CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        LG_ERROR("CTextSampleFile::loadFile::CreateFile Failed. [Error = 0x%08H]",
            ::GetLastError());
        return 0;
    }
    fileSize = ::GetFileSize(hFile, 0);

    char* pFileData = 0;
    if( fileSize && (pFileData = new char[fileSize + 2]) )
    {
        DWORD dwRead = 0;
        ::ReadFile(hFile, pFileData, fileSize, &dwRead, 0);

        if( dwRead != fileSize )
        {
            delete[] pFileData;
            pFileData = 0;
        }
        else
        {
            //
            //  Make sure that the text is ended with a whitespace
            //  readWord function may cause access violation if the ending character
            // is not a whitespace
            //
            pFileData[fileSize] = '\n';
            pFileData[fileSize + 1] = 0;
        }
    }
    ::CloseHandle(hFile);
    return pFileData;
}

void CTextSampleFile::unloadFile(char* pFileData)
{
    delete[] pFileData;
}

BOOL CBinSampleFile::Open(PCWSTR szFileName)
{
    HANDLE hFile = ::CreateFileW(szFileName, GENERIC_READ, FILE_SHARE_READ,
        0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        LG_ERROR("CBinSampleFile::Open::CreateFile Failed [Win32 Error = 0x%08H]",
            ::GetLastError());
        return FALSE;
    }

    DWORD dwFileSize = ::GetFileSize(hFile, 0);
    if( dwFileSize <= sizeof(HeaderStruct) )
    {
        LG_ERROR("CBinSampleFile::Open Failed [File too small, size = %d]",
            dwFileSize);
        ::CloseHandle(hFile);
        return FALSE;
    }

    DWORD   dwBytes = 0;
    HeaderStruct hs = {0};
    ::ReadFile(hFile, &hs, sizeof(hs), &dwBytes, 0);

    DWORD dwUnitSize   = ::atoi(hs.dataType+1) / 8;
    DWORD cbDataLength = dwFileSize - sizeof(HeaderStruct);
    DWORD nLength      = cbDataLength / dwUnitSize;

    CDataObjectPtr pData;
    pData = CDataObject::CreateObject((LONG)nLength);
    if( !pData )
    {
        return FALSE;
    }

    switch( hs.dataType[0] )
    {
    case 'i':
    case 'I':
        {
            ::ReadFile(hFile, pData->data, cbDataLength, &dwBytes, 0);
        }
        break;
    case 'f':
    case 'F':
        {
            BYTE* pBin = new BYTE[cbDataLength];
            ::ReadFile(hFile, pBin, cbDataLength, &dwBytes, 0);

            DOUBLE* fd = (DOUBLE*)pBin;
            for(DWORD i = 0; i < nLength; ++i, ++fd)
            {
                pData->data[i] = (LONG)(*fd+0.5);
            }

            delete[] pBin;
        }
        break;
    }
    ::CloseHandle(hFile);

    pData->freqBase  = hs.freqBase;
    pData->freqMulti = hs.freqMulti;
    pData->timeStamp = *(LONGLONG*)&hs.timeStamp;
    pData->ownerFile = szFileName;
    pData->indexInDoc = 0;
    GetName(pData->ownerFile, pData->name);
    pData->trimFrequency();

    m_pData = pData;
    return TRUE;
}

BOOL CBinSampleFile::Close()
{
    m_pData.release();
    return TRUE;
}

LONG CBinSampleFile::GetSampleCount()
{
    return m_pData ? 1 : 0;
}

CDataObjectPtr& CBinSampleFile::GetSample( LONG i )
{
    return m_pData;
}

BOOL CSeedSampleFile::Open(PCWSTR szFileName)
{
    CSeedFile seed;
    if( !seed.open(szFileName, NULL) )
    {
        LG_ERROR("CSeedSampleFile::Open Failed [Win32 Error = 0x%08H]",
            ::GetLastError());
        return FALSE;
    }

    int nColumns= seed.GetStationCount();

    for(int i = 0; i < nColumns; ++i)
    {
        if( !seed.DcmpExtrStationDataByTime(i, NULL, NULL, 0, 0) )
        {
            continue;
        }
        
        Station* pcurt = seed.GetStationInfo(i);
        CDataObjectPtr pObj = CDataObject::CreateObject(pcurt->iDataCnt);
        if( !pObj )
        {
            return FALSE;
        }

        memcpy(pObj->data, pcurt->pData, pcurt->iDataCnt * sizeof(int));

        ::SystemTimeToFileTime(&pcurt->datatime, (FILETIME*)&pObj->timeStamp);
        pObj->freqMulti = pcurt->dataFreq.numerator;
        pObj->freqBase = pcurt->dataFreq.denominator;
        pObj->indexInDoc = i;
        pObj->ownerFile = szFileName;
        pObj->name = pcurt->station;
        pObj->name.AppendChar(L'.');
        pObj->name.Append(pcurt->channel);
        
        m_vSamples.push_back(pObj);
    }
    return m_vSamples.size() != 0;
}

BOOL CSeedSampleFile::Close()
{
    m_vSamples.clear();
    return TRUE;
}

LONG CSeedSampleFile::GetSampleCount()
{
    return m_vSamples.size();
}

CDataObjectPtr& CSeedSampleFile::GetSample( LONG i )
{
    return m_vSamples[i];
}


CLinearGraphDoc::CLinearGraphDoc() : m_pFile(0)
{

}

CLinearGraphDoc::~CLinearGraphDoc()
{
    Close();
}

BOOL CLinearGraphDoc::Open(PCWSTR szFileName)
{
    if( m_pFile )
    {
        return FALSE;
    }

    size_t len = wcslen(szFileName);
    if( len >= 4 && !_wcsnicmp(szFileName+len-4, L".TXT", 4) )
    {
        m_pFile = openTextFile(szFileName);
    }
    else if( (len >= 5 && !_wcsnicmp(szFileName+len-5, L".SEED", 5))
        || (len >= 6 && !_wcsnicmp(szFileName+len-6, L".MSEED", 6)) )
    {
        m_pFile = openSeedFile(szFileName);
    }
    else if( len >= 5 && !_wcsnicmp(szFileName+len-5, L".EDAS", 5) )
    {
        m_pFile = openEdasFile(szFileName);
    }
    else if( (len >= 4 && !_wcsnicmp(szFileName+len-4, L".BIN", 4))
        || (len >= 5 && !_wcsnicmp(szFileName+len-5, L".BINX", 5)) )
    {
        m_pFile = openBinFile(szFileName);
    }
    else
    {
        LG_ERROR("Unknown document type: %ws", szFileName);
    }
    return m_pFile != 0;
}

BOOL CLinearGraphDoc::Close()
{
    if( m_pFile )
    {
        m_pFile->Close();
        delete m_pFile;
        m_pFile = 0;
    }
    return TRUE;
}

ISampleFile* CLinearGraphDoc::openTextFile( PCWSTR szFileName )
{
    LG_TRACE("Open %ws as TEXT file", szFileName);
    if( CTextSampleFile* pFile = new CTextSampleFile )
    {
        if( pFile->Open(szFileName) )
        {
            return static_cast<ISampleFile*>(pFile);
        }
        delete pFile;
    }
    return 0;
}

ISampleFile* CLinearGraphDoc::openSeedFile( PCWSTR szFileName )
{
    LG_TRACE("Open %ws as SEED file", szFileName);
    if( CSeedSampleFile * pSeed = new CSeedSampleFile )
    {
        if( pSeed->Open(szFileName) )
        {
            return static_cast<ISampleFile*>(pSeed);
        }
        delete pSeed;
    }
    return 0;
}

ISampleFile* CLinearGraphDoc::openEdasFile( PCWSTR szFileName )
{
    LG_TRACE("Open %ws as EDAS file", szFileName);
    LG_ERROR("EDAS document is not supported");
    return 0;
}

ISampleFile* CLinearGraphDoc::openBinFile( PCWSTR szFileName )
{
    LG_TRACE("Open %ws as BIN file", szFileName);
    if( CBinSampleFile* pFile = new CBinSampleFile )
    {
        if( pFile->Open(szFileName) )
        {
            return static_cast<ISampleFile*>(pFile);
        }
        delete pFile;
    }
    return 0;
}
