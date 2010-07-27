#pragma once
#include "LinearGraph.h"

#include <vector>

class CDataObject;

class CDataObjectPtr
{
public:
    CDataObjectPtr();
    CDataObjectPtr(const CDataObjectPtr& r);
    ~CDataObjectPtr();

    const CDataObjectPtr& operator =(const CDataObjectPtr& r);

    CDataObject* operator ->()
    {
        return m_pObj;
    }

    const CDataObject* operator ->() const
    {
        return m_pObj;
    }

    CDataObject& operator *()
    {
        return *m_pObj;
    }

    const CDataObject& operator *() const
    {
        return *m_pObj;
    }

    bool operator == (const CDataObjectPtr& r) const
    {
        return m_pObj == r.m_pObj;
    }

    bool operator != (const CDataObjectPtr& r) const
    {
        return m_pObj != r.m_pObj;
    }

    operator bool() const
    {
        return m_pObj != 0;
    }

    void release();

protected:
    friend class CDataObject;
    explicit CDataObjectPtr(CDataObject* pObj);

private:
    CDataObject*    m_pObj;
};

class CDataObject
{
private:
    // Do not construct or destruct CDataObject object explicitly
    // Use CreateObject method instead.
    //
    CDataObject(LONG* pData, LONG len);
    CDataObject(const CDataObject& obj){}
    ~CDataObject();

public:
    LONG*       data;           // Sample data
    LONG        length;         // Number of points in this sample
    ULONGLONG   timeStamp;      // Actually a FILETIME struct
    DWORD       freqMulti;      // Frequency, numerator
    DWORD       freqBase;       // Frequency, denominator
    size_t      indexInDoc;     // Index of this sample in file
    CString     name;           // Name of the sample
    CString     ownerFile;      // Name of the sample file

protected:
    friend class CDataObjectPtr;
    LONG volatile   refCount;   // Reference count

public:
    static CDataObjectPtr CreateObject(LONG nLen);

    BOOL hasTimestamp() const
    {
        return timeStamp && freqBase && freqMulti;
    }

    DOUBLE getFrequency() const
    {
        return (DOUBLE)freqMulti / freqBase;
    }

    BOOL getDateTime(SYSTEMTIME& st) const
    {
        return ::FileTimeToSystemTime((FILETIME*)&timeStamp, &st);
    }

    ULONGLONG getTimeSpan() const
    {
        return length * (ULONGLONG)freqBase / freqMulti;
    }

    BOOL isCompatibleWith(const CDataObjectPtr& r) const
    {
        return length == r->length && timeStamp == r->timeStamp
            && freqMulti == r->freqMulti && freqBase == r->freqBase;
    }

    void trimFrequency()
    {
        if( !freqBase || !freqMulti )
        {
            return ;
        }

        DWORD a = max(freqMulti, freqBase);
        DWORD b = min(freqMulti, freqBase);

        for(DWORD rem = a%b; rem; rem = a%b)
        {
            a = b;
            b = rem;
        }

        freqMulti /= b;
        freqBase /= b;
    }
};

typedef std::vector<CDataObjectPtr>    CDataObjectPtrVect;

struct ISampleFile
{
    virtual ~ISampleFile(){}

    virtual BOOL Open(PCWSTR szFileName) = 0;
    virtual BOOL Close() = 0;
    virtual LONG GetSampleCount() = 0;
    virtual CDataObjectPtr& GetSample(LONG i) = 0;

    BOOL GetName(const CString& pathName, CString& fileName) const
    {
        int ipos = pathName.ReverseFind(L'\\');
        if( ipos == -1 )
        {
            fileName = pathName;
            return FALSE;
        }

        fileName = 1 + ipos + (PCWSTR)pathName;
        fileName.Delete(fileName.ReverseFind(L'.'), 4);
        return TRUE;
    }
};

class CTextSampleFile: public ISampleFile
{
public:
    CTextSampleFile();
    ~CTextSampleFile();

    BOOL Open(PCWSTR szFileName);
    BOOL Close();
    LONG GetSampleCount();
    CDataObjectPtr& GetSample(LONG i);

protected:
    CDataObjectPtrVect      m_vSamples;

protected:
    char*  loadFile(PCWSTR fileName, DWORD& fileSize);
    void   unloadFile(char* pFileData);
    void   getTimeFreq(const char* pData, DWORD cbSize);
    void   asciiTimeTrim(LONGLONG& asciit);
    void   asciiTimeToFileTime(LONGLONG& asciit);

    static bool isFirstColumnTimeStamp(const char* pData);
    static DWORD __fastcall countLine(const char* pData, DWORD cbSize);
    static DWORD __fastcall countColumn(const char* pData, DWORD cbSize);
    static DWORD __fastcall countWord(const char* pData, DWORD cbSize);

    static const char* readWord(const char*& gp)
    {
        while( isWhitespace(*gp) ){ ++gp; }
        const char* word = gp;
        while( !isWhitespace(*gp) ){ ++gp; }
        return word;
    }

    inline static int isWhitespace(char ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    inline static int isLegalCharacter(char ch)
    {
        return isWhitespace(ch) || (ch >= '0' && ch <= '9')
            || ch == '+' || ch == '-' || ch == '.';
    }
};

class CBinSampleFile : public ISampleFile
{
public:
    struct HeaderStruct
    {
        CHAR stationName[32];
        CHAR name[8];
        DWORD freqMulti;
        DWORD freqBase;
        FILETIME timeStamp;
        CHAR dataType[8];
        BYTE reserved[960];
    };

    BOOL Open(PCWSTR szFileName);
    BOOL Close();
    LONG GetSampleCount();
    CDataObjectPtr& GetSample(LONG i);

private:
    CDataObjectPtr  m_pData;
};

class CSeedSampleFile : public ISampleFile
{
public:
    BOOL Open(PCWSTR szFileName);
    BOOL Close();
    LONG GetSampleCount();
    CDataObjectPtr& GetSample(LONG i);

private:
    CDataObjectPtrVect      m_vSamples;
};

class CLinearGraphDoc : public ISampleFile
{
public:
     CLinearGraphDoc();
    ~CLinearGraphDoc();

    BOOL Open(PCWSTR szFileName);
    BOOL Close();
    
    LONG GetSampleCount()
    {
        return m_pFile->GetSampleCount();
    }

    CDataObjectPtr& GetSample(LONG i)
    {
        return m_pFile->GetSample(i);
    }

private:
    ISampleFile* openTextFile(PCWSTR szFileName);
    ISampleFile* openSeedFile(PCWSTR szFileName);
    ISampleFile* openEdasFile(PCWSTR szFileName);
    ISampleFile* openBinFile(PCWSTR szFileName);

private:
    ISampleFile*    m_pFile;
};