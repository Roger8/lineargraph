
#ifndef _DATE_RECORD_STRUCT_H_
#define _DATE_RECORD_STRUCT_H_

typedef struct {
	unsigned short usYear;
	unsigned short usDay;
	unsigned char ucHours;
	unsigned char ucMinutes;
	unsigned char ucSeconds;
	unsigned char ucUnused;
	unsigned short usMiniSec;
} BTIME;

typedef struct {
	char szSeqNum[6];
	char chDataHeader;
	char chIdle;
	char szStationID[5];
	char szLocationID[2];
	char szChannelID[3];
	char szNetworkCode[2];
	BTIME tStart;
	unsigned short usSampleNum;
	short sRateFactor;
	short sRateMultiplier;
	unsigned char ucFlags;
	unsigned char ucIOFlags;
	unsigned char ucDataQualityFlags;
	unsigned char ucBlocketteNum;
	long lTimeCorrection;
	unsigned short usDataOffset;
	unsigned short usBlocketteOffset;
} FIX_DATA_HEADER;

/* data record blockette */
typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	float fSampleRate;
	char chFlags;
	unsigned char ucReserved;
} SAMPLE_RATE_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	float fAmplitude;
	float fPeriod;
	float fBackGroundEstimate;
	unsigned char ucEventFlags;
	unsigned char ucReserved;
	BTIME tOnset;
	char szDectorName[24];
} GENERIC_EVENT_DETECTION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	float fAmplitude;
	float fPeriod;
	float fBackGroundEstimate;
	unsigned char ucEventFlags;
	unsigned char ucReserved;
	BTIME tOnset;
	unsigned char szRatio[6];
	unsigned char ucLookbackValue;
	unsigned char ucPickAlgorathm;
	char szDectorName[24];
} MURDOCK_EVENT_DECTION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	BTIME tBegin;
	unsigned char ucStepNum;
	unsigned char ucFlags;
	unsigned long ulMiniStep;
	unsigned long ulMiniInterval;
	float fAmplitude;
	char szChannelInput[3];
	unsigned char ucReserved;
	unsigned long ulReferenceAmplitude;
	char szCoupling[12];
	char szRolloff[12];
} STEP_CALIBRATION_BLOCKETTE;


typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	BTIME tBegin;
	unsigned char ucReserved0;
	unsigned char ucFlags;
	unsigned long ulCalibration;
	float fperiod;
	float fAmplitude;
	char szChannelInput[3];
	unsigned char ucReserved1;
	unsigned long ulReferenceAmplitude;
	char szCoupling[12];
	char szRolloff[12];
} SINE_CALIBRATION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	BTIME tBegin;
	unsigned char ucReserved0;
	unsigned char ucFlags;
	unsigned long ulCalibration;
	float fPTOPAmplitude;
	char szChannelInput[3];
	unsigned char ucReserved1;
	unsigned long ulReferenceAmplitude;
	char szCoupling[12];
	char szRolloff[12];
	char szNoiseType[8];
} PSEUDO_RANDOM_CALIBRATION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	BTIME tBegin;
	unsigned char ucReserved0;
	unsigned char ucFlags;
	unsigned long ulCalibration;
	float fAmplitude;
	char szChannelInput[3];
	unsigned char ucReserved1;
} GENERIC_CALIBRATION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	BTIME tEnd;
	unsigned short usReserved;
} CALIBRATION_ABORT_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	float fAzimth;
	float fSlowness;
	unsigned short usConfiguration;
	unsigned short usReserved;
} BEAM_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	unsigned short usMiniArray;
} BEAM_DELAY_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	float fVCO;
	BTIME tException;
	char chMiniSec;
	unsigned char chQuality;
	unsigned long ulCount;
	char szExceptionType[16];
	char szModel[32];
	char szStatus[128];
} TIMING_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	char chEncodeFormat;
	char chWordOrder;
	char chRecordLength;
	char chReserved;
} DATA_ONLY_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	unsigned char ucTimingQuality;
	unsigned char ucMiniSecond;
	unsigned char ucReserved;
	unsigned char ucFrameCount;
} DATA_EXTENSION_BLOCKETTE;

typedef struct {
	unsigned short usBlocketteType;
	unsigned short usNextBlocketteNum;
	unsigned short usTotalLength;
	unsigned short usDataOffset;
	unsigned long ulRecordNum;
	unsigned char ucWordOrder;
	unsigned char ucDataFlags;
	unsigned char ucHeaderFieldsNum;
	char szDataHeaderString[1];
	/**/
} OPAQUE_DATA_BLOCKETTE;

#endif