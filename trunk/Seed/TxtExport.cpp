
//����txt export��
// #define CSEED_FILE_CLASS_IMPLEMENT

#include <stdio.h>
#include "rdseedlib.h"
#include "OutFile.h"
#include "Convert.h"
#include "OperateStruct.h"

#include "TxtExport.h"

CTxtExport::CTxtExport()
{
	m_txt= new COutFile;
	m_position = 0;
}

CTxtExport::~CTxtExport()
{
	Close(NULL);
	delete m_txt;
}

int CTxtExport::Create( const WCHAR* pfile, void* ppara )
{
	return m_txt->CreateFile(pfile, CREATE_ALWAYS, NORMAL_BUFFER_SIZE_CO, MAX_TXT_BUF_LEN);
}

int CTxtExport::Close( void* ppara )
{
	m_txt->Free();
	return TRUE;
}

int CTxtExport::IsExportValid()
{
	return m_txt->IsFileValid();
}

int CTxtExport::ExportData( int* pdata, int itype, int icnt, void* ppara )
{
	CtrlParam* pinfo;
	int addtime;
	int datatmp;
	//��Ϊ���㲽������
	FREQ loc;//��Ϊʱ�����ʹ�ã�������Ƶ�ʷ���
	int nloc;//����ֵ
	//��Ϊ����ƫ������
	FREQ rem;//��Ϊʱ�����ʹ�ã�������Ƶ�ʷ���
	int diff;//����ֵ

	//����ļ���δ����������ʧ��
	if(m_txt->IsFileValid() == FALSE || icnt < 0 )
		return -1;

	pinfo = (CtrlParam*)ppara;
	if(pinfo != NULL)
	{
		memcpy(&m_time, &pinfo->tstart, sizeof(SYSTEMTIME));
		m_time.wDayOfWeek = 0;
		diff = m_time.wMilliseconds;
		addtime = pinfo->ctrl;
	}else{
		addtime = 0;
		diff = 0;
	}
	//��������
	loc.denominator = pinfo->numerator;
	loc.numerator = pinfo->denominator;
	loc.numerator *= 1000;//����õ�����
	nloc = loc.numerator/loc.denominator;//ʱ���������������(�Ժ���Ϊ��λ)
	loc.numerator -= loc.denominator*nloc;//�����,���ų�����Ϊ��
	rem.denominator = loc.denominator;
	rem.numerator = 0;
	//��ʼ����txt�ļ�
	int nLen;
	m_position = 0;
	for(int n = 0; n < icnt; n++)
	{
		//��ȡ����
		if(itype == ARRAY_DATA)
		{
			datatmp = *pdata;
			pdata ++;
		}
		else
			datatmp = (int)pdata;
		//��ӡ����
		if(addtime)
		{
			nLen = sprintf_s(m_txt->m_lpCurrent, MAX_TXT_BUF_LEN, 
				"%04d%02d%02d%02d%02d%02d %d\r\n", 
				m_time.wYear, m_time.wMonth, m_time.wDay, 
				m_time.wHour, m_time.wMinute, m_time.wSecond, datatmp);
			//����ʱ��
			diff += nloc;//�������
			rem.numerator += loc.numerator;//��������
			if(rem.numerator >= rem.denominator)
			{
				diff ++;
				rem.numerator -= rem.denominator;
			}
			GetNextSampleTime(&diff);
		}
		else
		{
			nLen = Print_asm(m_txt->m_lpCurrent, datatmp);//more faster 
			// 			sprintf_s(m_txt->m_lpCurrent, MAX_TXT_BUF_LEN, "%d\r\n", datatmp);
		}
		if(m_txt->Adjust(nLen) == FALSE)
		{
			m_txt->Save();
			return -1;
		}
		m_position ++;//��¼����
	}

	//�����ļ�
	m_txt->Save();

	return icnt;
}


void CTxtExport::GetNextSampleTime(int* interval)
{
	unsigned int second;

	//get the seconds 
	second = unsigned int(*interval)/1000;//������ֵ

	//second
	if(second > 0 )
	{
		*interval -= second*1000;//���غ���ֵ
		second += m_time.wSecond;
		m_time.wSecond = second%60;
		second /= 60;
		//minute
		if (second > 0)
		{
			second += m_time.wMinute;
			m_time.wMinute = second%60;
			second /= 60;
			//hour
			if (second > 0)
			{
				second += m_time.wHour;
				m_time.wHour = second%24;
				second /= 24;
			}
			//day and year 
			if(second > 0)
			{
				SystemTimeAddFraction(&m_time, &m_time, (second*24*3600), 1);
			}
		}
	}
}


//convert a integer to a null-string. add \r\n\0 to the string 
int CTxtExport::Print_asm( char* lpBuf, int integer )
{
	int nLen = 0;
	__asm
	{
		mov	eax,	integer		//integer
		mov	edi,	lpBuf			//string address
		mov	esi,	edi

		xor	edx,	edx		//setEDX=0
		xor	ecx,	ecx		//setECX=0
		mov	ebx,	10		//fEDX:EAX/EBX

		//get positive integer
		test	eax,	0x80000000
		jz	positive
		not	eax
		inc	eax
		mov	byte ptr[edi],	'-'	//add a '-' symbol
		inc	edi			//esi:the buffer
		inc nLen

positive:
		add	esi,	13
start:
		div	ebx
		or	dl,	0x30		//get the char actor

		dec	esi	
		mov	byte ptr[esi],	dl	//save the data,edi:the data

		inc	ecx			//counter the char actors

		cmp	eax,0
		jz	over

		xor	edx,	edx
		jmp	start

over:
		add nLen, ecx
		cld
		rep	movsb			//move string to the destination
		mov	byte ptr[edi],	0DH	//\r
		inc	edi
		mov	byte ptr[edi],	0AH		//\n
		inc	edi
		mov	byte ptr[edi],	00H		//string over flag

	}//__asm over
	return nLen+2;//return the length of the string 
}




