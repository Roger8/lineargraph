
#ifndef _UNPACK_H_
#define _UNPACK_H_

// SEED 格式 数据压缩格式标准编号
#define DE_ASCII			0
#define DE_INT16			1
#define DE_INT24			2
#define DE_INT32			3
#define DE_FLOAT32			4
#define DE_FLOAT64			5

#define DE_STEIM1			10
#define DE_STEIM2			11
#define DE_GEOSCOPE24		12
#define DE_GEOSCOPE163		13
#define DE_GEOSCOPE164		14

#define DE_SRO				30
#define DE_DWWSSN			32

//文件的存储大小端
#define  LITTLE_ENDIAN		0
#define  BIG_ENDIAN			1


/* data frame steim */
typedef union {
	char		chByte[4];
	short	sShort[2];
	int		iInteger;
} U_DIFF;
typedef struct {
	unsigned int	uiCtrl;
	U_DIFF		uW[15];
} FRAME_STEIM;


int unpack_int_16(short* lpBuf, int iBufLen, int iSwap, 
				  int* lpData, int iDataLen);
int unpack_int_32(int* lpBuf, int iBufLen, int iSwap, 
				  int* lpData, int iDataLen);
int unpack_float_32(float* lpBuf, int iBufLen, int iSwap, 
					float* lpData, int iDataLen);
int unpack_float_64(double* lpBuf, int iBufLen, int iSwap, 
					double* lpData, int iDataLen);
int unpack_steim1(void* lpRes, int iBytes, int iSwap, 
				  int* lpData, int iDataLen, int* px0, int* pxn);
int unpack_steim2(void* lpRes, int iBytes, int iSwap, 
				  int* lpData, int iDataLen, int* px0, int* pxn);
int unpack_geoscope(const char * lpBuf, int iBufLen, int iSwap, 
					float* lpData, int iDataLen, int encoding);
int unpack_sro( int *lpBuf, int iBufLen, int iSwap, 
			   int *lpData, int iDataLen);
int unpack_dwwssn(short* lpBuf, int iBufLen, int iSwap, 
				  int* lpData, int iDataLen);

#endif
