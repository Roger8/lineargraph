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
    LONG        length;         // Number of points in this sample
    LONG*       data;           // Scaled sample value
    LONG        factor;         // data = factor * original_data
    ULONGLONG   timeStamp;      // Actually a FILETIME struct
    DWORD       freqNume;       // Frequency, numerator
    DWORD       freqDeno;       // Frequency, denominator
    DWORD       index;          // Index of this sample in file
    CString     name;           // Name of this object
    CString     file;           // Name of the file

protected:
    friend class CDataObjectPtr;
    LONG volatile   refCount;   // Reference count

public:
    static CDataObjectPtr CreateObject(LONG nLen);

    BOOL HasTimeStamp() const
    {
        return timeStamp && freqDeno && freqNume;
    }

    DOUBLE GetSamplingRate() const
    {
        return (DOUBLE)freqNume / freqDeno;
    }
    
    ULONGLONG TimeSpan() const
    {
        return length * (ULONGLONG)freqDeno / freqNume;
    }

    BOOL IsCompatibleWith(const CDataObjectPtr& r) const
    {
        return length == r->length && timeStamp == r->timeStamp
            && freqNume == r->freqNume && freqDeno == r->freqDeno;
    }

    void TrimFrequency()
    {
        if( !freqDeno || !freqNume )
        {
            return ;
        }

        DWORD a = max(freqNume, freqDeno);
        DWORD b = min(freqNume, freqDeno);

        for(DWORD rem = a%b; rem; rem = a%b)
        {
            a = b;
            b = rem;
        }

        freqNume /= b;
        freqDeno /= b;
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

    BOOL GetObjectName(const CString& pathName, CString& fileName) const
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

// Read only text files
//
class CTextFile
{
public:
    CTextFile(): m_pData(0), m_cbSize(0)
    {
    }

    ~CTextFile(){ Close(); }

    BOOL Open(PCWSTR fileName);
    BOOL Close();

    DWORD GetSize() const { return m_cbSize; }
    PCSTR GetData() const { return m_pData; }

private:
    PCSTR       m_pData;
    DWORD       m_cbSize;
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

    enum DataType
    {
        UnsupportedDataType = 0,
        IntegerData,
        FloatData
    };

protected:
    CDataObjectPtrVect      m_vSamples;

private:
    void  ReadTimeStamp(const char* gp, DWORD cbSize);
    void  ReadInt(const char* gp, DWORD cln, DWORD ccol);
    void  ReadFloat(const char* gp, DWORD cln, DWORD ccol);
    void  AsciiTimeTrim(LONGLONG& asciit);
    void  AsciiTimeToFileTime(LONGLONG& asciit);

public:
    static DWORD __fastcall CheckDataType(PCSTR pData, DWORD cbSize);
    static DWORD __fastcall CountLine(PCSTR pData, DWORD cbSize);
    static DWORD __fastcall CountColumn(PCSTR pData, DWORD cbSize);
    static DWORD __fastcall CountWord(PCSTR pData, DWORD cbSize);

    static int is_whitespace(char ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    }

    static int is_legal_char(char ch)
    {
        return is_whitespace(ch) || (ch>='0' && ch<='9') || ch=='+' || ch=='-';
    }

    //  To be faster, this method runs without boundary check
    //  Thus eat_word may cause access violation in case the stream pointed by
    // p is not whitespace terminated or the number of calling time is not
    // carefully controlled
    //
    static void eat_word(const char*& p)
    {
        while( is_whitespace(*p) )
        {
            ++p;
        }
        while( !is_whitespace(*p) )
        {
            ++p;
        }
    }

    static int is_timestamp(const char* p)
    {
        // Compare with 1900010100, which is the minimum value accepted as a decimal
        //string represented timestamp
        //
        return p ? (_atoi64(p) > 0x0713FDA74) : false;
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
    ISampleFile* OpenTextFile(PCWSTR szFileName);
    ISampleFile* OpenSeedFile(PCWSTR szFileName);
    ISampleFile* OpenEdasFile(PCWSTR szFileName);
    ISampleFile* OpenBinFile(PCWSTR szFileName);

private:
    ISampleFile*    m_pFile;
};