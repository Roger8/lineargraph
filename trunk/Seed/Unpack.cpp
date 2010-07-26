
#include "Unpack.h"
#include "SwapTimeStrMath.h"

//与具体解压方式相关的宏，外界不会使用
/* the 32-bit data number of a frame , not contain the first integer */
#define  VALS_PER_FRAME		15


/* steim1 flags  */	
#define  STEIM1_NON_DATA	0
#define  STEIM1_8BIT		1
#define  STEIM1_16BIT		2
#define  STEIM1_32BIT		3


/* steim2 flags */
#define  STEIM2_NON_DATA	0
#define  STEIM2_8BIT		1
#define  STEIM2_123_MASK	2
#define  STEIM2_567_MASK	3

#define  DBIN1_30BIT		1
#define  DBIN2_15BIT		2
#define  DBIN3_10BIT		3

#define  DBIN5_6BIT			0
#define  DBIN6_5BIT			1
#define  DBIN7_4BIT			2


/* Defines of some masks */
#define  MAX12	0x07ff				/* maximum 12 bit positive */
#define  MAX16	0x7fff				/* maximun 16 bit positive */
#define  MAX24	0x7fffff			/* maximum 24 bit positive */

/* Defines for GEOSCOPE encoding */
#define GEOSCOPE_MANTISSA_MASK		0x0fff		/* mask for mantissa */
#define GEOSCOPE_GAIN3_MASK			0x700		/* mask for gainrange factor */
#define GEOSCOPE_GAIN4_MASK			0xf000		/* mask for gainrange factor */
#define GEOSCOPE_SHIFT				12			/* # bits in mantissa */


/* Defines of the SRO encoding */
#define  SRO_MANTISSA_MASK		0x0fff		/* mask for mantissa */
#define SRO_GAINRANGE_MASK		0xf000		/* mask for gainrange factor */
#define SRO_SHIFT				12			/* # bits in mantissa */


//数据解压函数

/************************************************************************/
/* unpack_int_16():
/* Unpack int_16 SEED data
/* lpBuf:			ptr to the input data
/* iBufLen:		the size of the lpBuf in items
/* iSwap:		if to swap the data
/* lpData:		ptr to the unpacked data array
/* iDataLen:		the size of lpData in items
/* Return:		the number of the samples
/************************************************************************/ 
int unpack_int_16(short* lpBuf, int iBufLen, int  iSwap, int* lpData, int iDataLen)
{
	int i;
	short temp;
	if(iBufLen<1) return 0;
	if(iDataLen<1) return 0;
	for(i=0; i<iBufLen && i<iDataLen; i++)
	{
		temp = lpBuf[i];
		if(iSwap)
			lpData[i] = SWAP2(temp);
		else 
			lpData[i] = (int)temp;
	}
	return i;
} /* End of unpack_int_16() */

/************************************************************************/
/* unpack_int_32:
/* Unpack int_32 SEED data 
/* Return the number of the samples
/************************************************************************/
int unpack_int_32(int* lpBuf, int iBufLen, int  iSwap, int* lpData, int iDataLen)
{
	int i;
	int temp;
	if(iBufLen<1) return 0;
	if(iDataLen<1) return 0;
	for(i=0; i<iBufLen && i<iDataLen; i++)
	{
		temp = lpBuf[i];
		if(iSwap)
			lpData[i] = SWAP4(temp);
		else 
			lpData[i] = temp;
	}
	return i;
} /* unpack_int_32() */

/************************************************************************/
/* unpack_float_32:
/* Unpack float_32 SEED data 
/* Return the number of the samples
/************************************************************************/
int unpack_float_32(float* lpBuf, int iBufLen, int iSwap, float* lpData, int iDataLen)
{
	int i;
	float temp;
	if(iBufLen<1) return 0;
	if(iDataLen<1) return 0;
	for(i=0; i<iBufLen && i<iDataLen; i++)
	{
		temp = lpBuf[i];
		if(iSwap)
			Swap4f(&temp);
		lpData[i] = temp;
	}
	return i;
} /* unpack_float_32() */

/************************************************************************/
/* unpack_float_64:
/* Unpack float_64 SEED data 
/* Return the number of the samples
/************************************************************************/
int unpack_float_64(double* lpBuf, int iBufLen, int iSwap, double* lpData, int iDataLen)
{
	int i;
	double temp;
	if(iBufLen<1) return 0;
	if(iDataLen<1) return 0;
	for(i=0; i<iBufLen && i<iDataLen; i++)
	{
		temp = lpBuf[i];
		if(iSwap)
			Swap8d(&temp);
		lpData[i] = temp;
	}
	return i;
} /* End of unpack_float_64() */

/************************************************************************/
/* unpack_steim1:
/* Unpack STEIM1 data frames
/* lpRes:	address of a frame
/* iBytes:		
/* iSwap:		the swapping flag 
/* lpDate:		the address of the unpacked data buffer
/* iDateLen:	
/* px0, pxn:	the first and the end sample data 
/* Return:		the number of the samples
/************************************************************************/
int unpack_steim1(void* lpRes, int iBytes, int iSwap, 
				  int* lpData, int iDataLen, int* px0, int* pxn)
{
	//test
	// 	int lpData[4096];
	int iCtrl;			/* the first integer of the frame */
	int fn;			/* current frame number */
	int wn;			/* current word number in the frame */
	int nd = 0;		/* the data points number */
	int iflag;			/* the compress flag in the steim1 */
	int i;
	int * lpDiff = lpData;		/* lpData temporarily work as the different buffer */;
	int* lpPrev;
	int iFrameNum = iBytes/sizeof(FRAME_STEIM);
	FRAME_STEIM* lpSteim1 = (FRAME_STEIM*)lpRes;
	/* work as the temporary variables */
	short temp_16;
	int temp_32;
	int temp_diff;
	// 	int last_data;
	int iReturn;

	if(iBytes < 1) return 0;
	// 	if(iSampleNum < 1) return 0;
	if(iDataLen < 1) return 0;

	*px0 = lpSteim1->uW[0].iInteger;
	*pxn = lpSteim1->uW[1].iInteger;

	if(iSwap)
	{
		*px0 = SWAP4(*px0);
		*pxn = SWAP4(*pxn);
	}
	/* decode each frame and get the data */
	for(fn=0; fn<iFrameNum;fn++)
	{
		iCtrl = lpSteim1->uiCtrl;
		if(iSwap) iCtrl = SWAP4(iCtrl);

		/* decode each integer in the frame */
		for(wn=0; wn<VALS_PER_FRAME; wn++)
		{/* just 15 data in the frame , the first one integer is the control word */
			// 			if(nd > iSampleNum) break;
			iflag = (iCtrl>>( 2*(VALS_PER_FRAME-wn-1) ))&0x03;

			switch(iflag)
			{
			case STEIM1_NON_DATA:
				/* jump the data x0 and xn */
				break;
			case STEIM1_8BIT:
				{/* Next 4 bytes are 4 1-byte differences */
					for(i=0; i<4 && nd<iDataLen; i++, nd++)/* add the nd to record the data number */
						*lpDiff++ = (int)lpSteim1->uW[wn].chByte[i];
				}
				break;

			case STEIM1_16BIT:
				{/* Next 4 bytes are 2 2-byte differences */
					for(i=0; i<2 && nd<iDataLen; i++)
					{
						if(iSwap)
						{
							temp_16 = lpSteim1->uW[wn].sShort[i];
							temp_16 = SWAP2(temp_16);
							*lpDiff++ = (int)temp_16;
						}
						else *lpDiff++ = (int)lpSteim1->uW[wn].sShort[i];
						/* add the nd to record the data number */
						nd++;
					}
				}
				break;

			case STEIM1_32BIT:
				{/* Next 4 bytes are 1 4-byte difference */
					if(iSwap)
					{
						temp_32 = lpSteim1->uW[wn].iInteger;
						temp_32 = SWAP4(temp_32);
						*lpDiff++ = temp_32;
					}
					else *lpDiff++ = lpSteim1->uW[wn].iInteger;
					/* add the nd to record the data number */
					nd++;
				}
				break;
			}/* end of the switch */
		}/* end of one data decoding in the frame */
		/* get the next frame */
		lpSteim1++;
	}/* end of the each frame decoding */

	/* return the record decoded */
	iReturn = nd; 

	/* compute the data from the different buffer */
	lpDiff = lpData;	/* now the lpData buffer contains the difference of the data */
	lpPrev = lpData-1;
	// 	last_data = *px0;

	temp_diff = *(lpDiff+1);	/* temporarily store the difference, the first sample is just X0, for X-1=0 */
	i = iDataLen;

	/* store the first data */
	if(i > 0)
		*lpDiff = *px0;

	/* iDataLen is greater than the data integer number */
	/* calculate the nd-2 sample data , and the first data X0 is given, and */
	/* we must calculate the last data independently, for not to go beyond the borderline */
	nd-=2; 
	while( nd>0)
	{
		nd--;
		lpDiff++;
		lpPrev++;
		*lpDiff = temp_diff + *lpPrev;	/* get the right data */
		/* save the differences data  */
		temp_diff = *(lpDiff+1);
	}
	/* the last one data */
	lpDiff++;
	lpPrev++;
	*lpDiff = temp_diff + *lpPrev;
	/* check the data here */

	/* return the integer data number */
	return iReturn;
} /* End of unpack_steim1() */

/************************************************************************/
/* unpack_steim2:
/* Unpack STEIM2 data frames 
/* lpRes:		address of a frame
/* iBytes:		
/* iSwap:		the swapping flag 
/* lpDate:		the address of the unpacked data buffer
/* iDateLen:	
/* px0, pxn:	the first and the end sample data 
/* Return: the number of the samples
/************************************************************************/
int unpack_steim2(void* lpRes, int iBytes, int iSwap, 
				  int* lpData, int iDataLen, int* px0, int* pxn)
{
	int iCtrl;			/* the first integer of the frame */
	int fn;			/* current frame number */
	int wn;			/* current word number in the frame */
	int nd = 0;		/* the data points number */
	int iflag;			/* the compress flag in the steim1 */
	int i;
	int * lpDiff = lpData;		/* lpData temporarily work as the different buffer */;
	int* lpPrev;
	int iFrameNum = iBytes/sizeof(FRAME_STEIM);
	FRAME_STEIM* lpSteim2 = (FRAME_STEIM*)lpRes;
	/* work as the temporary variables */
	int temp_diff;
	int val, dnib;
	int iReturn;
	/* use to convert the data from the encoded data */
	int bits, n, m1, m2;

	if(iBytes < 1) return 0;
	if(iDataLen < 1) 
		return 0;
	else 
		iDataLen = iDataLen/sizeof(int);

	*px0 = lpSteim2->uW[0].iInteger;
	*pxn = lpSteim2->uW[1].iInteger;

	if(iSwap)
	{
		*px0 = SWAP4(*px0);
		*pxn = SWAP4(*pxn);
	}
	/* decode each frame and get the data */
	for(fn=0; fn<iFrameNum;fn++)
	{
		iCtrl = lpSteim2->uiCtrl;
		if(iSwap) iCtrl = SWAP4(iCtrl);

		/* decode each integer in the frame */
		for(wn = 0; wn < VALS_PER_FRAME; wn++)
		{/* just 15 data in the frame , the first one integer is the control word */
			iflag = (iCtrl>>( 2*(VALS_PER_FRAME-wn-1) ))&0x03;

			switch(iflag)
			{
			case STEIM2_NON_DATA:
				/* jump the data x0 and xn */
				break;
			case STEIM2_8BIT:
				{/* Next 4 bytes are 4 1-byte differences */
					for(i=0; i<4 && nd<iDataLen; i++, nd++)/* add the nd to record the data number */
						*lpDiff++ = (int)lpSteim2->uW[wn].chByte[i];
				}
				break;

			case STEIM2_123_MASK:
				{/* Next 4 bytes are 2 2-byte differences */
					val = lpSteim2->uW[wn].iInteger;
					if(iSwap)	val = SWAP4(val);
					dnib = ((val>>30)&0x03);
					switch(dnib)
					{
					case DBIN1_30BIT:
						/* 1 30-bit difference */
						bits = 30; n = 1; m1 = 0x3fffffff; m2 = 0x20000000; break;
					case DBIN2_15BIT:
						/* 2 15-bit differences */
						bits = 15; n = 2; m1 = 0x00007fff; m2 = 0x00004000; break;
					case DBIN3_10BIT:
						/* 3 10-bit differences */
						bits = 10; n = 3; m1 = 0x000003ff; m2 = 0x00000200; break;
					default :
						/* default as the 1 30-bit integer  */
						bits = 30; n = 1; m1 = 0x3fffffff; m2 = 0x20000000; break;
					}
					for(i=(n-1)*bits; i>=0 && nd<iDataLen; i-=bits, nd++)
					{
						*lpDiff = (val>>i)&m1;
						*lpDiff = (*lpDiff&m2)? (*lpDiff | (~m1)) : (*lpDiff);
						lpDiff++;
					}
				}
				break;

			case STEIM2_567_MASK:
				{/* Next 4 bytes are 2 2-byte differences */
					val = lpSteim2->uW[wn].iInteger;
					if(iSwap)	val = SWAP4(val);
					dnib = ((val>>30)&0x03);
					switch(dnib)
					{
					case DBIN5_6BIT:
						/* 5 6-bit difference */
						bits = 6; n = 5; m1 = 0x0000003f; m2 = 0x00000020; break;
					case DBIN6_5BIT:
						/* 6 5-bit differences */
						bits = 5; n = 6; m1 = 0x0000001f; m2 = 0x00000010; break;
					case DBIN7_4BIT:
						/* 7 4-bit differences */
						bits = 4; n = 7; m1 = 0x0000000f; m2 = 0x00000008; break;
					default :
						/* default as the 1 30-bit integer  */
						bits = 6; n = 5; m1 = 0x0000003f; m2 = 0x00000020; break;
					}
					for(i=(n-1)*bits; i>=0 && nd<iDataLen; i-=bits, nd++)
					{
						*lpDiff = (val>>i)&m1;
						*lpDiff = (*lpDiff&m2)? (*lpDiff | (~m1)) : (*lpDiff);
						lpDiff++;
					}
				}
				break;
			}/* end of the switch */
		}/* end of one data decoding in the frame */
		/* get the next frame */
		lpSteim2++;
	}/* end of the each frame decoding */

	/* return the record decoded ,total n+1 integers */
	iReturn = nd; 

	/* compute the data from the different buffer */
	lpDiff = lpData;	/* now the lpData buffer contains the difference of the data */
	lpPrev = lpData-1;

	temp_diff = *(lpDiff+1);	/* temporarily store the difference */
	i = iDataLen;

	/* store the first data */
	if(i > 0)
		*lpDiff = *px0;

	/* iDataLen is greater than the data integer number */
	/* calculate the nd-2 sample data , and the first data X0 is given, and */
	/* we must calculate the last data independently, for not to go beyond the borderline */
	nd-=2; 
	while( nd>0)
	{
		nd--;
		lpDiff++;
		lpPrev++;
		*lpDiff = temp_diff + *lpPrev;	/* get the right data */
		/* save the differences data  */
		temp_diff = *(lpDiff+1);
	}
	/* the last one data */
	lpDiff++;
	lpPrev++;
	*lpDiff = temp_diff + *lpPrev;
	/* check the data here */

	/* return the integer data number */
	return iReturn;
} /* End of unpack_steim2() */


/************************************************************************/
/* unpack_geoscope:
/* Unpack GEOSCOPE gain ranged data(demulituplexed only) encoded 
/* SEED data 
/* Return: the number of the samples
/************************************************************************/
int unpack_geoscope(const char * lpBuf,  int iBufLen, int iSwap, float* lpData, int iDataLen, int encoding)
{
	int nd = 0;
	int iMantissa;
	int iGainRange;
	int iExponent;
	int i;
	unsigned __int64 exp2val;
	short sint;
	double dSample = 0.0;
	union{
		unsigned char ucByte[4];
		unsigned int uInteger;
	} sample32;

	if(iBufLen < 1) return 0;
	if(iDataLen < 1) return 0;
	if(encoding != DE_GEOSCOPE24 && encoding != DE_GEOSCOPE163 && encoding != DE_GEOSCOPE164)
		return -1;

	for(nd=0; nd<iBufLen && nd<iDataLen; nd++)
	{
		switch(encoding)
		{
		case DE_GEOSCOPE24:
			{/* */
				sample32.uInteger = 0;
				if(iSwap)
					for(i=0; i<3; i++)
						sample32.ucByte[2-i] = lpBuf[i];
				else 
					for(i=0; i<3; i++)
						sample32.ucByte[1+i] = lpBuf[i];

				iMantissa = sample32.uInteger;
				/* Take 2's complement for mantissa for overflow */
				if(iMantissa > MAX24)
					iMantissa -= 2*(MAX24 + 1);
				dSample = (double)iMantissa;
			}
			break;
		case DE_GEOSCOPE163:
			{
				sint = *(short*)lpData;
				if(iSwap)
					sint = SWAP2(sint);

				/* Recover mantissa and gain range factor */
				iMantissa = (sint & GEOSCOPE_MANTISSA_MASK);
				iGainRange = ((sint & GEOSCOPE_GAIN3_MASK) >> GEOSCOPE_SHIFT);

				/* Exponent is just iGainRange for GEOSCOPE */
				iExponent = iGainRange;

				/* Calculate sample as mantissa / 2^exponent */
				exp2val = (unsigned __int64) 1 << iExponent;
				dSample = ((double) (iMantissa-2048)) / exp2val;
			}
			break;
		case DE_GEOSCOPE164:
			{
				sint = *(short*)lpData;
				if(iSwap)
					sint = SWAP2(sint);

				/* Recover mantissa and gain range factor */
				iMantissa = (sint & GEOSCOPE_MANTISSA_MASK);
				iGainRange = ((sint & GEOSCOPE_GAIN4_MASK) >> GEOSCOPE_SHIFT);

				/* Exponent is just iGainRange for GEOSCOPE */
				iExponent = iGainRange;

				/* Calculate sample as mantissa / 2^exponent */
				exp2val = (unsigned __int64) 1 << iExponent;
				dSample = ((double) (iMantissa-2048)) / exp2val;
			}
			break;
		}/* end of the switch case */

		/* Save sample in output array */
		lpData[nd] = (float)dSample;

		/* Increment lpData pointer depending on size */
		switch (encoding)
		{
		case DE_GEOSCOPE24:
			lpBuf += 3;
			break;
		case DE_GEOSCOPE163:
		case DE_GEOSCOPE164:
			lpData += 2;
			break;
		}
	}/* end of the for cycle */
	return nd;
} /* End of unpack_geoscope() */


/************************************************************************/
/* unpack_sro:
/* Unpack SRO gain ranged data encoded SEED data 
/* Return: the number of the samples 
/************************************************************************/
int unpack_sro( int *lpBuf, int iBufLen, int iSwap, int *lpData, int iDataLen)
{
	int nd = 0;
	int iMantissa;
	int iGainRange;
	int iAddGainrage;
	int iMult;
	int iAddResult;
	int iExponent;
	unsigned short usInt;
	int iSample;

	if(iBufLen < 0) return 0;
	if(iDataLen < 0) return 0;

	iAddGainrage = 0;
	iMult = -1;
	iAddResult = 10;

	for(nd=0; nd<iBufLen && nd<iDataLen; nd++)
	{
		usInt = *(unsigned short*)lpBuf;
		if ( iSwap )
			usInt = SWAP2(usInt);

		/* Recover mantissa and gain range factor */
		iMantissa = (usInt & SRO_MANTISSA_MASK);
		iGainRange = (usInt & SRO_GAINRANGE_MASK) >> SRO_SHIFT;

		/* Take 2's complement for mantissa */
		if ( iMantissa > MAX12 )
			iMantissa -= 2 * (MAX12 + 1);

		/* Calculate exponent, SRO exponent = 0..10 */
		iExponent = (iMult * (iGainRange + iAddGainrage)) + iAddResult;

		if ( iExponent < 0 || iExponent > 10 )
			return -1;
		/* Calculate sample as mantissa * 2^exponent */
		iSample = iMantissa*((unsigned __int64) 1 << iExponent);

		/* Save sample in output array */
		lpData[nd] = iSample;
	}
	return nd;
} /* End of unpack_sro() */ 

/************************************************************************/
/*  unpack_dwwssn:
/* Unpack DWWSSN encoded SEED data
/* Return: the number of the samples
/************************************************************************/
int unpack_dwwssn(short* lpBuf, int iBufLen, int iSwap, int* lpData, int iDataLen)
{
	int nd = 0;
	int iSample;
	unsigned short usInt;

	if(iBufLen) return 0;
	if(iDataLen) return 0;

	for(nd=0; nd<iBufLen && nd<iDataLen; nd++)
	{
		usInt = *(unsigned short*)lpBuf;
		if(iSwap)
			usInt = SWAP2(usInt);
		iSample = (int)usInt;

		/* Take 2's complement for sample */
		if ( iSample > MAX16 )
			iSample -= 2 * (MAX16 + 1);

		/* Save sample in output array */
		lpData[nd] = iSample;
	}
	return nd;
} /* End of the unpack_dwwssn() */
