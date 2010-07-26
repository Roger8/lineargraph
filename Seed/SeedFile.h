
#ifndef _SEED_FILE_H_
#define _SEED_FILE_H_

#include "ExportMacro.h"

class CSeedLog;
class CMapFile;
class CStationList;
struct Station;
struct FRACTION;
struct CtrlParam;

class SEED_FILE_CLASS_LIB CSeedFile
{
public:
	CSeedFile();
	~CSeedFile();

	//���ļ���У�飬��ȡ���е�̨վ������Ϣ
	int open(const WCHAR* pFileName, CSeedLog* lpLog);
	//�ر��ļ����ͷ����еĻ���ռ�
	int close();
	//��ȡһ��̨վ������,�ļ�����Ԥ��ʱʹ��,���ﲻ������ݵ�ȱʧ�Ͷ��࣬ͳһ����һ��
	int DecompStation(int index);
	//����̨վ������
	int GetStationCount();
	//����̨վ����Ϣ
	Station* GetStationInfo(int index);

	//����״̬�������ͷ��ļ�ռ���ڴ�
	//�����ظ�����
	void suspend();
	//�ظ���״̬��
	//�����ظ�����
	int resume();

	//������������
	int ExportData(const WCHAR* ppath, int index, CtrlParam* pctrl);
	int ExportHeaderInfo(const WCHAR* lpPath);

	//���ض���ʱ����ڣ���ȡһ��̨վ���������ݵ�
	int DcmpExtrStationDataByTime(int index, SYSTEMTIME* lpStart, SYSTEMTIME* lpEnd, 
			int numerator, int denominator);
	//��ȡһ��̨վ���������ݵ㣬����ʱ���Ӱ��
// 	int DcmpExtrStationData(int index, int numerator, int denominator);

	//��ȡstation list�е���Ϣ
	int GetStationIndex(const WCHAR* pstation, const WCHAR* pchannel);

	//�ͷŽ�ѹ������
	int FreeStationData();
	//������־
	int SetLogFile(CSeedLog* lpLog);
	//������ڱ�׼seed��ʽ
	void SetDefaultFormat(int iformat);

	//���ȸ��ٱ���
	int m_position;

private:
	//CFapFile CStationList ����CSeedFile�Լ������ģ�ʼ�մ���
	//CSeedLog������紫��ģ�����Ϊ�գ����ԣ�log����ǰ������log����Ч��
	CMapFile* seed;
	CStationList* list;
	CSeedLog* log;	//ȫ��ֻ��һ��log�࣬����紫��
	int m_step, m_swap;
	int m_defFormat;
	WCHAR path[MAX_PATH];
	WCHAR name[MAX_PATH];

	//��ȡһ��record������
	int DecompOneRecord(char* lpBase, int* lpData, int iBufLen);
	//���ض�Ƶ�ʳ�ȡһ����������ݵ�
	int ExtractOneDataRecord(const int* lpData, int iCnt, FRACTION* pbegin, 
		FRACTION* pend, FRACTION* pstep, int* lpDest);

	int GetExportFileName(const WCHAR* lpPath, WCHAR* lpName, int iType, Station* lpst);
};

#endif