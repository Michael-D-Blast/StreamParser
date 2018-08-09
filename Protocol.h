#pragma once

#include "Platform.h"

#define CMD_STOP_CHARGING			1
#define RSP_STOP_CHARGING			2
#define CMD_START_CHARGING			3
#define RSP_START_CHARGING			4
#define CMD_SETTING_PARAMETERS		5
#define RSP_SETTING_PARAMETERS		5

#define RSP_HEART_BEAT				101
#define CMD_HEART_BEAT				102
#define RSP_CHARGING_STATUS_UPLOAD	103
#define CMD_CHARGING_STATUS_UPLOAD	104
#define RSP_SIGN_IN					105
#define CMD_SIGN_IN					106
#define RSP_WARNING_UPLOAD			107
#define CMD_WARNING_UPLOAD			108
#define RSP_CHARGING_STARTED		109
#define CMD_CHARGING_STARTED		110
#define CMD_FETCH_REALTIME_STATUS	111
#define RSP_FETCH_REALTIME_STATUS	112

#define RSP_CHARGING_RECORD_UPLOAD	201
#define CMD_CHARGING_RECORD_UPLOAD	202


enum StopChargingCmdResult	// Refer to CmdStopCharging
{
	SUCCESS = 0,
	FAILURE,
	NOT_IN_CHARGING,
};

enum AllowChargingWhenOfflineFlag	// Refer to RspStopCharging
{
	NOT_ALLOW = 0,
	ALLOW,
};

enum ChargingMode	// Refer to CmdStartCharging
{
	UNLIMITED = 0,
	TIME,
	ELECTRICITY,
	SOC,
};

enum StartChargingResult	// Refer to CmdStartCharging
{
	SUCCESS = 0,
	CC1_NOT_CONNECTED,
	INSULATION_DETECTION_TIMEOUT,
	INSULATION_DETECTION_ABNORMAL,
	OUT_OF_SERVICE,
	SYSTEM_FAULT,
	APS_NOT_MATCH,
	APS_START_FAILURE,
	TIMEOUT,
	BMS_HANDSHAKE_FAILURE,
	BMS_COMMUNICATION_CONFIGURATION_FAILURE,
	BMS_PARAMETER_ABNORMAL,
	ALREADY_IN_STARTING,
	UNKNOWN_REASON,
};

enum SettingParametersCmdResult		// Refer to RspSettingParamters
{
	SUCCESS = 0,
	FAILURE,
	WRONG_PARAMETER,
};

enum WorkingStatus	// Refer to CmdChargingStatusUpload
{
	IDLE = 0,
	STARTING_CHARGING,
	IN_CHARGING,
	CHARGING_COMPLETE,
	START_FAILURE,
	GUN_IS_NOT_CHARING,
	IN_DETECTING,
	SYSTEM_FAULT,
};

enum BMSChargingMode	// Refer to CmdChargingStatusUpload
{
	CONSTANT_VOLTAGE = 0,
	CONSTANT_CURRENT,
};

enum SignInFlag		// Refer to RspSignIn
{
	SUCCESS = 0,
	FAILURE,
};

enum WarningLevel	// Refer to CmdWarningUpload
{
	NOT_AFFECT_USAGE = 0,
	AFFECT_USAGE,
};

enum BRMBatteryPropertyRight	// Refer to CmdChargingStarted
{
	RENT = 1,
	OWN,
};

enum ChargingRecordHandleResult	// Refer to ChargingRecordUpload
{
	DONE = 0,
	UNACCEPTABLE,
	BUSY,
};

#pragma pack (push)
#pragma pack (1)

struct FrameHead
{
	uint16 magicNumber;		// Fixed to 0xF566
	char   chargerNumber[32];	// Fill with '\0' at the end if length is less then 32
	uint16 frameLength;		// The length of the whole frame
	uint16 cmdType;
	uint16 cmdSerialNumber;	// A random number
	uint8 cmdBody[0];
};

struct FrameTail
{
	uint8 status;	// Currently, according to protocal 2018_8_6, when status is 1, sign area is supported
	char signatureInMD5[32];	// Generated from timeInBCD and private key of charger, private key is provided by customer
	uint8 timeInBCD[8];		// eg. 2015-07-22-13-16-15 -> 0x20 0x15 0x07 0x22 0x13 0x16 0x15 0xff
	uint8 checksum;		// cmdType + cmdBody, lower 8bits
};

/////////////////////////////////////
// Commands from server to charger //
/////////////////////////////////////

// CMD 1
struct CmdStopCharging
{
	uint8 gunIndex;		// From 1
	uint8 frameTail[0];
};

// CMD 2
struct RspStopCharging
{
	uint8  gunIndex;		// From 1
	uint32 cmdResult;	// 0: success, 1: failure, 2: not in charging
	char   chargingSerialNumber[32];	// The one received in CmdStartCharging. Fill with 0 if not in charging
	uint8  frameTail[0];
};

// CMD 3
struct CmdStartCharging
{
	uint8  gunIndex;		// From 1
	char   passwordForStopCharging[8];	// Fill with 0 at the end if length is less then 8
	char   userID[32];	// Fill with \0 at the end if length is less then 8
	uint8  allowChargingWhenOffline;	// Refer to AllowChargingWhenOfflineFlag
	uint8  chargingMode;	// Refer to ChargingMode
	uint16 chargingEndValue;	// Unit diffs with mode, time: s, electricity: degree, soc: 0.1%
	char   chargingSerialNumber[32];		// Fill with \0 at the end if length is less then 32
	uint8  frameTail[0];
};

// CMD 4
struct RspStartCharging
{
	uint8  gunIndex;		// From 1
	uint32 startChargingResult;	// Refer to StartChargingResult
	char   passwordForStopCharging[8];	// Fill with 0 at the end if length is less then 8
	char   chargingSerialNumber[32];		// Fill with \0 at the end if length is less then 32
	uint8  frameTail[0];
};

// CMD 5
struct CmdSettingParameters
{
	uint8 heartBeatIntervalInSec;
	uint8 heartBeatTimeoutTimes;
	uint8 informationUploadInterval;
	char  serverUrl[32];
	uint8 serverPort[4];	// eg. 10001 -> 0x2711 -> 0x11 0x27
	char  chargerQRCode[256];
	char  chargerID[32];
	uint8 timeInBCD[8];
	uint8 frameTail[0];
};

// CMD 6
struct RspSettingParamters
{
	uint8 cmdResult;	// Refer to SettingParametersCmdResult
	uint8 frameTail[0];
};


/////////////////////////////////////
// Commands from charger to server //
/////////////////////////////////////

// CMD 101
struct RspHeartBeat
{
	uint16 heartBeatSequenceNum;
	uint8 frameTail[0];
};

// CMD 102
struct CmdHeartBeat
{
	uint16 heartBeatSequenceNum;
	uint8 heartBeatIntervalInSec;
	uint8 frameTail[0];
};

// CMD 103
struct RspChargingStatusUpload
{
	uint8 gunIndex;		// From 1
	uint8 frameTail[0];
};

// CMD 104
struct CmdChargingStatusUpload
{
	uint8  gunQuantity;
	uint8  gunIndex;		// From 1
	uint8  workingStatus;	// Refer to WorkingStatus
	uint16 currentSOCPercentageWithPrecision2;
	uint16 chargingDCVoltageWithPrecision1;
	uint16 chargingDCCurrentWithPrecision1;
	uint16 requiredBMSVoltageWithPrecision1;
	uint16 requiredBMSCurrentWithPrecision1;
	uint8  bmsChargingMode;	// Refer to BmsChargingMode
	uint16 remainingChargingTimeInMin;	// Invalid for AC
	uint32 charingTimeTillNowInSec;
	uint32 chargingElectricityTillNowInKwhWithPrecision2;
	uint32 electricMeterValueBeforeChargingInKwWithPrecision2;
	uint32 currentElectricMeterValueInKwWithPrecision2;
	uint32 chargingPowerInKwPerBITWithPrecision1;
	uint8  airOutletTemperature;
	uint8  environmentTemperature;
	uint8  gunTemperature;
	char   VIN[18];
	uint8  frameTail[0];
};

// CMD 105
struct RspSignIn
{
	uint8 currentServerTimeInBCD[8];
	uint8 signInFlag;	// Refer to SignInFlag
	uint8 frameTail[0];
};

// CMD 106
struct CmdSignIn
{
	uint32 startTimes;
	uint8  gunQuantity;
	uint8  heartBeatInterval;
	uint8  heartBeatTimeoutTimes;
	uint8  currentServerTimeInBCD[8];
	uint8  frameTail[0];
};

// CMD 107
struct RspWarningUpload
{
	uint8 reply;	// Fix to 0
	uint8 frameTail[0];
};

// CMD 108
struct CmdWarningUpload
{
	uint8 warningLevel;		// Refer to WarningLevel
	uint8 warningMessage[32];
	uint8 frameTail[0];
};

// CMD 109
struct RspChargingStarted
{
	uint8 gunIndex;		// From 1
	uint8 frameTail[0];
};

// CMD 110
struct CmdChargingStarted
{
	uint8  gunIndex;		// From 1
	uint32 startChargingResult;		// Refer to StartChargingResult
	uint8  chargingStartTimeInBCD[8];
	uint8  BRMBMSProtocalVersion[3];
	uint8  BRMBatteryType;
	uint32 BRMAccumulatorSystemRatedCapacityInAhWithPrecision1;
	uint32 BRMAccumulatorSystemRatedVoltageWithPrecision1;
	uint32 BRMBatteryVendor;
	uint32 BRMBatterySerialNum;
	uint16 BRMBatteryProductionDateYear;		// eg. 2015 -> 0x07DF
	uint8  BRMBatteryProductionDateMonth;
	uint8  BRMBatteryProductionDateDay;
	uint32 BRMBatteryChargingTimes;
	uint8  BRMBatteryPropertyRight;		// Refer to BRMBatteryPropertyRight
	char   BRMVIN[18];
	uint8  BRMBMSSoftwareVersion[8];
	uint32 BCPBatteryMaxVoltageWithPrecision1;
	uint32 BCPBatteryMaxCurrentWithPrecision1;
	uint32 BCPBatteryRatedCapacityInKwhWithPrecision1;
	uint32 BCPMaxTotalVoltageWithPrecision1;
	int8   BCPMaxTemperatureInOffesetTo50C;
	uint16 BCPCarAccumulatorChargeabilityPercentageWithPrecision1;
	uint32 BCPCarAccumulatorCurrentVoltageWithPrecision1;
	uint8  frameTail[0];
};

// CMD 111
struct CmdFetchRealtimeStatus
{
	uint8 gunIndex;		// From 1
	uint8 frameTail[0];
};

// CMD 112
struct RspFetchRealtimeStatus
{
	uint8  gunQuantity;
	uint8  gunIndex;		// From 1
	uint8  workingStatus;
	uint16 currentSOCPercentageWithPrecision2;
	uint16 chargingDCVoltageWithPrecision1;
	uint16 chargingDCCurrentWithPrecision1;
	uint16 requiredBMSVoltageWithPrecision1;
	uint16 requiredBMSCurrentWithPrecision1;
	uint8  bmsChargingMode;	// Refer to BmsChargingMode
	uint16 remainingChargingTimeInMin;	// Invalid for AC
	uint32 charingTimeTillNowInSec;
	uint32 chargingElectricityTillNowInKwhWithPrecision2;
	uint32 electricMeterValueBeforeChargingInKwWithPrecision2;
	uint32 currentElectricMeterValueInKwWithPrecision2;
	uint32 chargingPowerInKwPerBITWithPrecision1;
	uint8  airOutletTemperature;
	uint8  environmentTemperature;
	uint8  gunTemperature;
	char   VIN[18];
	uint8  frameTail[0];
};

// CMD 201
struct RspChargingRecordUpload
{
	uint8 gunIndex;		// From 1
	char  chargingSerialNumber[32];
	uint8 handleResult;		// Refer to ChargingRecordHandleResult
	uint8 frameTail[0];
};

// CMD 202
struct CmdChargingRecordUpload
{
	char   chargerID[32];
	uint8  gunIndex;		// From 1
	uint8  chargingStartTimeInBCD[8];
	uint8  chargingEndTimeInBCD[8];
	uint32 chargingTotalTimeInSec;
	uint16 startSOCPercentageWithPrecision2;
	uint16 endSOCPercentageWithPrecision2;
	uint32 chargingEndReason;
	uint32 chargingElectricityInKwhWithPrecision2;
	uint32 electricMeterValueBeforeChargingInKwWithPrecision2;
	uint32 electricMeterValueAfterChargingInKwWithPrecision2;
	char   VIN[18];
	char   plateNum[8];
	uint16 chargingElectricityInHalfHour[48];
	char   chargingSerialNumber[32];
	uint8  frameTail[0];
};

#pragma pack (pop)