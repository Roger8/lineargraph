#include "LinearGraphDoc.h"

CDataObject::CDataObject(LONG* pData, LONG len): data(pData), length(len)
{
    factor = 1;
    refCount  = 1;
    timeStamp = 0;
    freqDeno  = freqNume = 0;
    index = 0;
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
        m_pObj->index = -1;
        m_pObj->timeStamp = 0;
        m_pObj->freqDeno = m_pObj->freqNume = 1;
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

BOOL CTextFile::Open(PCWSTR fileName)
{
    if( m_pData || !fileName )
    {
        return FALSE;
    }

    HANDLE hFile = ::CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        return FALSE;
    }

    DWORD dwSize;
    m_cbSize = ::GetFileSize(hFile, &dwSize);
    if( dwSize || m_cbSize > 0x40000000 )
    {
        ::CloseHandle(hFile);
        ::SetLastError(ERROR_FILE_TOO_LARGE);
        return FALSE;
    }

    char* pData = new char[m_cbSize+2];
    pData[m_cbSize] = '\n';
    pData[m_cbSize+1] = 0;
    ::ReadFile(hFile, pData, m_cbSize, &dwSize, 0);
    if( dwSize != m_cbSize )
    {
        delete[] pData;
        pData = 0;
        m_cbSize = 0;
    }
    return (m_pData = pData) != 0;
}

BOOL CTextFile::Close()
{
    delete[] m_pData;
    m_pData  = 0;
    m_cbSize = 0;
    return TRUE;
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
    if( m_vSamples.size() )
    {
        return FALSE;
    }

    CTextFile textFile;
    if( !textFile.Open(szFileName) )
    {
        LG_ERROR("CTextFile::Open Failed [Win32 Error = %d]", ::GetLastError());
        return FALSE;
    }

    DWORD dataType;
    dataType = CheckDataType(textFile.GetData(), textFile.GetSize());
    if( dataType != IntegerData )
    {
        ::SetLastError(ERROR_UNSUPPORTED_TYPE);
        LG_ERROR("CTextFile::Open Failed [Unsupported Data Type]");
        return FALSE;
    }

    DWORD nLines, nColumns, nWords;
    nLines   = CountLine(textFile.GetData(), textFile.GetSize());
    nColumns = CountColumn(textFile.GetData(), textFile.GetSize());
    nWords   = CountWord(textFile.GetData(), textFile.GetSize());
    LG_TRACE("File Statistics: %s type, %d lines, %d columns, %d words",
        (dataType==IntegerData) ? "Integer":"Float", nLines, nColumns, nWords);

    if( nWords != nLines * nColumns )
    {
        LG_ERROR("CTextSampleFile::Open Failed [Corrupted Data]");
        return FALSE;
    }

    BOOL fTimeStamp = is_timestamp(textFile.GetData());
    if( fTimeStamp )
    {
        --nColumns;
    }

    for(DWORD i = 0; i < nColumns; ++i)
    {
        CDataObjectPtr pObj;
        if( !(pObj = CDataObject::CreateObject(nLines)) )
        {
            return FALSE;
        }

        m_vSamples.push_back(pObj);

        pObj->index = i;
        pObj->file  = szFileName;
        GetObjectName(pObj->file, pObj->name);

        if( nColumns > 1 )
        {
            pObj->name.AppendFormat(L"[%d]", i);
        }
    }

    if( fTimeStamp )
    {
        ReadTimeStamp(textFile.GetData(), textFile.GetSize());
    }

    //  Convert text data to binary data
    //
    switch( dataType )
    {
    case IntegerData:
        LG_TRACE_PERFORMANCE(ReadInt(textFile.GetData(), nLines, nColumns));
        break;
    case FloatData:
        LG_TRACE_PERFORMANCE(ReadFloat(textFile.GetData(), nLines, nColumns));
        break;
    }
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

void CTextSampleFile::ReadTimeStamp(PCSTR pData, DWORD cbSize)
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

    if( !tEnd ){ tEnd = tBegin; }

    AsciiTimeTrim(tBegin);
    AsciiTimeTrim(tEnd);
    AsciiTimeToFileTime(tBegin);
    AsciiTimeToFileTime(tEnd);

    for(size_t i = 0; i < m_vSamples.size(); ++i)
    {
        m_vSamples[i]->timeStamp = tBegin;
        m_vSamples[i]->freqDeno  = (DWORD)((tEnd - tBegin) / 10000);
        m_vSamples[i]->freqNume = (DWORD)fmulti * 1000;
        m_vSamples[i]->TrimFrequency();
    }
}

void CTextSampleFile::AsciiTimeTrim(LONGLONG& asciit)
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

void CTextSampleFile::AsciiTimeToFileTime(LONGLONG& t)
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

DWORD __fastcall CTextSampleFile::CheckDataType(PCSTR pData, DWORD cbSize)
{
    BOOL bFloat = FALSE;
    for(DWORD i = 0; i < cbSize; ++i)
    {
        if( '.' == pData[i] )
        {
            bFloat = TRUE;
        }
        else if( !is_legal_char(pData[i]) )
        {
            return UnsupportedDataType;
        }
    }

    // Float data is not supported yet
    //
    //return bFloat ? FloatData : IntegerData;
    return IntegerData;
}

DWORD CTextSampleFile::CountLine(PCSTR pData, DWORD cbSize)
{
    DWORD nLines = 0;
    for(DWORD i = 0; i < cbSize; )
    {
        if( '\n' == pData[i++] )
        {
            // Skip blank lines
            while( (i<cbSize) && is_whitespace(pData[i]) )
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

DWORD CTextSampleFile::CountColumn(PCSTR pData, DWORD cbSize)
{
    DWORD nColumns = 0;
    for(DWORD i = 0; i < cbSize; ++i)
    {
        if( '\n' == pData[i] )
        {
            nColumns = CountWord(pData, i);
            break;
        }
    }
    return nColumns;
}

DWORD CTextSampleFile::CountWord(PCSTR pData, DWORD cbSize)
{
    DWORD wordCount = 0;
    DWORD dfaState  = 0;

    for(DWORD i = 0; i < cbSize; ++i)
    {
        if( is_whitespace(pData[i]) )
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

void CTextSampleFile::ReadInt(const char* gp, DWORD cLn, DWORD cCol)
{
    // Extract the raw pointers for faster access
    LONG** apData = new LONG*[cCol];
    for(DWORD i = 0; i < cCol; ++i)
    {
        apData[i] = m_vSamples[i]->data;
    }

    if( is_timestamp(gp) )
    {
        for(DWORD i = 0; i < cLn; ++i)
        {
            eat_word(gp); // Skip time stamp
            for (DWORD j = 0; j < cCol; ++j)
            {
                apData[j][i] = atoi(gp);
                eat_word(gp);
            }
        }
    }
    else
    {
        for(DWORD i = 0; i < cLn; ++i)
        {
            for (DWORD j = 0; j < cCol; ++j)
            {
                apData[j][i] = atoi(gp);
                eat_word(gp);
            }
        }
    }

    delete[] apData;
}

void CTextSampleFile::ReadFloat(const char* gp, DWORD cLn, DWORD cCol)
{
    // Extract the raw pointers for faster access
    LONG** apData = new LONG*[cCol];
    for(DWORD i = 0; i < cCol; ++i)
    {
        m_vSamples[i]->factor = 1000;
        apData[i] = m_vSamples[i]->data;
    }

    char* p;
    if( is_timestamp(gp) )
    {
        for(DWORD i = 0; i < cLn; ++i)
        {
            eat_word(gp); // Skip time stamp
            for (DWORD j = 0; j < cCol; ++j)
            {
                apData[j][i] = (LONG)(strtod(gp, &p) * 1000);
                gp = p;
            }
        }
    }
    else
    {
        for(DWORD i = 0; i < cLn; ++i)
        {
            for (DWORD j = 0; j < cCol; ++j)
            {
                apData[j][i] = (LONG)(strtod(gp, &p) * 1000);
                gp = p;
            }
        }
    }

    delete[] apData;
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
            LG_TRACE("Data type: Integer");
            ::ReadFile(hFile, pData->data, cbDataLength, &dwBytes, 0);
        }
        break;
    case 'f':
    case 'F':
        {
            LG_TRACE("Data type: Float");

            BYTE* pBin = new BYTE[cbDataLength];
            ::ReadFile(hFile, pBin, cbDataLength, &dwBytes, 0);

            DOUBLE* fd = (DOUBLE*)pBin;
            for(DWORD i = 0; i < nLength; ++i, ++fd)
            {
                pData->data[i] = (LONG)((*fd)*1000+0.5);
            }
            pData->factor = 1000;
            delete[] pBin;
        }
        break;
    }
    ::CloseHandle(hFile);

    pData->freqDeno  = hs.freqBase;
    pData->freqNume = hs.freqMulti;
    pData->timeStamp = *(LONGLONG*)&hs.timeStamp;
    pData->file = szFileName;
    pData->index = 0;
    GetObjectName(pData->file, pData->name);
    pData->TrimFrequency();

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
        pObj->freqNume = pcurt->dataFreq.numerator;
        pObj->freqDeno = pcurt->dataFreq.denominator;
        pObj->index = i;
        pObj->file = szFileName;
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
        m_pFile = OpenTextFile(szFileName);
    }
    else if( (len >= 5 && !_wcsnicmp(szFileName+len-5, L".SEED", 5))
        || (len >= 6 && !_wcsnicmp(szFileName+len-6, L".MSEED", 6)) )
    {
        m_pFile = OpenSeedFile(szFileName);
    }
    else if( len >= 5 && !_wcsnicmp(szFileName+len-5, L".EDAS", 5) )
    {
        m_pFile = OpenEdasFile(szFileName);
    }
    else if( (len >= 4 && !_wcsnicmp(szFileName+len-4, L".BIN", 4))
        || (len >= 5 && !_wcsnicmp(szFileName+len-5, L".BINX", 5)) )
    {
        m_pFile = OpenBinFile(szFileName);
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

ISampleFile* CLinearGraphDoc::OpenTextFile( PCWSTR szFileName )
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

ISampleFile* CLinearGraphDoc::OpenSeedFile( PCWSTR szFileName )
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

ISampleFile* CLinearGraphDoc::OpenEdasFile( PCWSTR szFileName )
{
    LG_TRACE("Open %ws as EDAS file", szFileName);
    LG_ERROR("EDAS document is not supported");
    return 0;
}

ISampleFile* CLinearGraphDoc::OpenBinFile( PCWSTR szFileName )
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
