/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 */
// Copyright (c) 2018 Qualcomm Technologies, Inc.
// All rights reserved.
// Redistribution and use in source and binary forms, with or without modification, are permitted (subject to the limitations in the disclaimer below) 
// provided that the following conditions are met:
// Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// Neither the name of Qualcomm Technologies, Inc. nor the names of its contributors may be used to endorse or promote products derived 
// from this software without specific prior written permission.
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE. 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "qurt_signal.h"
#include "qapi/qurt_thread.h"
#include "stdint.h"
#include <qcli.h>
#include <qcli_api.h>
#include <qurt_timer.h>
#include <qurt_pipe.h>
#include "qapi_timer.h"
#include "qapi/qapi_status.h"
#include <qapi_i2c_master.h>
#include "qapi_otp_tlv.h"
#include "qapi_timer.h"
#include "qurt_timer.h"    /* Timer for Throughput Calculation.         */
#include "home_automation.h"
#include "qcli_util.h"
#include "ble_server.h" /* OTA service API.                        */
#include "qapi_fs.h"


/*NEW: */
extern uint16_t smoke_Detector_Value;


QCLI_Group_Handle_t ble_Group;

#define LOG_ERROR(...)                    QCLI_Printf(ble_Group, __VA_ARGS__)
#define LOG_WARN(...)                     QCLI_Printf(ble_Group, __VA_ARGS__)
#define LOG_INFO(...)                     QCLI_Printf(ble_Group, __VA_ARGS__)
#define LOG_VERBOSE(...)                  QCLI_Printf(ble_Group, __VA_ARGS__)

extern uint16_t bulb_State;

BLBD_Q_t *blbd_qdata;

BLBD_Q_t timer_signal = {BLBD_PERIODIC_TIMER_SIGNAL_INTR, 0}; 

qapi_TIMER_set_attr_t BLBD_Set_Timer_Attr;

qapi_TIMER_define_attr_t BLBD_Create_Timer_Attr;

qapi_BLE_BD_ADDR_t Current_Conn_Addr = {0};

/* Internal Variables to this Module (Remember that all variables    */
/* declared static are initialized to 0 automatically by the         */
/* compiler as part of standard C/C++).                              */

QCLI_Group_Handle_t ble_Group;               /* Handle for our main QCLI Command Group. */

static DeviceInfo_t *CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress);

static uint32_t            BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static uint32_t            ScanTimerID;             /* Scan Timer ID.                  */

static uint32_t            ConnTimerID;             /*  connection Timer ID.                  */

static qapi_BLE_HCI_DriverInformation_t HCI_DriverInformation;
                                                    /* The HCI Driver structure that   */
                                                    /* holds the HCI Transports        */
                                                    /* settings used to initialize     */
                                                    /* Bluetopia.                      */

static unsigned int        ConnectionCount;         /* Holds the number of connected   */
                                                    /* remote devices.                 */

DeviceInfo_t       *DeviceInfoList;          /* Holds the list head for the     */
                                                    /* remote device info list         */

DeviceInfo_t       *PersistDeviceInfoList;

typedef char               BoardStr_t[16];          /* User to represent a structure to*/
                                                    /* hold a BD_ADDR return from      */
                                                    /* BD_ADDRToStr.                   */



   /* Generic Access Profile (GAPLE) Internal Variables.                */

static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */

static BLEParameters_t     BLEParameters;           /* Variable which is used to hold  */
                                                    /* the BLE Scan/Advertising/...    */
                                                    /* Connection Parameters that have */
                                                    /* been configured at the CLI.     */

static qapi_BLE_BD_ADDR_t  LocalBD_ADDR;            /* Holds the BD_ADDR of the        */
                                                    /* local device.                   */

static qapi_BLE_BD_ADDR_t  SelectedRemoteBD_ADDR;   /* Holds the BD_ADDR of the        */
                                                    /* connected remote device that is */
                                                    /* currently selected.             */

static qapi_BLE_BD_ADDR_t  SecurityRemoteBD_ADDR;   /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
static qapi_BLE_BD_ADDR_t  BLBD_SecurityRemoteBD_ADDR;   /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */
                                                    /* authenticating.                 */

static qapi_BLE_Encryption_Key_t ER = {0x28, 0xBA, 0xE1, 0x37, 0x13, 0xB2, 0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51};
                                                    /* The Encryption Root Key should  */
                                                    /* be generated in such a way as   */
                                                    /* to guarantee 128 bits of        */
                                                    /* entropy.                        */

static qapi_BLE_Encryption_Key_t IR = {0x41, 0x09, 0xA0, 0x88, 0x09, 0x6B, 0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE};
                                                    /* The Identity Root Key should    */
                                                    /* be generated in such a way as   */
                                                    /* to guarantee 128 bits of        */
                                                    /* entropy.                        */

static qapi_BLE_Encryption_Key_t DHK;               /* The DHK key can be              */
                                                    /* regenerated on the fly using the*/
                                                    /* constant IR and ER keys and     */
                                                    /* are used globally, for all      */
                                                    /* devices.                        */

static qapi_BLE_Encryption_Key_t IRK;               /* The IRK key can be              */
                                                    /* regenerated on the fly using the*/
                                                    /* constant IR and ER keys and     */
                                                    /* are used globally, for all      */
                                                    /* devices.                        */

static boolean_t           ScanInProgress;          /* A boolean flag to show if a scan*/
                                                    /* is in process                   */

static boolean_t           LocalDeviceIsMaster;     /* Variable which indicates if the */
                                                    /* local device is the master      */
                                                    /* of the connection.              */

static qapi_BLE_GAP_LE_Address_Type_t  RemoteAddressType;
                                                    /* Variable which holds the remote */
                                                    /* address type for a connected    */
                                                    /* remote device until it can be   */
                                                    /* stored in the remote device     */
                                                    /* information.                    */

static qapi_Persist_Handle_t PersistHandle;         /* Variable which holds the handle */
                                                    /* to a persistent storage         */
                                                    /* instance.                       */

static boolean_t                                  LocalOOBValid; /* Variable which     */
                                                    /* holds if we have received OOB   */
                                                    /* from Local device.              */

static qapi_BLE_Secure_Connections_Randomizer_t   LocalOOBRandomizer; /* Variable      */
                                                    /* which holds Local OOB           */
                                                    /* Randomizer.                     */

static qapi_BLE_Secure_Connections_Confirmation_t LocalOOBConfirmation; /* Variable    */
                                                    /* which holds Local OOB           */
                                                    /* Confirmation.                   */

static boolean_t                                  RemoteOOBValid; /* Variable which    */
                                                    /* holds if we have received OOB   */
                                                    /* from remote device.             */

static qapi_BLE_Secure_Connections_Randomizer_t   RemoteOOBRandomizer; /* Variable     */
                                                    /* which holds remote OOB          */
                                                    /* Randomizer.                     */

static qapi_BLE_Secure_Connections_Confirmation_t RemoteOOBConfirmation; /* Variable   */
                                                    /* which holds remote OOB          */
                                                    /* Confirmation.                   */

   /* BT5 Internal Variables.                                           */
#ifdef V2

#endif

   /* Automation IO Service (AIOS) Internal Variables.                  */

   /* Device Information Service (DIS) Internal Variables.              */

static uint32_t            DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

   /* Generic Access Profile Service (GAPS) Internal Variables.         */

static uint32_t            GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static Send_Info_t         SendInfo;                /* Variable that contains          */
                                                    /* information about a data        */
                                                    /* transfer process.               */


static boolean_t           DisplayAdvertisingEventData; /* Flag to indicate if we      */
                                                    /* should have verbose adv report  */
                                                    /* outputs.                        */

static qapi_TIMER_handle_t PeriodicScanTimer;

uint32_t blbd_duration = 3000;

uint16_t bulb_char_handle = 0;

int blb_num_bulbs_found = 0;

int blbd_scan_permitted = 0;

#define SCAN_PERMITTED      0x01
#define MOBILE_CONNECTED    0x02



//uint8_t state_flags = 0;
uint8_t state_flags = SCAN_PERMITTED;

   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
static char *ErrorCodeStr[] =
{
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_NO_ERROR",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_PDU",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE",
   "QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES"
} ;

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

   /* The following array is used to map Device Appearance Values to    */
   /* strings.                                                          */
static GAPS_Device_Appearance_Mapping_t AppearanceMappings[] =
{
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_UNKNOWN,                        "Unknown"                   },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_PHONE,                  "Generic Phone"             },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_COMPUTER,               "Generic Computer"          },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_WATCH,                  "Generic Watch"             },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_SPORTS_WATCH,                   "Sports Watch"              },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_CLOCK,                  "Generic Clock"             },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_DISPLAY,                "Generic Display"           },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_GENERIC_REMOTE_CONTROL, "Generic Remote Control"    },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_EYE_GLASSES,            "Eye Glasses"               },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_TAG,                    "Generic Tag"               },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_KEYRING,                "Generic Keyring"           },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_MEDIA_PLAYER,           "Generic Media Player"      },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_BARCODE_SCANNER,        "Generic Barcode Scanner"   },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_THERMOMETER,            "Generic Thermometer"       },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_THERMOMETER_EAR,                "Ear Thermometer"           },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_HEART_RATE_SENSOR,      "Generic Heart Rate Sensor" },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BELT_HEART_RATE_SENSOR,         "Belt Heart Rate Sensor"    },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_BLOOD_PRESSURE,         "Generic Blood Pressure"    },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BLOOD_PRESSURE_ARM,             "Blood Pressure: ARM"       },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_BLOOD_PRESSURE_WRIST,           "Blood Pressure: Wrist"     },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HUMAN_INTERFACE_DEVICE,         "Human Interface Device"    },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_KEYBOARD,                   "HID Keyboard"              },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_MOUSE,                      "HID Mouse"                 },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_JOYSTICK,                   "HID Joystick"              },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_GAMEPAD,                    "HID Gamepad"               },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_DIGITIZER_TABLET,           "HID Digitizer Tablet"      },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_CARD_READER,                "HID Card Reader"           },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_DIGITAL_PEN,                "HID Digitizer Pen"         },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_HID_BARCODE_SCANNER,            "HID Bardcode Scanner"      },
   { QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_GLUCOSE_METER,          "Generic Glucose Meter"     }
} ;

#define NUMBER_OF_APPEARANCE_MAPPINGS     (sizeof(AppearanceMappings)/sizeof(GAPS_Device_Appearance_Mapping_t))

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */


#define MAXIMUM_TEST_BUFFER                  1024

static unsigned int        NumberACLPackets;

static unsigned int        NumberOutstandingACLPackets;

static unsigned int        MaxACLPacketSize;

static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter);

static QCLI_Command_Status_t StorePersistentData(void);
//static QCLI_Command_Status_t LoadPersistentData(DeviceInfo_t **Headlist);
static QCLI_Command_Status_t LoadPersistentData(DeviceInfo_t **Headlist,qapi_BLE_BD_ADDR_t *BulbAddress , qapi_BLE_GAP_LE_Address_Type_t *type);

static void QAPI_BLE_BTPSAPI GATT_BLBD_Service_Discovery_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, uint32_t CallbackParameter);

static QCLI_Command_Status_t DiscoverBLBDServices(qapi_BLE_BD_ADDR_t BD_ADDR);

static void QAPI_BLE_BTPSAPI BLBD_GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter);

static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress);


static void BLBDPopulateHandles(AIOP_Client_Information_t *ClientInfo, 
   qapi_BLE_GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData, DeviceInfo_t *DeviceInfo);



/**
 * @func : CheckEncryptionStatus
 * @Desc : The following function is responsible for checking the encryption status of a connection.*/



int CheckEncryptionStatus (qapi_BLE_BD_ADDR_t RemoteDevice) {
    DeviceInfo_t *DeviceInfo;
    if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, RemoteDevice)) != NULL) {
       
        LOG_ERROR("Device infor %02x",DeviceInfo->Flags );
        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ENCRYPTED) {
            LOG_ERROR("**********Encrypted\n\n");
            return 0;                                        
        }
    }     
    return -1;
}


/**
 * @func : BD_ADDRToStr
 * @Desc : The following function is responsible for converting data of type
 *         BD_ADDR to a string.
 */
static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
    snprintf((char *)BoardStr, (sizeof(BoardStr_t)/sizeof(char)), "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

/**
 * @func : DisplayFunctionError
 * @Desc : Displays a function error message
 */
static void DisplayFunctionError(char *Function, int Status)
{
    LOG_ERROR("%s Failed: %d.\n", Function, Status);
}


static QCLI_Command_Status_t StorePersistentData(void)
{
    uint8_t                NumberDevices;
    uint8_t                Index;
    DeviceInfo_t          *DeviceInfo ;
    qapi_Status_t          Result;
    PersistentData_t      *PersistentData;
    QCLI_Command_Status_t  ret_val;


    QCLI_Printf(ble_Group, "Persist handle : %p\n" , PersistHandle);
    QCLI_Printf(ble_Group, "In StorePersistentData\n");

    /* First, check that valid Bluetooth Stack ID exists.                */
    if(BluetoothStackID)
    {
        /* Now make sure the storage handle is initialized.               */
        if(PersistHandle)
        {
            if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
            {

                DeviceInfo    = PersistDeviceInfoList;
                NumberDevices = 0;

                while(DeviceInfo)
                {
                    ++NumberDevices;
                    DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                }

                QCLI_Printf(ble_Group, "Number of devices %d\n", NumberDevices);

                //if((PersistentData = (PersistentData_t *)malloc(PERSISTENT_DATA_SIZE(1))) != NULL)
                if((PersistentData = (PersistentData_t *)malloc(sizeof(PersistentData_t))) != NULL)
                {

                    QCLI_Printf(ble_Group, "Persist handle : %p\n" , PersistHandle);
                    QCLI_Printf(ble_Group, "In store persistent data, during memory allocation\n ");
                    //memset(PersistentData, 0, PERSISTENT_DATA_SIZE(NumberDevices));
                    memset(PersistentData, 0, sizeof(PersistentData_t));

                    PersistentData->LocalAddress        = LocalBD_ADDR;
                    PersistentData->NumberRemoteDevices = 1;

                    DeviceInfo = PersistDeviceInfoList;
                    Index      = 0;

                    while(DeviceInfo)
                    {
                        QCLI_Printf(ble_Group, "Persist handle : %p\n" , PersistHandle);
                        QCLI_Printf(ble_Group, "Devices are present in the persist info list\n ");
                        if (DeviceInfo->device_type == DEVICE_TYPE_BULB) {
                            PersistentData->BulbFlag = 1;
                            memcpy(&PersistentData->BulbAddress,&DeviceInfo->RemoteAddress,sizeof(qapi_BLE_BD_ADDR_t));
                            PersistentData->BulbAddressType = DeviceInfo->RemoteAddressType;
                        } else { 
                            PersistentData->RemoteDevices.LastAddress     = DeviceInfo->RemoteAddress;
                            PersistentData->RemoteDevices.LastAddressType = DeviceInfo->RemoteAddressType;
                            PersistentData->RemoteDevices.device_type =     DeviceInfo->device_type;
                            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                            {
                                PersistentData->RemoteDevices.Flags             |= PERSISTENT_REMOTE_DEVICE_DATA_FLAG_LTK_VALID;
                                PersistentData->RemoteDevices.EncryptionKeySize  = DeviceInfo->EncryptionKeySize;
                                PersistentData->RemoteDevices.LTK                = DeviceInfo->LTK;
                            }

                            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                            {
                                PersistentData->RemoteDevices.Flags               |= PERSISTENT_REMOTE_DEVICE_DATA_FLAG_IDENTITY_VALID;
                                PersistentData->RemoteDevices.IdentityAddress      = DeviceInfo->IdentityAddressBD_ADDR;
                                PersistentData->RemoteDevices.IdentityAddressType  = DeviceInfo->IdentityAddressType;
                                PersistentData->RemoteDevices.IRK                  = DeviceInfo->IRK;
                            }

                        }
                        DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                        QCLI_Printf(ble_Group, "Hanged inside the loop \n");
                    }

                    QCLI_Printf(ble_Group, "before put*******************************");
                    QCLI_Printf(ble_Group, "Persist handle : %p\n" , PersistHandle);
                    Result = qapi_Persist_Put(PersistHandle, sizeof(PersistentData_t), (uint8_t *)PersistentData);

                    QCLI_Printf(ble_Group, "After put*******************************");
                    QCLI_Printf(ble_Group, "Persist handle : %p\n" , PersistHandle);
                    if(Result == QAPI_OK)
                    {
                        QCLI_Printf(ble_Group, "qapi_Persist_Put_Data() Success.\n");

                        ret_val = QCLI_STATUS_SUCCESS_E;
                    }
                    else
                    {
                        QCLI_Printf(ble_Group, "qapi_Persist_Put_Data() Failure: %d.\n", Result);

                        ret_val = QCLI_STATUS_ERROR_E;
                    }

                    free(PersistentData);
                }
                else
                {
                    QCLI_Printf(ble_Group, "Unable to allocate memory\n");

                    ret_val = QCLI_STATUS_ERROR_E;
                }

                qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
            }
            else
            {
                QCLI_Printf(ble_Group, "Unable to acquire Bluetooth Stack Lock.\n");

                ret_val = QCLI_STATUS_ERROR_E;
            }
        }
        else
        {
            QCLI_Printf(ble_Group, "Persistent Storage Not Initialized\n");

            ret_val = QCLI_STATUS_ERROR_E;
        }
    }
    else
        ret_val = QCLI_STATUS_ERROR_E;

    return(ret_val);
}





/* The following function deletes (and frees all memory) every       */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{

   qapi_BLE_BSC_FreeGenericListEntryList((void **)(ListHead), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr));
}

//DeviceInfo_t *DeviceInfoList;

#if 0
static QCLI_Command_Status_t LoadPersistentData(DeviceInfo_t **Headlist)
{
    uint8_t                Index;
    uint32_t               DataSize;
    DeviceInfo_t          *DeviceInfo;
    DeviceInfo_t          *DeviceInfoList;
    qapi_Status_t          Result;
    PersistentData_t      *PersistentData;
    QCLI_Command_Status_t  ret_val;


    /* First, check that valid Bluetooth Stack ID exists.                */
    if(BluetoothStackID)
    {
        /* Now make sure the storage handle is initialized.               */
        if(PersistHandle)
        {
            if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
            {
                /* Don't proceed if there are devices in the list unless a  */
                /* "force" was specified"                                   */
                // if(((Parameter_Count >= 1) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value)) || (!DeviceInfoList))
                //  {
                /* Attempt to read the data.                             */
                Result = qapi_Persist_Get(PersistHandle, &DataSize, (uint8_t **)&PersistentData);

                if(Result == QAPI_OK)
                {
                    QCLI_Printf(ble_Group, "qapi_Persist_Get() Success. Number devices: %u\n", PersistentData->NumberRemoteDevices);
                    /* Verify the data and length seem valid.             */
                    if((PersistentData) && (DataSize >= PERSISTENT_DATA_SIZE(0)) && (DataSize >= PERSISTENT_DATA_SIZE(PersistentData->NumberRemoteDevices)))
                    {
                        /* Verify the the local BD_ADDRs match.            */
                        if(QAPI_BLE_COMPARE_BD_ADDR(LocalBD_ADDR, PersistentData->LocalAddress))
                        {


                            /* Clear the list if it is not empty (user      */
                            /* specified "force").                          */
                            if(DeviceInfoList)
                            {
                                QCLI_Printf(ble_Group, "Warning: Device List is not empty. It will be cleared.\n");
                                FreeDeviceInfoList(&DeviceInfoList);
                                DeviceInfoList = NULL;
                            }

                            /* Data all seems valid, so build the device    */
                            /* list.                                        */
                            for(Index=0;Index<PersistentData->NumberRemoteDevices;Index++)
                            {
                                /* Attempt to create a new list entry for the*/
                                /* device.                                   */
                                if((DeviceInfo = CreateNewDeviceInfoEntry(&DeviceInfoList, PersistentData->RemoteDevices[Index].LastAddress)) != NULL)
                                {
                                    /* Note the address type of the address   */
                                    /* used to create the entry.              */
                                    DeviceInfo->RemoteAddressType = PersistentData->RemoteDevices[Index].LastAddressType;

                                    /* Note any encryption information if     */
                                    /* valid.                                 */
                                    if(PersistentData->RemoteDevices[Index].Flags & PERSISTENT_REMOTE_DEVICE_DATA_FLAG_LTK_VALID)
                                    {
                                        DeviceInfo->Flags             |= DEVICE_INFO_FLAGS_LTK_VALID;
                                        DeviceInfo->EncryptionKeySize  = PersistentData->RemoteDevices[Index].EncryptionKeySize;
                                        DeviceInfo->LTK                = PersistentData->RemoteDevices[Index].LTK;
                                    }

                                    /* Note any identity information if valid.*/
                                    if(PersistentData->RemoteDevices[Index].Flags & PERSISTENT_REMOTE_DEVICE_DATA_FLAG_IDENTITY_VALID)
                                    {
                                        DeviceInfo->Flags                  |= DEVICE_INFO_FLAGS_IRK_VALID;
                                        DeviceInfo->IdentityAddressBD_ADDR  = PersistentData->RemoteDevices[Index].IdentityAddress;
                                        DeviceInfo->IdentityAddressType     = PersistentData->RemoteDevices[Index].IdentityAddressType;
                                        DeviceInfo->IRK                     = PersistentData->RemoteDevices[Index].IRK;

                                        /* We also need to set up the resolving*/
                                        /* list entry, since this is done in   */
                                        /* the GAP LE Auth callback.           */
                                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                                        DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                                        DeviceInfo->ResolvingListEntry.Local_IRK                  = IRK;
                                    }
                                }
                                else
                                {
                                    QCLI_Printf(ble_Group, "Failed to create device entry\n");
                                }
                            }

                            QCLI_Printf(ble_Group, "Persistent data loaded\n");

                            ret_val = QCLI_STATUS_SUCCESS_E;

                        } 

                    }
                    else
                    {
                        QCLI_Printf(ble_Group, "Local BD_ADDR does not match persistent data. The persistent data is invalid.\n");

                        ret_val = QCLI_STATUS_ERROR_E;
                    }
                }
                else
                {
                    QCLI_Printf(ble_Group, "Persistent data is invalid\n");

                    ret_val = QCLI_STATUS_ERROR_E;
                }

                qapi_Persist_Free(PersistHandle, (uint8_t *)PersistentData);
            }
            else
            {
                QCLI_Printf(ble_Group, "qapi_Persist_Get() Failure: %d\n", Result);

                ret_val = QCLI_STATUS_ERROR_E;
            }
            //  }
            /* else
               {
               QCLI_Printf(ble_Group, "Device List is not empty.\n");

               ret_val = QCLI_STATUS_ERROR_E;
               }*/

            qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
        }

        else
        {
            QCLI_Printf(ble_Group, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
        }
        else
        {
            QCLI_Printf(ble_Group, "Persistent Storage Not Initialized\n");

            ret_val = QCLI_STATUS_ERROR_E;
        }
    }
    else { 
        ret_val = QCLI_STATUS_ERROR_E;
    }
    *HeadList = DeviceInfoList;
    return(ret_val);
}
#endif  /*Waste*/


static QCLI_Command_Status_t LoadPersistentData(DeviceInfo_t **Headlist,qapi_BLE_BD_ADDR_t *BulbAddress , qapi_BLE_GAP_LE_Address_Type_t *type)
{
   uint8_t                Index;
   uint32_t               DataSize;
   DeviceInfo_t          *DeviceInfo;
   qapi_Status_t          Result;
   PersistentData_t      *PersistentData;
   QCLI_Command_Status_t  ret_val;
   DeviceInfo_t          *DeviceInfoList =  NULL;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now make sure the storage handle is initialized.               */
      if(PersistHandle)
      {
         if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
         {
            /* Don't proceed if there are devices in the list unless a  */
            /* "force" was specified"                                   */
           // if(((Parameter_Count >= 1) && (Parameter_List[0].Integer_Is_Valid) && (Parameter_List[0].Integer_Value)) || (!DeviceInfoList))
          //  {
               /* Attempt to read the data.                             */
               Result = qapi_Persist_Get(PersistHandle, &DataSize, (uint8_t **)&PersistentData);

               if(Result == QAPI_OK)
               {
                  QCLI_Printf(ble_Group, "qapi_Persist_Get() Success. Number devices: %u\n", PersistentData->NumberRemoteDevices);
                  /* Verify the data and length seem valid.             */
                  if((PersistentData) && (DataSize >= PERSISTENT_DATA_SIZE(0)) && (DataSize >= PERSISTENT_DATA_SIZE(PersistentData->NumberRemoteDevices)))
                  {
                     /* Verify the the local BD_ADDRs match.            */
                     if(QAPI_BLE_COMPARE_BD_ADDR(LocalBD_ADDR, PersistentData->LocalAddress))
                     {
                        /* Clear the list if it is not empty (user      */
                        /* specified "force").                          */
                        if(DeviceInfoList)
                        {
                           QCLI_Printf(ble_Group, "Warning: Device List is not empty. It will be cleared.\n");
                           FreeDeviceInfoList(&DeviceInfoList);
                           DeviceInfoList = NULL;
                        }
                        if(PersistentData->BulbFlag == 1) {
                            memcpy(BulbAddress, &(PersistentData->BulbAddress),sizeof(qapi_BLE_BD_ADDR_t));
                            memcpy(type, &(PersistentData->BulbAddressType),   sizeof(qapi_BLE_GAP_LE_Address_Type_t));
                        } 
                        /* Data all seems valid, so build the device    */
                        /* list.                                        */
                       
                      //  for(Index=0;Index<PersistentData->NumberRemoteDevices;Index++)
                       // {
                           /* Attempt to create a new list entry for the*/
                           /* device.                                   */
                           if((DeviceInfo = CreateNewDeviceInfoEntry(&DeviceInfoList, PersistentData->RemoteDevices.LastAddress)) != NULL)
                           {
                              /* Note the address type of the address   */
                              /* used to create the entry.              */
                              DeviceInfo->RemoteAddressType = PersistentData->RemoteDevices.LastAddressType;
                              DeviceInfo->device_type        = PersistentData->RemoteDevices.device_type; 

                              /* Note any encryption information if     */
                              /* valid.                                 */
                              if(PersistentData->RemoteDevices.Flags & PERSISTENT_REMOTE_DEVICE_DATA_FLAG_LTK_VALID)
                              {
                                 DeviceInfo->Flags             |= DEVICE_INFO_FLAGS_LTK_VALID;
                                 DeviceInfo->EncryptionKeySize  = PersistentData->RemoteDevices.EncryptionKeySize;
                                 DeviceInfo->LTK                = PersistentData->RemoteDevices.LTK;
                              }

                              /* Note any identity information if valid.*/
                              if(PersistentData->RemoteDevices.Flags & PERSISTENT_REMOTE_DEVICE_DATA_FLAG_IDENTITY_VALID)
                              {
                                 DeviceInfo->Flags                  |= DEVICE_INFO_FLAGS_IRK_VALID;
                                 DeviceInfo->IdentityAddressBD_ADDR  = PersistentData->RemoteDevices.IdentityAddress;
                                 DeviceInfo->IdentityAddressType     = PersistentData->RemoteDevices.IdentityAddressType;
                                 DeviceInfo->IRK                     = PersistentData->RemoteDevices.IRK;

                                 /* We also need to set up the resolving*/
                                 /* list entry, since this is done in   */
                                 /* the GAP LE Auth callback.           */
                                 DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                                 DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                                 DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                                 DeviceInfo->ResolvingListEntry.Local_IRK                  = IRK;
                              }
                           }
                           else
                           {
                              QCLI_Printf(ble_Group, "Failed to create device entry\n");
                           }
                       // }

                        QCLI_Printf(ble_Group, "Persistent data loaded\n");

                        ret_val = QCLI_STATUS_SUCCESS_E;
                     }
                     else
                     {
                        QCLI_Printf(ble_Group, "Local BD_ADDR does not match persistent data. The persistent data is invalid.\n");

                        ret_val = QCLI_STATUS_ERROR_E;
                     }
                  }
                  else
                  {
                     QCLI_Printf(ble_Group, "Persistent data is invalid\n");

                     ret_val = QCLI_STATUS_ERROR_E;
                  }

                  qapi_Persist_Free(PersistHandle, (uint8_t *)PersistentData);
               }
               else
               {
                  QCLI_Printf(ble_Group, "qapi_Persist_Get() Failure: %d\n", Result);

                  ret_val = QCLI_STATUS_ERROR_E;
               }
          //  }
           /* else
            {
               QCLI_Printf(ble_Group, "Device List is not empty.\n");

               ret_val = QCLI_STATUS_ERROR_E;
            }*/

            qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
         }
         else
         {
            QCLI_Printf(ble_Group, "Unable to acquire Bluetooth Stack Lock.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }
      }
      else
      {
         QCLI_Printf(ble_Group, "Persistent Storage Not Initialized\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   }
   else{ 
    
      ret_val = QCLI_STATUS_ERROR_E;
   }
    *Headlist = DeviceInfoList;
   return(ret_val);
}





/**
 * @func : SearchDeviceInfoEntryByBD_ADDR
 * @Desc : The following function searches the specified List for the
 *         specified Connection BD_ADDR
 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    BoardStr_t    BoardStr;
    DeviceInfo_t *ret_val = NULL;
    DeviceInfo_t *DeviceInfo;

    if (ListHead)
    {
        DeviceInfo = *ListHead;
        while (DeviceInfo)
        {
            if (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->RemoteAddress, RemoteAddress))
            {
                LOG_VERBOSE("Found device ********************\n\n");
                ret_val = DeviceInfo;
                break;
            }
            else
            {
                if(QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress))
                {
                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                    {
                        if(qapi_BLE_GAP_LE_Resolve_Address(BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                        {
                            DeviceInfo->RemoteAddress     = RemoteAddress;
                            DeviceInfo->RemoteAddressType = QAPI_BLE_LAT_RANDOM_E;

                            LOG_VERBOSE("\n");
                            LOG_VERBOSE("Resolved Address (");
                            BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                            LOG_VERBOSE("%s", BoardStr);
                            LOG_VERBOSE(")\n");
                            LOG_VERBOSE("   Identity Address:       ");
                            BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                            LOG_VERBOSE("%s\n", BoardStr);
                            LOG_VERBOSE("   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                            ret_val = DeviceInfo;
                            break;
                        }
                    }
                }
            }
            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
        }
    }

    return (ret_val);
}




static DeviceInfo_t *SearchDeviceInfoEntryByType(DeviceInfo_t **ListHead, uint8_t type)
{
   
    QCLI_Printf(ble_Group, "In SearchDeviceInfoEntryByType\n");
    DeviceInfo_t *DeviceInfo;

    if (ListHead)
    {

        DeviceInfo = *ListHead;
        while (DeviceInfo)
        {
            if (DeviceInfo->device_type == type)
            {

               return ( DeviceInfo);
            }
            
            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
        }
    }

    return NULL;
}

/**
 * @func : SearchDeviceInfoEntryByBD_ADDR
 * @Desc : The following function searches the specified List for the
 *         specified Connection BD_ADDR
 */
DeviceInfo_t *GetCurrentPeerDeviceInfo(void)
{
    return SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, SelectedRemoteBD_ADDR);
}

/**
 * @func : GetConnectionCount
 * @Desc : The following function returns current connection count
 */
int GetConnectionCount(void)
{
    return ConnectionCount;
}

/**
 * @func : DeleteDeviceInfoEntry
 * #Desc : The following function searches the specified Key Info List for
 *         the specified BD_ADDR and removes it from the List
 */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    return(qapi_BLE_BSC_DeleteGenericListEntry(QAPI_BLE_EK_BD_ADDR_T_E, (void *)(&RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead)));
}

/**
 * @func : CreateNewDeviceInfoEntry
 * @Desc : the following function will create a device information entry and
 *         add it to the specified List
 */
static DeviceInfo_t *CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    DeviceInfo_t *ret_val = NULL;
    boolean_t     Result;

    if ((ListHead) && (!QAPI_BLE_COMPARE_NULL_BD_ADDR(RemoteAddress)))
    {
        if ((ret_val = malloc(sizeof(DeviceInfo_t))) != NULL)
        {
            memset(ret_val, 0, sizeof(DeviceInfo_t));
            memcpy(&ret_val->RemoteAddress,&RemoteAddress,sizeof(qapi_BLE_BD_ADDR_t));

//            ret_val->RemoteAddress = RemoteAddress;

            Result = qapi_BLE_BSC_AddGenericListEntry_Actual(QAPI_BLE_EK_BD_ADDR_T_E, QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, RemoteAddress), QAPI_BLE_BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoInfoPtr), (void **)(ListHead), (void *)(ret_val));
            if (!Result)
            {
                free(ret_val);
            }
        }
    }

    return (ret_val);
}

/**
 * @func : AppearanceToString
 * @Desc : The following function is used to map a Appearance Value to it's
 *         string representation.
 */
static boolean_t AppearanceToString(uint16_t Appearance, char **String)
{
    boolean_t    ret_val;
    unsigned int Index;

    if (String)
    {
        for (Index=0,ret_val=FALSE;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
        {
            if (AppearanceMappings[Index].Appearance == Appearance)
            {
                *String = AppearanceMappings[Index].String;
                ret_val = TRUE;
                break;
            }
        }
    }
    else
        ret_val = FALSE;
    return (ret_val);
}


/**
 * @func : GATT_ClientEventCallback_GAPS
 * @Desc : This function will be called whenever a GATT Response is received for
 *         request that was made when this function was registered
 */
static void QAPI_BLE_BTPSAPI GATT_ClientEventCallback_GAPS(uint32_t BluetoothStackID, qapi_BLE_GATT_Client_Event_Data_t *GATT_Client_Event_Data, uint32_t CallbackParameter)
{
    boolean_t     DisplayPrompt;
    char         *NameBuffer;
    uint16_t      Appearance;
    BoardStr_t    BoardStr;
    DeviceInfo_t *DeviceInfo;

    if ((BluetoothStackID) && (GATT_Client_Event_Data))
    {
        DisplayPrompt = true;

        switch (GATT_Client_Event_Data->Event_Data_Type)
        {
            case QAPI_BLE_ET_GATT_CLIENT_ERROR_RESPONSE_E:
                if (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
                {
                    LOG_ERROR("Error Response.\n");
                    BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
                    LOG_ERROR("Connection ID:   %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID);
                    LOG_ERROR("Transaction ID:  %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID);
                    LOG_ERROR("Connection Type: %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR");
                    LOG_ERROR("BD_ADDR:         %s.\n", BoardStr);
                    LOG_ERROR("Error Type:      %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)?"Response Error":"Response Timeout");

                    if (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == QAPI_BLE_RET_ERROR_RESPONSE_E)
                    {
                        LOG_ERROR("Request Opcode:  0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode);
                        LOG_ERROR("Request Handle:  0x%04X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle);
                        LOG_ERROR("Error Code:      0x%02X.\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode);
                        LOG_ERROR("Error Mesg:      %s.\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]);
                    }
                }
                else
                    LOG_ERROR("Error - Null Error Response Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CLIENT_EXCHANGE_MTU_RESPONSE_E:
                if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
                {
                    LOG_VERBOSE("Exchange MTU Response.\n");
                    BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
                    LOG_VERBOSE("Connection ID:   %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID);
                    LOG_VERBOSE("Transaction ID:  %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID);
                    LOG_VERBOSE("Connection Type: %s.\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR");
                    LOG_VERBOSE("BD_ADDR:         %s.\n", BoardStr);
                    LOG_VERBOSE("MTU:             %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU);
                }
                else
                {
                    LOG_ERROR("Error - Null Write Response Data.\n");
                }
                break;
            case QAPI_BLE_ET_GATT_CLIENT_READ_RESPONSE_E:
                if (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
                {
                    DisplayPrompt = false;
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
                    {
                        if ((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceNameHandle)
                        {
                            if ((NameBuffer = (char *)malloc(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                            {
                                memset (NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                                memcpy (NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);
                                LOG_VERBOSE("Remote Device Name: %s.\n", NameBuffer);
                                DisplayPrompt = true;

                                free(NameBuffer);
                            }
                        }
                        else
                        {
                            if ((uint16_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
                            {
                                if (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_LENGTH)
                                {
                                    Appearance = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                                    if (AppearanceToString(Appearance, &NameBuffer))
                                        LOG_VERBOSE("Remote Device Appearance: %s(%u).\n", NameBuffer, Appearance);
                                    else
                                        LOG_VERBOSE("Remote Device Appearance: Unknown(%u).\n", Appearance);
                                }
                                else
                                    LOG_VERBOSE("Invalid Remote Appearance Value Length %u.\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                                DisplayPrompt = true;
                            }
                        }
                    }
                }
                else
                {
                    LOG_ERROR("Error - Null Read Response Data.\n");
                }
                break;
            default:
                break;
        }

        if (DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : GATT_Connection_Event_Callback
 * @Desc : Generic Attribute Profile (GATT) Event Callback function
 *         prototypes.The following function is for an GATT Connection Event Callback.
 *         This function is called for GATT Connection Events that occur on
 *         the specified Bluetooth Stack.
 */
extern volatile uint32_t notify_Thread_Flag;

static void QAPI_BLE_BTPSAPI GATT_Connection_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, uint32_t CallbackParameter)
{
    boolean_t                    DisplayPrompt;
    int                          Result;
    uint16_t                     MTU;
    BoardStr_t                   BoardStr;
    DeviceInfo_t                *DeviceInfo = NULL;
    uint32_t                     ConnectionID;
    boolean_t                    FoundMatch = FALSE;

    if ((BluetoothStackID) && (GATT_Connection_Event_Data))
    {
        DisplayPrompt = true;

        switch (GATT_Connection_Event_Data->Event_Data_Type)
        {
            case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_CONNECTION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
                {
                    ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
                    LOG_INFO("etGATT_Connection_Device_Connection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
                    BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
                    LOG_INFO("   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID);
                    LOG_INFO("   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
                    LOG_INFO("   Remote Device:   %s.\n", BoardStr);
                    LOG_INFO("   Connection MTU:  %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU);
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) == NULL)
                    {
                        if ((DeviceInfo = CreateNewDeviceInfoEntry(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) == NULL)
                            LOG_ERROR("Failed to create remote device information.\n");
                    }

                    if (DeviceInfo)
                    {
                        ConnectionCount++;
                        SelectedRemoteBD_ADDR = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice;
                        LOG_VERBOSE("\nSelected Remote Device:\n");
                        LOG_VERBOSE("   Address:       %s\n", BoardStr);
                        LOG_VERBOSE("   ID:            %u\n", ConnectionID);
                        DeviceInfo->RemoteDeviceIsMaster = (LocalDeviceIsMaster) ? FALSE : TRUE;
                        DeviceInfo->RemoteAddressType    = RemoteAddressType;
                        DeviceInfo->ConnectionID         = ConnectionID;

                        DeviceInfo->TransmitCredits = 0;

                        if (!qapi_BLE_GATT_Query_Maximum_Supported_MTU(BluetoothStackID, &MTU))
                            qapi_BLE_GATT_Exchange_MTU_Request(BluetoothStackID, DeviceInfo->ConnectionID, MTU, GATT_ClientEventCallback_GAPS, 0);
                    }
                    else
                    {
                        if ((Result = qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) != 0)
                            LOG_ERROR("qapi_BLE_GAP_LE_Disconnect returned %d.\n", Result);
                    }
                }
                else
                    LOG_ERROR("Error - Null Connection Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_DISCONNECTION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
                {
                    LOG_INFO("etGATT_Connection_Device_Disconnection with size %u: \n", GATT_Connection_Event_Data->Event_Data_Size);
                    BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
                    LOG_INFO("   Connection ID:   %u.\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID);
                    LOG_INFO("   Connection Type: %s.\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == QAPI_BLE_GCT_LE_E)?"LE":"BR/EDR"));
                    LOG_INFO("   Remote Device:   %s.\n", BoardStr);
                    if ((ConnectionCount) && (DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice)) != NULL)
                    {
                        ConnectionCount--;

                        if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                            DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
                            DeviceInfo->ConnectionID = 0;
                            LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                        }
                        else
                        {
                            LOG_VERBOSE("\nDelete device infor entry\n");
                            if ((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice)) != NULL)
                           // if ((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList,SelectedRemoteBD_ADDR )) != NULL)
                            {
                                LOG_VERBOSE("\nThe remote device information has been deleted.\n", BoardStr);
                                DeviceInfo = NULL;
                            }
                        }
                    }

                    if (ConnectionCount)
                    {
#if 1 
                        DeviceInfo = DeviceInfoList;
                        while (DeviceInfo)
                        {
                            if (DeviceInfo->ConnectionID)
                            {
                                SelectedRemoteBD_ADDR = DeviceInfo->RemoteAddress;
                                LOG_INFO("\nSelected Remote Device:\n");
                                BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                                LOG_INFO("   Address:       %s\n", BoardStr);
                                LOG_INFO("   ID:            %u\n", DeviceInfo->ConnectionID);
                                break;
                            }

                            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                        }
#endif
                    }
                }
                else
                    LOG_ERROR("Error - Null Disconnection Data.\n");
                break;
            case QAPI_BLE_ET_GATT_CONNECTION_SERVER_NOTIFICATION_E:
                if (GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
                {
                    DisplayPrompt = false;
                    if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice)) != NULL)
                    {
                        if (FoundMatch == TRUE)
                        {
                            break;
                        }

                        LOG_ERROR("Error - Unknown attribute handle for notification.\n");
                        DisplayPrompt = true;
                    }
                }
                else
                    LOG_ERROR("Error - Null Server Notification Data.\n");
                break;
            default:
                DisplayPrompt = false;
                break;
        }

        if (DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : OpenStack
 * @Desc : The following function is responsible for opening the SS1
 *         Bluetooth Protocol Stack.  This function accepts a pre-populated
 *         HCI Driver Information structure that contains the HCI Driver
 *         Transport Information.  This function returns zero on successful
 *         execution and a negative value on all errors.
 */
static int OpenStack(qapi_BLE_HCI_DriverInformation_t *HCI_DriverInformation)
{
    int                    Result;
    int                    ret_val = 0;
    char                   BluetoothAddress[16];
    uint32_t               ServiceID;
    uint8_t                HC_SCO_Data_Packet_Length;
    uint16_t               HC_Total_Num_SCO_Data_Packets;
    uint16_t               HC_Total_Num_LE_Data_Packets;
    uint16_t               HC_LE_Data_Packet_Length;
    uint8_t                TempData;
    uint8_t                Status;
    uint32_t               passkey = 123456; /*Setting the default passkey*/
    char                   BLE_NAME[20] = "QCA";

    if (!BluetoothStackID)
    {
        if(HCI_DriverInformation)
        {
            LOG_INFO("OpenStack().\n");

            Result = qapi_BLE_BSC_Initialize(HCI_DriverInformation, 0);
            if ( Result > 0 )
            {
                BluetoothStackID = Result;
                LOG_INFO("Bluetooth Stack ID: %d.\n", BluetoothStackID);

                LE_Parameters.IOCapability      = QAPI_BLE_LIC_DISPLAY_ONLY_E;
                LE_Parameters.OOBDataPresent    = FALSE;
                LE_Parameters.MITMProtection    = (LE_Parameters.IOCapability != QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E) ? DEFAULT_MITM_PROTECTION : FALSE;
                LE_Parameters.SecureConnections = DEFAULT_SECURE_CONNECTIONS;

                qapi_BLE_GAP_LE_Set_Fixed_Passkey(BluetoothStackID,&passkey);  //PASSKEY is set to default value

                Result = qapi_BLE_HCI_LE_Read_Buffer_Size(BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &TempData);

                if ((Result) || (Status != 0) || (!HC_LE_Data_Packet_Length) || (!TempData))
                {
                    if ((!qapi_BLE_HCI_Read_Buffer_Size(BluetoothStackID, &Status, &HC_LE_Data_Packet_Length, &HC_SCO_Data_Packet_Length, &HC_Total_Num_LE_Data_Packets, &HC_Total_Num_SCO_Data_Packets)) && (!Status))
                        Result = 0;
                    else
                        Result = -1;
                }
                else
                {
                    HC_Total_Num_LE_Data_Packets = (uint16_t)TempData;
                    Result                       = 0;
                }

                NumberACLPackets            = HC_Total_Num_LE_Data_Packets;
                NumberOutstandingACLPackets = 0;
                MaxACLPacketSize            = HC_LE_Data_Packet_Length;

                if (!Result)
                    LOG_VERBOSE("Number ACL Buffers: %d, ACL Buffer Size: %d\n", NumberACLPackets, MaxACLPacketSize);
                else
                    LOG_ERROR("Unable to determine ACL Buffer Size.\n");

                if (!qapi_BLE_GAP_Query_Local_BD_ADDR(BluetoothStackID, &LocalBD_ADDR))
                {
                    BD_ADDRToStr(LocalBD_ADDR, BluetoothAddress);

                    LOG_INFO("BD_ADDR: %s\n", BluetoothAddress);
                }

              //  get_ble_localdevice_name(BLE_NAME, sizeof(BLE_NAME));

                QAPI_BLE_ASSIGN_BD_ADDR(SelectedRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                QAPI_BLE_ASSIGN_BD_ADDR(BLBD_SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);



                qapi_BLE_GAP_LE_Diversify_Function(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&IR), 1,0, &IRK);
                qapi_BLE_GAP_LE_Diversify_Function(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&IR), 3, 0, &DHK);



                DeviceInfoList = NULL;

                Result = qapi_Persist_Initialize(&PersistHandle, "/spinor/ble", "ble_data", ".bin", NULL, 0);
               
                LOG_VERBOSE("Persistent handle after initializing.... %p \n", PersistHandle );
    

                
                if (Result < 0)
                    LOG_ERROR("Persistent Storage Initialization Failed: %d\n", Result);

                /* Initialize the GATT Service. */
                if ((Result = qapi_BLE_GATT_Initialize(BluetoothStackID, (QAPI_BLE_GATT_INITIALIZATION_FLAGS_SUPPORT_LE | QAPI_BLE_GATT_INITIALIZATION_FLAGS_DISABLE_SERVICE_CHANGED_CHARACTERISTIC), GATT_Connection_Event_Callback, 0)) == 0)
                {
                   


                    Result = qapi_BLE_GAPS_Initialize_Service(BluetoothStackID, &ServiceID);
                    
                    
                    if (Result > 0)
                    {
                        GAPSInstanceID = (unsigned int)Result;

                        qapi_BLE_GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, BLE_NAME);
                        qapi_BLE_GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, QAPI_BLE_GAP_DEVICE_APPEARANCE_VALUE_GENERIC_COMPUTER);

                        Result = qapi_BLE_GAP_LE_Set_Address_Resolution_Enable(BluetoothStackID, TRUE);
                        
                        
                        if (!Result)
                        {
                            qapi_BLE_GAPS_Set_Central_Address_Resolution(BluetoothStackID, GAPSInstanceID, QAPI_BLE_GAR_ENABLED_E);
                            Result = qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout(BluetoothStackID, 60);
                            if (Result)
                                LOG_ERROR("Error - qapi_BLE_GAP_LE_Set_Resolvable_Private_Address_Timeout() returned %d.\n", Result);
                        }
                        else
                            LOG_ERROR("Error - qapi_BLE_GAP_LE_Set_Address_Resolution_Enable() returned %d.\n", Result);
                    }
                    else
                        DisplayFunctionError("qapi_BLE_GAPS_Initialize_Service()", Result);

                    //               Result = qapi_BLE_SLoWP_Initialize(BluetoothStackID);

                    ret_val = 0;
                }
                else
                {
                    DisplayFunctionError("qapi_BLE_GATT_Initialize()", Result);

                    qapi_BLE_BSC_Shutdown(BluetoothStackID);
                    BluetoothStackID = 0;
                    ret_val          = -1;
                }
            }
            else
            {
                DisplayFunctionError("qapi_BLE_BSC_Initialize()", Result);
                BluetoothStackID = 0;
                ret_val          = -1;
            }
        }
        else
        {
            ret_val = -1;
        }
    }
    else
    {
        ret_val = 0;
    }

    return (ret_val);
}

/* The following function is responsible for placing the local       */
/* Bluetooth device into Pairable mode.  Once in this mode the device*/
/* will response to pairing requests from other Bluetooth devices.   */
/* This function returns zero on successful execution and a negative */
/* value on all errors.                                              */
static int SetPairable(void)
{
    int Result;
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* Attempt to set the attached device to be pairable.             */
        Result = qapi_BLE_GAP_LE_Set_Pairability_Mode(BluetoothStackID, QAPI_BLE_LPM_PAIRABLE_MODE_ENABLE_EXTENDED_EVENTS_E);

        /* Next, check the return value of the GAP Set Pairability mode   */
        /* command for successful execution.                              */
        if(!Result)
        {
            /* The device has been set to pairable mode, now register an   */
            /* Authentication Callback to handle the Authentication events */
            /* if required.                                                */
            Result = qapi_BLE_GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

            /* Next, check the return value of the GAP Register Remote     */
            /* Authentication command for successful execution.            */
            if(Result)
            {
                /* An error occurred while trying to execute this function. */
                DisplayFunctionError("GAP_LE_Register_Remote_Authentication", Result);

                ret_val = Result;
            }
        }
        else
        {
            /* An error occurred while trying to make the device pairable. */
            DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

            ret_val = Result;
        }
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

/* Generic Access Profile (GAPLE) helper functions.                  */

/* The following function is responsible for placing the Local       */
/* Bluetooth Device into General Discoverablity Mode.  Once in this  */
/* mode the Device will respond to Inquiry Scans from other Bluetooth*/
/* Devices.  This function requires that a valid Bluetooth Stack ID  */
/* exists before running.  This function returns zero on successful  */
/* execution and a negative value if an error occurred.              */
static int SetDisc(void)
{
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* * NOTE * Discoverability is only applicable when we are        */
        /*          advertising so save the default Discoverability Mode  */
        /*          for later.                                            */
        LE_Parameters.DiscoverabilityMode = QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

/* The following function is responsible for placing the Local       */
/* Bluetooth Device into Connectable Mode.  Once in this mode the    */
/* Device will respond to Page Scans from other Bluetooth Devices.   */
/* This function requires that a valid Bluetooth Stack ID exists     */
/* before running.  This function returns zero on success and a      */
/* negative value if an error occurred.                              */
static int SetConnect(void)
{
    int ret_val = 0;

    /* First, check that a valid Bluetooth Stack ID exists.              */
    if(BluetoothStackID)
    {
        /* * NOTE * Connectability is only an applicable when advertising */
        /*          so we will just save the default connectability for   */
        /*          the next time we enable advertising.                  */
        LE_Parameters.ConnectableMode = QAPI_BLE_LCM_CONNECTABLE_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        ret_val = QCLI_STATUS_ERROR_E;
    }

    return(ret_val);
}

static int CloseStack(void);
int close_stack(void)
{
    return CloseStack();
}
/**
 * @func : CloseStack
 * @Desc : The following function is responsible for closing the SS1
 *         Bluetooth Protocol Stack.  This function requires that the
 *         Bluetooth Protocol stack previously have been initialized via the
 *         OpenStack() function.
 */
static int CloseStack(void)
{
    int           ret_val = 0;
    DeviceInfo_t *DeviceInfo;

    if (BluetoothStackID)
    {
        /* If there are remote devices connected.                         */
        if(ConnectionCount)
        {
            /* Go ahead and flag that we are no longer connected to any    */
            /* remote devices.                                             */
            ConnectionCount = 0;

            /* Lock the Bluetooth stack.                                   */
            if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
            {
                /* We need to loop through the remote device information    */
                /* entries and disconnect any remote devices that are still */
                /* connected.                                               */
                DeviceInfo = DeviceInfoList;
                while(DeviceInfo)
                {
                    /* If the GATT Connection ID is valid, then we are       */
                    /* connected to the remote device.                       */
                    if(DeviceInfo->ConnectionID)
                    {
                        /* Flag that the remote device is no longer connected.*/
                        DeviceInfo->ConnectionID = 0;

                        /* Send the disconnection request.                    */
                        qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, DeviceInfo->RemoteAddress);
                    }

                    DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
                }

                /* Un-lock the Bluetooth Stack.                             */
                qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
            }
        }

        if (DISInstanceID)
        {
            qapi_BLE_DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

            DISInstanceID = 0;
        }

        if (GAPSInstanceID)
        {
            qapi_BLE_GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

            GAPSInstanceID = 0;
        }

       
        /* Release the storage instance.                                  */
        qapi_Persist_Cleanup(PersistHandle);
        PersistHandle = NULL;

        /* Cleanup GATT Module.                                           */
        qapi_BLE_GATT_Cleanup(BluetoothStackID);

        /* Simply close the Stack                                         */
        qapi_BLE_BSC_Shutdown(BluetoothStackID);

        /* Inform the user that the stack has been shut down.             */
        LOG_INFO("Stack Shutdown.\n");

        /* Free all remote device information entries.                    */
        FreeDeviceInfoList(&DeviceInfoList);
        DeviceInfoList   = NULL;

        /* Flag that the Stack is no longer initialized.                  */
        BluetoothStackID = 0;
        RemoteOOBValid   = FALSE;
        LocalOOBValid    = FALSE;

        /* Flag success to the caller.                                    */
        ret_val          = 0;
    }
    else
    {
        ret_val = -1;
    }

    return (ret_val);
}

/**
 * @func : InitializeBluetooth
 * @Desc : The following function is responsible for initializing the stack
 */
int InitializeBluetooth(void)
{
    int Result;
    int ret_val;

    QAPI_BLE_HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, 115200, QAPI_BLE_COMM_PROTOCOL_UART_E);

    /* First, check that the stack is not currently initialized.         */
    if(!BluetoothStackID)
    {
        /* Attempt to open the stack.                                     */
        Result = OpenStack(&HCI_DriverInformation);
        if(!Result)
        {
            /* Set the default pairability.                                */
            Result = SetPairable();
            if(!Result)
            {
                /* Set the default discoverability.                         */
                Result = SetDisc();
                if(!Result)
                {
                    /* Set the default connectability.                       */
                    Result = SetConnect();
		     if (!BLE_IOService())
    		     {
                         LOG_ERROR("BLE: Error Registering Services()\n");
                         return QCLI_STATUS_ERROR_E;
                     }else {
	                 LOG_ERROR("BLE: Services Registered successfully\n");
                     }	
                }
            }

            /* If the failure occurred after the stack initialized then    */
            /* shut it down.                                               */
            if(Result)
                CloseStack();
        }

        /* Set the QCLI error type appropriately.                         */
        if(!Result)
            ret_val = QCLI_STATUS_SUCCESS_E;
        else
            ret_val = QCLI_STATUS_ERROR_E;
    }
    else
    {
        /* No valid Bluetooth Stack ID exists.                            */
        LOG_WARN("Bluetooth stack is already initialized.");

        ret_val = QCLI_STATUS_SUCCESS_E;
    }

    return(ret_val);
}

/**
 * @func : SearchDeviceInfoEntryTypeAddress
 * @Desc : The following function searches the specified List for the
 *         specified Address and Type.  This function returns NULL if
 *         either the List Head is invalid, the BD_ADDR is invalid, or the
 *         Connection BD_ADDR was NOT found.
 */
static DeviceInfo_t *SearchDeviceInfoEntryTypeAddress(DeviceInfo_t **ListHead, qapi_BLE_GAP_LE_Address_Type_t AddressType, qapi_BLE_BD_ADDR_t RemoteAddress)
{
    BoardStr_t                      BoardStr;
    DeviceInfo_t                   *ret_val = NULL;
    DeviceInfo_t                   *DeviceInfo;
    qapi_BLE_GAP_LE_Address_Type_t  TempType;

    if (ListHead)
    {
        DeviceInfo = *ListHead;
        while (DeviceInfo)
        {
            if ((DeviceInfo->RemoteAddressType == AddressType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->RemoteAddress, RemoteAddress)))
            {
                ret_val = DeviceInfo;
                break;
            }
            else
            {
                if ((AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) || (AddressType == QAPI_BLE_LAT_RANDOM_IDENTITY_E))
                {
                    if (AddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                        TempType = QAPI_BLE_LAT_PUBLIC_E;
                    else
                        TempType = QAPI_BLE_LAT_RANDOM_E;
                    if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                    {
                        if ((DeviceInfo->IdentityAddressType == TempType) && (QAPI_BLE_COMPARE_BD_ADDR(DeviceInfo->IdentityAddressBD_ADDR, RemoteAddress)))
                        {
                            DeviceInfo->RemoteAddressType = AddressType;
                            DeviceInfo->RemoteAddress     = DeviceInfo->IdentityAddressBD_ADDR;

                            ret_val = DeviceInfo;
                            break;
                        }
                    }
                }
                else
                {
                    if ((AddressType == QAPI_BLE_LAT_RANDOM_E) && (QAPI_BLE_GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(RemoteAddress)))
                    {
                        if (DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
                        {
                            if (qapi_BLE_GAP_LE_Resolve_Address(BluetoothStackID, &(DeviceInfo->IRK), RemoteAddress))
                            {
                                DeviceInfo->RemoteAddress     = RemoteAddress;
                                DeviceInfo->RemoteAddressType = QAPI_BLE_LAT_RANDOM_E;

                                LOG_VERBOSE("\n");
                                LOG_VERBOSE("Resolved Address (");
                                BD_ADDRToStr(DeviceInfo->RemoteAddress, BoardStr);
                                LOG_VERBOSE("%s", BoardStr);
                                LOG_VERBOSE(")\n");
                                LOG_VERBOSE("   Identity Address:       ");
                                BD_ADDRToStr(DeviceInfo->IdentityAddressBD_ADDR, BoardStr);
                                LOG_VERBOSE("%s\n", BoardStr);
                                LOG_VERBOSE("   Identity Address Type:  %s\n", ((DeviceInfo->IdentityAddressType == QAPI_BLE_LAT_PUBLIC_IDENTITY_E) ? "Public Identity" : "Random Identity"));

                                ret_val = DeviceInfo;
                                break;
                            }
                        }
                    }
                }
            }

            DeviceInfo = DeviceInfo->NextDeviceInfoInfoPtr;
        }
    }

    return (ret_val);
}

/**
 * @func : ConfigureCapabilities
 * @Desc : This Function Configures for the Bonding(Pair) information
 */
static void ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities)
{
    /* Make sure the Capabilities pointer is semi-valid.                 */
    if(Capabilities)
    {
        /* Initialize the capabilities.                                   */
        memset(Capabilities, 0, QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE);

        /* Configure the Pairing Capabilities structure.                  */
        Capabilities->Bonding_Type                    = QAPI_BLE_LBT_NO_BONDING_E;
        Capabilities->IO_Capability                   = LE_Parameters.IOCapability;
        Capabilities->Flags                           = 0;

        if(LE_Parameters.MITMProtection)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED;

        if(LE_Parameters.SecureConnections)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

        if(RemoteOOBValid)
            Capabilities->Flags |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT;

        if(LocalOOBValid)
        {
            Capabilities->Flags                     |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_LOCAL_OOB_DATA_VALID;
            Capabilities->LocalOOBData.Flags         = 0;
            Capabilities->LocalOOBData.Confirmation  = LocalOOBConfirmation;
            Capabilities->LocalOOBData.Randomizer    = LocalOOBRandomizer;
        }

        /* ** NOTE ** This application always requests that we use the    */
        /*            maximum encryption because this feature is not a    */
        /*            very good one, if we set less than the maximum we   */
        /*            will internally in GAP generate a key of the        */
        /*            maximum size (we have to do it this way) and then   */
        /*            we will zero out how ever many of the MSBs          */
        /*            necessary to get the maximum size.  Also as a slave */
        /*            we will have to use Non-Volatile Memory (per device */
        /*            we are paired to) to store the negotiated Key Size. */
        /*            By requesting the maximum (and by not storing the   */
        /*            negotiated key size if less than the maximum) we    */
        /*            allow the slave to power cycle and regenerate the   */
        /*            LTK for each device it is paired to WITHOUT storing */
        /*            any information on the individual devices we are    */
        /*            paired to.                                          */
        Capabilities->Maximum_Encryption_Key_Size        = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

        /* This application only demonstrates using the Long Term Key's   */
        /* (LTK) for encryption of a LE Link and the Identity Resolving   */
        /* Key (IRK) for resolving resovable private addresses (RPA's),   */
        /* however we could request and send all possible keys here if we */
        /* wanted to.                                                     */
        Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
        Capabilities->Receiving_Keys.Identification_Key = TRUE;
        Capabilities->Receiving_Keys.Signing_Key        = FALSE;
        Capabilities->Receiving_Keys.Link_Key           = FALSE;

        Capabilities->Sending_Keys.Encryption_Key       = TRUE;
        Capabilities->Sending_Keys.Identification_Key   = TRUE;
        Capabilities->Sending_Keys.Signing_Key          = FALSE;
        Capabilities->Sending_Keys.Link_Key             = FALSE;
    }
}

/**
 * @func : SlavePairingRequestResponse
 * @Desc : Function Responsible to Send the Pairing response
 */
static int SlavePairingRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR)
{
    int                                                   ret_val;
    BoardStr_t                                            BoardStr;
    qapi_BLE_GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

    if (BluetoothStackID)
    {
        
        LOG_INFO("Sending slave pairing response\n");
        BD_ADDRToStr(BD_ADDR, BoardStr);
        LOG_INFO("Sending Pairing Response to %s.\n", BoardStr);

        AuthenticationResponseData.GAP_LE_Authentication_Type = QAPI_BLE_LAR_PAIRING_CAPABILITIES_E;
        AuthenticationResponseData.Authentication_Data_Length = QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE;
        //AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities.Bonding_Type = QAPI_BLE_LBT_NO_BONDING_E;
/* changes made */
        ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities));
        
        if ((ret_val = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData)) == QAPI_BLE_BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
        {
            LOG_WARN("Secure Connections not supported, disabling Secure Connections.\n");

            AuthenticationResponseData.Authentication_Data.Extended_Pairing_Capabilities.Flags &= ~QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

            ret_val = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData);
        }
        LOG_INFO("GAP_LE_Authentication_Response returned %d.\n", ret_val);
    }
    else
    {
        LOG_ERROR("Stack ID Invalid.\n");
        ret_val = -1;
    }

    return (ret_val);
}

/**
 * @func : DisplayLegacyPairingInformation
 * @Desc :  The following function displays the pairing capabilities that is
 *          passed into this function.
 */
static void DisplayLegacyPairingInformation(qapi_BLE_GAP_LE_Pairing_Capabilities_t *Pairing_Capabilities)
{
    switch (Pairing_Capabilities->IO_Capability)
    {
        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Display Only.\n");
            break;
        case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
            LOG_VERBOSE("   IO Capability:       Display Yes/No.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard Only.\n");
            break;
        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
            LOG_VERBOSE("   IO Capability:       No Input No Output.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard/Display.\n");
            break;
    }

    LOG_VERBOSE("   MITM:                %s.\n", (Pairing_Capabilities->MITM)?"TRUE":"FALSE");
    LOG_VERBOSE("   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
    LOG_VERBOSE("   OOB:                 %s.\n", (Pairing_Capabilities->OOB_Present)?"OOB":"OOB Not Present");
    LOG_VERBOSE("   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
    LOG_VERBOSE("   Sending Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("   Receiving Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
}

/**
 * @func : DisplayPairingInformation
 * @Desc : The following function displays the pairing capabilities that is
 *         passed into this function.
 */
static void DisplayPairingInformation(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Pairing_Capabilities)
{
    switch (Pairing_Capabilities->IO_Capability)
    {
        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Display Only.\n");
            break;
        case QAPI_BLE_LIC_DISPLAY_YES_NO_E:
            LOG_VERBOSE("   IO Capability:       Display Yes/No.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_ONLY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard Only.\n");
            break;
        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
            LOG_VERBOSE("   IO Capability:       No Input No Output.\n");
            break;
        case QAPI_BLE_LIC_KEYBOARD_DISPLAY_E:
            LOG_VERBOSE("   IO Capability:       Keyboard/Display.\n");
            break;
    }

    LOG_INFO("   Bonding Type:        %s.\n", (Pairing_Capabilities->Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding");
    LOG_INFO("   MITM:                %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_MITM_REQUESTED)?"TRUE":"FALSE");
    LOG_INFO("   Secure Connections:  %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS)?"TRUE":"FALSE");
    LOG_INFO("   OOB:                 %s.\n", (Pairing_Capabilities->Flags & QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_OOB_DATA_PRESENT)?"OOB":"OOB Not Present");
    LOG_INFO("   Encryption Key Size: %d.\n", Pairing_Capabilities->Maximum_Encryption_Key_Size);
    LOG_VERBOSE("   Sending Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Sending_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Sending_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("      Link Key:         %s.\n", ((Pairing_Capabilities->Sending_Keys.Link_Key)?"YES":"NO"));
    LOG_VERBOSE("   Receiving Keys: \n");
    LOG_VERBOSE("      LTK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Encryption_Key)?"YES":"NO"));
    LOG_VERBOSE("      IRK:              %s.\n", ((Pairing_Capabilities->Receiving_Keys.Identification_Key)?"YES":"NO"));
    LOG_VERBOSE("      CSRK:             %s.\n", ((Pairing_Capabilities->Receiving_Keys.Signing_Key)?"YES":"NO"));
    LOG_VERBOSE("      Link Key:         %s.\n", ((Pairing_Capabilities->Receiving_Keys.Link_Key)?"YES":"NO"));
}

/**
 * @func : GAP_LE_Event_Callback
 * @Desc : The following function is for the GAP LE Event Receive Data
 *         Callback.  This function will be called whenever a Callback has
 *         been registered for the specified GAP LE Action that is associated
 *         with the Bluetooth Stack.
 */
static void QAPI_BLE_BTPSAPI GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter)
{

    QCLI_Command_Status_t                                  status_persistent;
    boolean_t                                              DisplayPrompt;
    int                                                    Result;
    uint16_t                                               EDIV;
    BoardStr_t                                             BoardStr;
    unsigned int                                           Index;
    DeviceInfo_t                                          *DeviceInfo;
    DeviceInfo_t                                          *DeviceInfo_persistlist;
    qapi_BLE_Random_Number_t                               RandomNumber;
    qapi_BLE_Long_Term_Key_t                               GeneratedLTK;
    qapi_BLE_GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
    qapi_BLE_GAP_LE_Connection_Parameters_t                ConnectionParams;
    qapi_BLE_GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
    qapi_BLE_GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
    qapi_BLE_GAP_LE_Direct_Advertising_Report_Data_t      *DirectDeviceEntryPtr;
#ifdef V2
    qapi_BLE_GAP_LE_Extended_Advertising_Report_Data_t    *ExtDeviceEntryPtr;
#endif
    qapi_BLE_GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

    if((BluetoothStackID) && (GAP_LE_Event_Data))
    {
        DisplayPrompt = true;

        switch(GAP_LE_Event_Data->Event_Data_Type)
        {
#ifdef V2
            case QAPI_BLE_ET_LE_SCAN_TIMEOUT_E:
                LOG_WARN("etLE_Scan_Timeout with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                break;
            case QAPI_BLE_ET_LE_PHY_UPDATE_COMPLETE_E:
                LOG_INFO("etLE_PHY_Update_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_VERBOSE("  Status:  %d.\n", (int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->Status));
                LOG_VERBOSE("  Address: 0x%02X%02X%02X%02X%02X%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR5,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR4,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR3,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR2,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR1,
                        GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR0);
               // LOG_VERBOSE("  Tx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->TX_PHY));
                //LOG_VERBOSE("  Rx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->RX_PHY));

                break;
            case QAPI_BLE_ET_LE_ADVERTISING_SET_TERMINATED_E:
                LOG_INFO("etLE_Advertising_Set_Terminated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_VERBOSE("  Status:                                  %d.\n", (int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Status));
                LOG_VERBOSE("  Advertising Handle:                      %u.\n", (unsigned int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Advertising_Handle));
                LOG_VERBOSE("  Number of Completed Advertising Events:  %u.\n", (unsigned int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Num_Completed_Ext_Advertising_Events));

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_SUCCESS)
                {
                    switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Connection_Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Connection Address Type:                 %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Connection Address Type:                 Invalid.\n");
                            break;
                    }

                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Set_Terminated_Event_Data->Connection_Address, BoardStr);
                    LOG_VERBOSE("  Connection Address:                      %s.\n", BoardStr);
                }
                break;
            case QAPI_BLE_ET_LE_SCAN_REQUEST_RECEIVED_E:
                LOG_INFO("etLE_Scan_Request_Received with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                LOG_INFO("  Advertising Handle:          %d.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Advertising_Handle);

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Scanner_Address_Type)
                {
                    case QAPI_BLE_LAT_PUBLIC_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                        break;
                    case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                        LOG_VERBOSE("  Scanner Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                        break;
                    default:
                        LOG_VERBOSE("  Scanner Address Type:        Invalid.\n");
                        break;
                }

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Scan_Request_Received_Event_Data->Scanner_Address, BoardStr);
                LOG_VERBOSE("  Scanner Address:             %s.\n", BoardStr);
                break;
            case QAPI_BLE_ET_LE_CHANNEL_SELECTION_ALGORITHM_UPDATE_E:
                LOG_VERBOSE("etLE_Channel_Selection_Algorithm_Update with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Channel_Selection_Algorithm)
                {
                    case QAPI_BLE_SA_ALGORITHM_NUM1_E:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA #1");
                        break;
                    case QAPI_BLE_SA_ALGORITHM_NUM2_E:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA #2");
                        break;
                    default:
                        LOG_VERBOSE("  Channel Selection Algorithm:        %s.\n", "CSA Unkown");
                        break;
                }

                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address_Type)
                {
                    case QAPI_BLE_LAT_PUBLIC_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                        break;
                    case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                        LOG_VERBOSE("  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                        break;
                    default:
                        LOG_VERBOSE("  Connection Address Type:            Invalid.\n");
                        break;
                }

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address, BoardStr);
                LOG_VERBOSE("  Connection Address:                 %s.\n", BoardStr);
                break;
            case QAPI_BLE_ET_LE_EXTENDED_ADVERTISING_REPORT_E:
                LOG_VERBOSE("etLE_Extended_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    ExtDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Extended_Advertising_Report_Event_Data->Extended_Advertising_Data[Index]);

                    switch(ExtDeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_ANONYMOUS_E:
                            LOG_VERBOSE("  Address Type:     %s.\n", "Anonymous");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type:     Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address:          0x%02X%02X%02X%02X%02X%02X.\n", ExtDeviceEntryPtr->BD_ADDR.BD_ADDR5, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR4, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR3, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR2, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR1, ExtDeviceEntryPtr->BD_ADDR.BD_ADDR0);

                    if(DisplayAdvertisingEventData)
                    {
                        LOG_VERBOSE("  Event Type Flags: 0x%08X.\n", ExtDeviceEntryPtr->Event_Type_Flags);
                        LOG_VERBOSE("  Tx Power:         %d.\n", (int)ExtDeviceEntryPtr->Tx_Power);
                        LOG_VERBOSE("  RSSI:             %d.\n", (int)ExtDeviceEntryPtr->RSSI);
                        LOG_VERBOSE("  Advertising SID:  %d.\n", (int)ExtDeviceEntryPtr->Advertising_SID);
//                        LOG_VERBOSE("  Primary PHY:      %s.\n", PHYToString(ExtDeviceEntryPtr->Primary_PHY));

                        switch(ExtDeviceEntryPtr->Data_Status)
                        {
                            case QAPI_BLE_DS_COMPLETE_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Complete");
                                break;
                            case QAPI_BLE_DS_INCOMPLETE_DATA_PENDING_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Incomplete - More data pending");
                                break;
                            default:
                            case QAPI_BLE_DS_INCOMPLETE_DATA_TRUNCATED_E:
                                LOG_VERBOSE("  Data Status:      %s.\n", "Incomplete - data truncated");
                                break;
                        }

                        if(ExtDeviceEntryPtr->Event_Type_Flags & GAP_LE_EXTENDED_ADVERTISING_EVENT_TYPE_SECONDARY_PHY_VALID)
                      //      LOG_VERBOSE("  Secondary PHY:    %s.\n", PHYToString(ExtDeviceEntryPtr->Secondary_PHY));

                        LOG_VERBOSE("  Data Length:      %u.\n", (unsigned int)ExtDeviceEntryPtr->Raw_Report_Length);
                    }
                }
                break;
#endif

            case QAPI_BLE_ET_LE_DATA_LENGTH_CHANGE_E:
                LOG_VERBOSE("etLE_Data_Length_Change with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->BD_ADDR, BoardStr);
                LOG_VERBOSE("  Connection Address:                 %s.\n", BoardStr);
                LOG_VERBOSE("  Max Tx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets);
                LOG_VERBOSE("  Max Tx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxTime);
                LOG_VERBOSE("  Max Rx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets);
                LOG_VERBOSE("  Max Rx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxTime);
                break;
            case QAPI_BLE_ET_LE_ADVERTISING_REPORT_E:
                LOG_VERBOSE("etLE_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

                    switch(DeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type:        Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address: 0x%02X%02X%02X%02X%02X%02X.\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);

                    if(DisplayAdvertisingEventData)
                    {
                        switch(DeviceEntryPtr->Advertising_Report_Type)
                        {
                            case QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_CONNECTABLE_DIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_DIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCAN_RESPONSE_E:
                                LOG_VERBOSE("  Advertising Type: %s.\n", "QAPI_BLE_RT_SCAN_RESPONSE_E");
                                break;
                        }

                        LOG_VERBOSE("  RSSI: %d.\n", (int)(DeviceEntryPtr->RSSI));
                        LOG_VERBOSE("  Data Length: %d.\n", DeviceEntryPtr->Raw_Report_Length);
                    }
                }
                break;
            case QAPI_BLE_ET_LE_CONNECTION_COMPLETE_E:
                LOG_INFO("etLE_Connection_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

                    LOG_VERBOSE("   Status:              0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
                    LOG_VERBOSE("   Role:                %s.\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
                    switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("   Address Type:        Invalid.\n");
                            break;
                    }
                    LOG_VERBOSE("   BD_ADDR:             %s.\n", BoardStr);

                    if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_NO_ERROR)
                    {
                        LOG_VERBOSE("   Connection Interval: %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Connection_Interval);
                        LOG_VERBOSE("   Slave Latency:       %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Slave_Latency);

                        LocalDeviceIsMaster =  GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;
                        RemoteAddressType   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;

                        if((DeviceInfo = SearchDeviceInfoEntryTypeAddress(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) != NULL)
                        {
                            uint8_t            Peer_Identity_Address_Type;
                            uint8_t            StatusResult;
                            qapi_BLE_BD_ADDR_t Peer_Identity_Address;
                            qapi_BLE_BD_ADDR_t Local_Resolvable_Address;

                            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                                Peer_Identity_Address_Type = 0x00;
                            else if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_RANDOM_IDENTITY_E)
                                Peer_Identity_Address_Type = 0x01;
                            else
                                Peer_Identity_Address_Type = 0x02;

                            if(Peer_Identity_Address_Type != 0x02)
                            {
                                Peer_Identity_Address = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;

                                if(!qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(BluetoothStackID, Peer_Identity_Address_Type, Peer_Identity_Address, &StatusResult, &Local_Resolvable_Address))
                                {
                                    LOG_VERBOSE("   qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(): 0x%02X.\n", StatusResult);
                                    if(!StatusResult)
                                    {
                                        BD_ADDRToStr(Local_Resolvable_Address, BoardStr);
                                        LOG_VERBOSE("   Local RPA:           %s.\n", BoardStr);
                                    }
                                }
                            }

                            if(LocalDeviceIsMaster)
                            {
                                if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                {
                                    LOG_INFO("Attempting to Re-Establish Security.\n");

                                    GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                                    memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                                    GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                                    memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                                    GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                    Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                                    if(Result)
                                        LOG_INFO("GAP_LE_Reestablish_Security returned %d.\n", Result);
                                }
                                else
                                {
                                    LOG_ERROR("Can't re-establish security: LTK is missing.\n");
                                }
                            }
                        }
                    }
                }
                state_flags = state_flags | MOBILE_CONNECTED;
                break;
            case QAPI_BLE_ET_LE_DISCONNECTION_COMPLETE_E:
                LOG_INFO("etLE_Disconnection_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
                notify_Thread_Flag = 0;
                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
                {
                    LOG_VERBOSE("   Status: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
                    LOG_VERBOSE("   Reason: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
                    LOG_VERBOSE("   BD_ADDR: %s.\n", BoardStr);

                    SendInfo.BytesToSend = 0;
                    SendInfo.BytesSent   = 0;
                }
                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address)) != NULL){ 
                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_ENCRYPTED) {
                       DeviceInfo->Flags = DeviceInfo->Flags & (~DEVICE_INFO_FLAGS_ENCRYPTED);
                        LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                }
                     
                }
                
                state_flags = state_flags & ~MOBILE_CONNECTED;

                // To advertise after disconnection
                
                AdvertiseLE(1);
              
                break;
            case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATE_REQUEST_E:
                LOG_VERBOSE("etLE_Connection_Parameter_Update_Request with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
                    LOG_VERBOSE("   BD_ADDR:                     %s\n", BoardStr);
                    LOG_VERBOSE("   Connection Interval Minimum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min);
                    LOG_VERBOSE("   Connection Interval Maximum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max);
                    LOG_VERBOSE("   Slave Latency:               %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency);
                    LOG_VERBOSE("   Supervision Timeout:         %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout);

                    ConnectionParams.Connection_Interval_Min    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min;
                    ConnectionParams.Connection_Interval_Max    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max;
                    ConnectionParams.Slave_Latency              = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency;
                    ConnectionParams.Supervision_Timeout        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout;
                    ConnectionParams.Minimum_Connection_Length  = 0;
                    ConnectionParams.Maximum_Connection_Length  = 10000;

                    qapi_BLE_GAP_LE_Connection_Parameter_Update_Response(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, TRUE, &ConnectionParams);
                }
                break;
            case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATED_E:
                LOG_VERBOSE("etLE_Connection_Parameter_Updated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
                {
                    BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
                    LOG_VERBOSE("   BD_ADDR:             %s\n", BoardStr);
                    LOG_VERBOSE("   Status:              %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status);
                    LOG_VERBOSE("   Connection Interval: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval);
                    LOG_VERBOSE("   Slave Latency:       %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency);
                    LOG_VERBOSE("   Supervision Timeout: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout);
                }
                break;
            case QAPI_BLE_ET_LE_ENCRYPTION_CHANGE_E:
                LOG_VERBOSE("etLE_Encryption_Change with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
                    if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->BD_ADDR)) != NULL) { 
                        QCLI_Printf(ble_Group, "flags %02x\n", DeviceInfo->Flags);
                        DeviceInfo->Flags = DeviceInfo->Flags | DEVICE_INFO_FLAGS_ENCRYPTED;
                        QCLI_Printf(ble_Group, "flags %02x\n", DeviceInfo->Flags);
                        
                        if((GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data) != NULL) { 
                            BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->BD_ADDR, BoardStr);     
                            QCLI_Printf(ble_Group, "      BD_ADDR: %s. \n", BoardStr);
                        }
                    }
                    break;
            case QAPI_BLE_ET_LE_ENCRYPTION_REFRESH_COMPLETE_E:
                LOG_VERBOSE("etLE_Encryption_Refresh_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
                break;
            case QAPI_BLE_ET_LE_AUTHENTICATION_E:
                LOG_VERBOSE("etLE_Authentication with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

                if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
                {
                    BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

                    switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
                    {
                        case QAPI_BLE_LAT_LONG_TERM_KEY_REQUEST_E:
                            LOG_VERBOSE("    latKeyRequest: \n");
                            LOG_VERBOSE("      BD_ADDR: %s.\n", BoardStr);

                            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                            memset(&RandomNumber, 0, sizeof(RandomNumber));
                            EDIV = 0;

                            if((Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV == EDIV) && (QAPI_BLE_COMPARE_RANDOM_NUMBER(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand, RandomNumber)))
                            {
                                LOG_VERBOSE("inside 1\n");
                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                        LOG_VERBOSE("inside 3\n");
                                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                    {
                                        LOG_VERBOSE("inside 3\n");
                                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                        memcpy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), QAPI_BLE_LONG_TERM_KEY_SIZE);
                                    }
                                }
                            }
                            else
                            {
                                LOG_VERBOSE("inside 4\n");
                                Result = qapi_BLE_GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&DHK), (qapi_BLE_Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                                if(!Result)
                                {
                                    LOG_VERBOSE("      GAP_LE_Regenerate_Long_Term_Key Success.\n");

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                                }
                                else
                                {
                                    LOG_VERBOSE("      GAP_LE_Regenerate_Long_Term_Key returned %d.\n",Result);
                                }
                            }
                            LOG_VERBOSE("inside 5\n");

                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                            if(Result)
                            {
                                LOG_VERBOSE("      GAP_LE_Authentication_Response returned %d.\n",Result);
                            }
                            break;
                        case QAPI_BLE_LAT_SECURITY_REQUEST_E:
                            LOG_VERBOSE("    latSecurityRequest:.\n");
                            LOG_VERBOSE("      BD_ADDR: %s.\n", BoardStr);
                            LOG_VERBOSE("      Bonding Type: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding"));
                            LOG_VERBOSE("      MITM: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                    if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                                    {
                                        LOG_VERBOSE("Attempting to Re-Establish Security.\n");

                                        GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                                        memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                                        GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                                        memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                                        GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                                        Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, SecurityRemoteBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                                        if(Result)
                                        {
                                            LOG_VERBOSE("GAP_LE_Reestablish_Security returned %d.\n", Result);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_PAIRING_REQUEST_E:
                            LOG_INFO("Pairing Request: %s.\n", BoardStr);
                            DisplayLegacyPairingInformation(&Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);
                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;
                                SlavePairingRequestResponse(SecurityRemoteBD_ADDR);
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_EXTENDED_PAIRING_REQUEST_E:
                            LOG_INFO("Extended Pairing Request: %s.\n", BoardStr);
                            DisplayPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request));

                            if(QAPI_BLE_COMPARE_NULL_BD_ADDR(SecurityRemoteBD_ADDR))
                            {
                                SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;
                                SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                            }
                            else
                            {
                                LOG_WARN("\nSecurity is already in progress with another remote device.\n");

                                GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                                GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_CONFIRMATION_REQUEST_E:
                            LOG_VERBOSE("latConfirmationRequest.\n");

                            switch(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type)
                            {
                                case QAPI_BLE_CRT_NONE_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    switch(LE_Parameters.IOCapability)
                                    {
                                        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                            LOG_VERBOSE("Invoking Just Works.\n");

                                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                            if(Result)
                                            {
                                                LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                            }
                                            break;
                                        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                        default:
                                            LOG_INFO("Confirmation of Pairing.\n");

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_PASSKEY_E:
                                    LOG_VERBOSE("Call LEPasskeyResponse [PASSCODE].\n");
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_E:
                                    LOG_INFO("Passkey: %06u.\n", (unsigned int)(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                                    break;
                                default:
                                    LOG_VERBOSE("Authentication method not supported.\n");
                                    break;
                            }
                            break;
                        case QAPI_BLE_LAT_EXTENDED_CONFIRMATION_REQUEST_E:
                            LOG_VERBOSE("latExtendedConfirmationRequest.\n");

                            LOG_VERBOSE("   Secure Connections:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
                            LOG_VERBOSE("   Just Works Pairing:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");
                            LOG_VERBOSE("   Keypress Notifications: %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_KEYPRESS_NOTIFICATIONS_REQUESTED)?"YES":"NO");

                            switch(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Request_Type)
                            {
                                case QAPI_BLE_CRT_NONE_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    switch(LE_Parameters.IOCapability)
                                    {
                                        case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                            LOG_VERBOSE("Invoking Just Works.\n");

                                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                            if(Result)
                                            {
                                                LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                            }
                                            break;
                                        case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                        default:
                                            LOG_VERBOSE("Confirmation of Pairing.\n");

                                            if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                            break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_PASSKEY_E:
                                    LOG_VERBOSE("Call LEPasskeyResponse [PASSKEY].\n");
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_E:
                                    LOG_INFO("Passkey: %06u.\n", Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = QAPI_BLE_LAR_PASSKEY_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (uint8_t)(sizeof(uint32_t));
                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey;

                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                        DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                                case QAPI_BLE_CRT_DISPLAY_YES_NO_E:
                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                                    if(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)
                                    {
                                        switch(LE_Parameters.IOCapability)
                                        {
                                            case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                                LOG_VERBOSE("Invoking Just Works.\n");

                                                Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                                if(Result)
                                                {
                                                    LOG_VERBOSE("qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                                }
                                                break;
                                            case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                                LOG_VERBOSE("Confirmation of Pairing.\n");

                                                GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                                break;
                                            default:
                                                LOG_VERBOSE("Confirmation of Pairing.\n");

                                                if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                                break;
                                        }
                                    }
                                    else
                                    {
                                        LOG_INFO("Confirmation Value: %ld\n", (unsigned long)Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                                        if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                            DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                        break;
                                    }
                                    break;
                                case QAPI_BLE_CRT_OOB_SECURE_CONNECTIONS_E:
                                    LOG_VERBOSE("OOB Data Request.\n");

                                    GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_OUT_OF_BAND_DATA_E;
                                    GAP_LE_Authentication_Response_Information.Authentication_Data_Length = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_SIZE;
                                    if(RemoteOOBValid)
                                    {
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags        = 0;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Confirmation = RemoteOOBConfirmation;
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Randomizer   = RemoteOOBRandomizer;
                                    }
                                    else
                                        GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_FLAGS_OOB_NOT_RECEIVED;

                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                        DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                                default:
                                    LOG_ERROR("Authentication method not supported.\n");
                                    break;
                            }
                            break;
                        case QAPI_BLE_LAT_SECURITY_ESTABLISHMENT_COMPLETE_E:
                            LOG_VERBOSE("Security Re-Establishment Complete: %s.\n", BoardStr);
                            LOG_VERBOSE("                            Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);

                            if(Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status == QAPI_BLE_GAP_LE_SECURITY_ESTABLISHMENT_STATUS_CODE_LONG_TERM_KEY_ERROR)
                            {
                                if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                                {
                                    DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                                    LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                                }
                            }

                            QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                            break;
                        case QAPI_BLE_LAT_PAIRING_STATUS_E:

                            LOG_VERBOSE("Pairing Status w.r.t mobile");
                            LOG_VERBOSE("Pairing Status: %s.\n", BoardStr);
                            LOG_VERBOSE("        Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                            if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == QAPI_BLE_GAP_LE_PAIRING_STATUS_NO_ERROR)
                            {
                                LOG_VERBOSE("        Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);

#if 1 
                                if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,Authentication_Event_Data->BD_ADDR)) != NULL) {
                                    DeviceInfo -> device_type = DEVICE_TYPE_MOBILE;
                                        if ((DeviceInfo_persistlist = SearchDeviceInfoEntryByType(&PersistDeviceInfoList,DEVICE_TYPE_MOBILE)) != NULL) { 
                                                memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t));
                                        }  else {

                                            if((DeviceInfo_persistlist = CreateNewDeviceInfoEntry(&PersistDeviceInfoList, Authentication_Event_Data->BD_ADDR )) != NULL)                    
                                            {

                                                memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t));
                                                QCLI_Printf(ble_Group, "Device entry created in the persist info list\n");

                                            }

                                        }

                                }

                                QCLI_Printf(ble_Group," Before : call for store persist data w.r.t mobile ");
                                status_persistent = StorePersistentData();
                                
                                QCLI_Printf(ble_Group," After : call for store persist data w.r.t mobile ");
                                QCLI_Printf(ble_Group,"status of persistent storage after mobile device conn: %d\n",status_persistent );
#endif
                            }
                            else
                            {
                                qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                            }

                            QAPI_BLE_ASSIGN_BD_ADDR(SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                            break;
                        case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_REQUEST_E:
                            LOG_VERBOSE("Encryption Information Request %s.\n", BoardStr);
                            break;
                        case QAPI_BLE_LAT_IDENTITY_INFORMATION_REQUEST_E:
                            LOG_VERBOSE("Identity Information Request %s.\n", BoardStr);

                            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = QAPI_BLE_LAR_IDENTITY_INFORMATION_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = (uint8_t)QAPI_BLE_GAP_LE_IDENTITY_INFORMATION_DATA_SIZE;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address      = LocalBD_ADDR;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address_Type = QAPI_BLE_LAT_PUBLIC_IDENTITY_E;
                            GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.IRK          = IRK;

                            Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                            if(!Result)
                            {
                                LOG_INFO("   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
                            }
                            else
                            {
                                LOG_ERROR("   Error - SM_Generate_Long_Term_Key returned %d.\n", Result);
                            }
                            break;
                        case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_E:
                            LOG_VERBOSE(" Encryption Information from RemoteDevice: %s.\n", BoardStr);
                            LOG_VERBOSE("                             Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size);

                            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                            {
                                memcpy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), sizeof(DeviceInfo->LTK));
                                DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                                memcpy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), sizeof(DeviceInfo->Rand));
                                DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                                DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                                LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                            }
                            else
                            {
                                LOG_ERROR("No Key Info Entry for this device.\n");
                            }
                            break;
                        case QAPI_BLE_LAT_IDENTITY_INFORMATION_E:
                            LOG_VERBOSE(" Identity Information from RemoteDevice: %s.\n", BoardStr);

                            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                            {
                                memcpy(&(DeviceInfo->IRK), &(Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK), sizeof(DeviceInfo->IRK));
                                DeviceInfo->IdentityAddressBD_ADDR = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                                DeviceInfo->IdentityAddressType    = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                                DeviceInfo->Flags                 |= DEVICE_INFO_FLAGS_IRK_VALID;
                                LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);

                                DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                                DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                                DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                                DeviceInfo->ResolvingListEntry.Local_IRK                  = IRK;
                            }
                            else
                            {
                                LOG_ERROR("No Key Info Entry for this device.\n");
                            }
                            break;
                        default:
                            LOG_VERBOSE("Unhandled event: %u\n", Authentication_Event_Data->GAP_LE_Authentication_Event_Type);
                            break;
                    }
                }
                break;
            case QAPI_BLE_ET_LE_DIRECT_ADVERTISING_REPORT_E:
                LOG_INFO("etLE_Direct_Advertising_Report with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
                LOG_VERBOSE("  %d Responses.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries);

                for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
                {
                    DirectDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Direct_Advertising_Data[Index]);

                    switch(DirectDeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            LOG_VERBOSE("  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            LOG_VERBOSE("  Address Type: Invalid.\n");
                            break;
                    }

                    LOG_VERBOSE("  Address:      0x%02X%02X%02X%02X%02X%02X.\n", DirectDeviceEntryPtr->BD_ADDR.BD_ADDR5, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR4, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR3, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR2, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR1, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR0);
                    LOG_VERBOSE("  RSSI:         %d.\n", (int)(DirectDeviceEntryPtr->RSSI));
                }
                break;
            default:
                DisplayPrompt = false;
                break;
        }

        if(DisplayPrompt)
            QCLI_Display_Prompt();
    }
}

/**
 * @func : AdvertiseLE
 * @Desc : The following function is responsible for enabling/disabling LE
 *         Advertisements
 */
int AdvertiseLE(uint32_t enable)
{
    int                                         Result;
    int                                         ret_val;
    unsigned int                                Length;
    unsigned int                                UUIDIndex;
    char                                        Name[QAPI_BLE_GAP_MAXIMUM_DEVICE_NAME_LENGTH+1];
    unsigned int                                NameLength;
    uint16_t                                    DeviceAppearance;
    qapi_BLE_GAP_LE_Connectability_Parameters_t ConnectabilityParameters;
    union
    {
        qapi_BLE_Advertising_Data_t              AdvertisingData;
        qapi_BLE_Scan_Response_Data_t            ScanResponseData;
    } Advertisement_Data_Buffer;


    /* First, check that valid Bluetooth Stack ID exists.                */
    if (BluetoothStackID)
    {
        /* Make sure that all of the parameters required for this function*/
        /* appear to be at least semi-valid.                              */
#ifndef V1
        if ((enable >= AET_DISABLE_E) && (enable <= AET_ENABLE_CHANNEL_39_E))
#else
            if ((enable >= 0) && (enable <= 1))
#endif
            {
                /* Determine whether to enable or disable Advertising.         */
#ifndef V1
                if (enable == 0)
#else
                    if (enable == AET_DISABLE_E)
#endif
                    {
                        /* Disable Advertising.                                     */
                        Result = qapi_BLE_GAP_LE_Advertising_Disable(BluetoothStackID);
                        if(!Result)
                        {
                            LOG_INFO("   GAP_LE_Advertising_Disable success.\n");

                            ret_val = QCLI_STATUS_SUCCESS_E;
                        }
                        else
                        {
                            LOG_ERROR("   GAP_LE_Advertising_Disable returned %d.\n", Result);

                            ret_val = QCLI_STATUS_ERROR_E;
                        }
                    }
                    else
                    {
                        /* Set the Advertising Data.                                */
                        memset(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(qapi_BLE_Advertising_Data_t));

                        /* Reset the length to zero.                                */
                        Length = 0;

                        /* Set the Flags A/D Field (1 byte type and 1 byte Flags.   */
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length]   = 2;
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2] = 0;

                        /* Configure the flags field based on the Discoverability   */
                        /* Mode.                                                    */
                        if(LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_GENERAL_DISCOVERABLE_MODE_E)
                            Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2]    = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                        else
                        {
                            if(LE_Parameters.DiscoverabilityMode == QAPI_BLE_DM_LIMITED_DISCOVERABLE_MODE_E)
                                Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length+2] = QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                        }

                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length + 2] |= QAPI_BLE_HCI_LE_ADVERTISING_FLAGS_BR_EDR_NOT_SUPPORTED_FLAGS_BIT_MASK;

                        /* Update the current length of the advertising data.       */
                        /* * NOTE * We MUST add one to account for the length field,*/
                        /*          which does not include itself.                  */
                        Length += (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[Length] + 1);

                        /* Include the GAPS Device Appearance.                      */
                        if((Result = qapi_BLE_GAPS_Query_Device_Appearance(BluetoothStackID, (uint32_t)GAPSInstanceID, &DeviceAppearance)) == 0)
                        {
                            /* Make sure we do not exceed the bounds of the buffer.  */
                            if((Length + (unsigned int)QAPI_BLE_NON_ALIGNED_WORD_SIZE + 2) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                            {
                                ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length]   = 3;
                                ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
                                ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+2]), DeviceAppearance);

                                /* Update the current length of the advertising data. */
                                /* * NOTE * We MUST add one to account for the length */
                                /*          field, which does not include itself.     */
                                Length += (((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1);
                            }
                            else
                                LOG_WARN("   The device appearance CANNOT fit in the advertising data.\n", Result);
                        }
                        else
                            LOG_ERROR("   qapi_BLE_GAPS_Query_Device_Appearance(dtAdvertising) returned %d.\n", Result);

                        /* Include the 16-bit service UUIDs if the service is       */
                        /* registered.                                              */
                        /* * NOTE * SPPLE is excluded since it has a 128 bit UUID.  */

                        /* Make sure we have room in the buffer.                    */
                        /* * NOTE * We will make sure we have room for at least one */
                        /*          16-bit UUID.                                    */
                        if((Length + (unsigned int)QAPI_BLE_UUID_16_SIZE + 2) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                        {
                            ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length]   = 1;
                            ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length+1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;

                            /* Store the UUID Index location.                        */
                            /* * NOTE * We will do this to make the code more        */
                            /*          readable.                                    */
                            UUIDIndex = (Length + 2);

                            /* If DIS is registered.                                 */
                            if(DISInstanceID)
                            {
                                /* Make sure we do not exceed the bounds of the       */
                                /* buffer.                                            */
                                /* * NOTE * We MUST add one to account for the length */
                                /*          field, which does not include itself.     */
                                if((Length + ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1 + QAPI_BLE_UUID_16_SIZE) <= (unsigned int)QAPI_BLE_ADVERTISING_DATA_MAXIMUM_SIZE)
                                {
                                    /* Assign the DIS Service UUID.                    */
                                    QAPI_BLE_DIS_ASSIGN_DIS_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));

                                    /* Update the UUID Index.                          */
                                    UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                    /* Update the advertising report data entry length */
                                    /* since we have added another UUID.               */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
#ifdef OFFLINE
#define QAPI_BLE_DIS_ASSIGN_OFFLINE_SERVICE_UUID_16(_x)                           QAPI_BLE_ASSIGN_BLUETOOTH_UUID_16(*((qapi_BLE_UUID_16_t *)(_x)), 0x18, 0x18)               

                                    QAPI_BLE_DIS_ASSIGN_OFFLINE_SERVICE_UUID_16(&(((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[UUIDIndex]));

                                    /* Update the UUID Index.                          */
                                    UUIDIndex += QAPI_BLE_UUID_16_SIZE;

                                    /* Update the advertising report data entry length */
                                    /* since we have added another UUID.               */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] += QAPI_BLE_UUID_16_SIZE;
#endif

                                }
                                else
{
                                    /* Flag that we could not include all the service  */
                                    /* UUIDs.                                          */
                                    ((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_PARTIAL;
                                }
                            }

                            /* Update the current length of the advertising data.    */
                            /* * NOTE * We MUST add one to account for the length    */
                            /*          field, which does not include itself.        */
                            Length += (((uint8_t *)&Advertisement_Data_Buffer.AdvertisingData)[Length] + 1);
                        }
                        else
                            LOG_WARN("   The 16-bit service UUID's CANNOT fit in the advertising data.\n");

                        /* Write the advertising data to the chip.                  */
                        Result = qapi_BLE_GAP_LE_Set_Advertising_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.AdvertisingData));
                        if(!Result)
                        {
                            /* Initialize the scan response data.                    */
                            memset(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(qapi_BLE_Scan_Response_Data_t));

                            /* Reset the length to zero.                             */
                            Length = 0;

                            /* First include the local device name.                  */
                            if((Result = qapi_BLE_GAPS_Query_Device_Name(BluetoothStackID, (uint32_t)GAPSInstanceID, Name)) == 0)
                            {
                                /* Get the name length.                               */
                                NameLength = strlen(Name);

                                /* Determine if we need to truncate the device name.  */
                                if(NameLength < ((unsigned int)QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2))
                                {
                                    /* Format the complete device name into the scan   */
                                    /* response data.                                  */
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] = (uint8_t)(1 + NameLength);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
                                }
                                else
                                {
                                    /* Format the truncated device name into the scan  */
                                    /* response data.                                  */
                                    NameLength = ((unsigned int)QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] = (uint8_t)(1 + NameLength);
                                    ((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 1] = QAPI_BLE_HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                                }

                                /* Make sure we can fit the device name into the scan */
                                /* response data.                                     */
                                if((Length + NameLength + 2) <= (unsigned int)(QAPI_BLE_SCAN_RESPONSE_DATA_MAXIMUM_SIZE - 2))
                                {
                                    /* Simply copy the name into the scan response     */
                                    /* data.                                           */
                                    memcpy(&(((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length + 2]), Name, NameLength);

                                    /* Update the current length of the scan response  */
                                    /* data.                                           */
                                    /* * NOTE * We MUST add one to account for the     */
                                    /*          length field, which does not include   */
                                    /*          itself.                                */
                                    Length += (((uint8_t *)&Advertisement_Data_Buffer.ScanResponseData)[Length] + 1);
                                }
                                else
                                    LOG_WARN("   The device name CANNOT fit in the scan response data.\n", Result);
                            }
                            else
                                LOG_ERROR("   qapi_BLE_GAPS_Query_Device_Name(dtAdvertising) returned %d.\n", Result);

                            Result = qapi_BLE_GAP_LE_Set_Scan_Response_Data(BluetoothStackID, Length, &(Advertisement_Data_Buffer.ScanResponseData));
                            if(!Result)
                            {
                                /* Configure the advertising channel map for the      */
                                /* default advertising parameters.                    */
#ifndef V1
                                BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
#else
                                switch(enable)
                                {
                                    case AET_ENABLE_ALL_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = (QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37 | QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38);
                                        break;
                                    case AET_ENABLE_CHANNEL_37_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37;
                                        break;
                                    case AET_ENABLE_CHANNEL_38_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38;
                                        break;
                                    case AET_ENABLE_CHANNEL_39_E:
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_39;
                                        break;
                                    default:
                                        /* Enable all channels if the user specified and*/
                                        /* invalid enumeration.                         */
                                        BLEParameters.AdvertisingParameters.Advertising_Channel_Map = (QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_37 | QAPI_BLE_HCI_LE_ADVERTISING_CHANNEL_MAP_ENABLE_CHANNEL_38);
                                        break;
                                }
#endif

                                /* Set up the default advertising parameters if they  */
                                /* have not been configured at the CLI.               */
                                if(!(BLEParameters.Flags & BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID))
                                {
                                    /* Configure the remaining default advertising     */
                                    /* parameters.                                     */
                                    BLEParameters.AdvertisingParameters.Scan_Request_Filter       = QAPI_BLE_FP_NO_FILTER_E;
                                    BLEParameters.AdvertisingParameters.Connect_Request_Filter    = QAPI_BLE_FP_NO_FILTER_E;
                                    BLEParameters.AdvertisingParameters.Advertising_Interval_Min  = 400;
                                    BLEParameters.AdvertisingParameters.Advertising_Interval_Max  = 600;

                                    /* Flag that the parameters are valid so we don't  */
                                    /* set them unnecessarily.                         */
                                    BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID;
                                }

                                /* Configure the Connectability Parameters.           */
                                /* * NOTE * We will ALWAYS advertise                  */
                                ConnectabilityParameters.Connectability_Mode = LE_Parameters.ConnectableMode;
                                ConnectabilityParameters.Own_Address_Type    = QAPI_BLE_LAT_PUBLIC_E;

                                /* If the connectable mode is set for direct          */
                                /* connectable.                                       */
                                if((ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_LOW_DUTY_CYCLE_DIRECT_CONNECTABLE_E) || (ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_DIRECT_CONNECTABLE_E))
                                {
                                    /* We will set our own address type to resolvable  */
                                    /* fallback public.                                */
                                    ConnectabilityParameters.Own_Address_Type = QAPI_BLE_LAT_RESOLVABLE_FALLBACK_PUBLIC_E;
                                }

                                /* Initialize the direct address to zero and the type */
                                /* to public.                                         */
                                /* * NOTE * If the ConnectableMode is set to one of   */
                                /*          the Direct Connectable types, then the    */
                                /*          direct address and type MUST be specified.*/
                                /*          If they are NOT specified, then           */
                                /*          qapi_BLE_GAP_LE_Advertising_Enable() will */
                                /*          fail.                                     */
                                ConnectabilityParameters.Direct_Address_Type   = QAPI_BLE_LAT_PUBLIC_E;
                                QAPI_BLE_ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);


                                /* If the user did NOT supply the direct address,  */
                                /* then we need to make sure we are NOT in the a   */
                                /* direct connectable mode.                        */
                                if((ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_LOW_DUTY_CYCLE_DIRECT_CONNECTABLE_E) || (ConnectabilityParameters.Connectability_Mode == QAPI_BLE_LCM_DIRECT_CONNECTABLE_E))
                                {
                                    /* Override the local device's connectable mode.*/
                                    /* * NOTE * If this is NOT done, then           */
                                    /*          qapi_BLE_GAP_LE_Advertising_Enable()*/
                                    /*          will fail.                          */
                                    ConnectabilityParameters.Own_Address_Type    = QAPI_BLE_LAT_PUBLIC_E;
                                    ConnectabilityParameters.Connectability_Mode = QAPI_BLE_LCM_CONNECTABLE_E;

                                    /* Inform the user.                             */
                                    LOG_WARN("Using connectable un-directed advertising with public address.\n");
                                }

                                /* Now enable advertising.                         */
                                Result = qapi_BLE_GAP_LE_Advertising_Enable(BluetoothStackID, TRUE, &(BLEParameters.AdvertisingParameters), &(ConnectabilityParameters), GAP_LE_Event_Callback, 0);
                                if(!Result)
                                {
                                    LOG_INFO("   GAP_LE_Advertising_Enable success, Advertising Interval Range: %u - %u.\n", (unsigned int)BLEParameters.AdvertisingParameters.Advertising_Interval_Min, (unsigned int)BLEParameters.AdvertisingParameters.Advertising_Interval_Max);

                                    ret_val = QCLI_STATUS_SUCCESS_E;
                                }
                                else
                                {
                                    LOG_ERROR("   GAP_LE_Advertising_Enable returned %d.\n", Result);

                                    ret_val = QCLI_STATUS_ERROR_E;
                                }
                            }
                            else
                            {
                                LOG_ERROR("   qapi_BLE_GAP_LE_Set_Advertising_Data(dtScanResponse) returned %d.\n", Result);

                                ret_val = QCLI_STATUS_ERROR_E;
                            }

                        }
                        else
                        {
                            LOG_ERROR("   qapi_BLE_GAP_LE_Set_Advertising_Data(dtAdvertising) returned %d.\n", Result);

                            ret_val = QCLI_STATUS_ERROR_E;
                        }
                    }
            }
            else
            {
                ret_val = QCLI_STATUS_USAGE_E;
            }
    }
    else
        ret_val = QCLI_STATUS_ERROR_E;

    return(ret_val);
}

/**
 * @func : GetBLEStackID
 * @Desc : The following function is responsible for enabling/disabling LE
 *         Advertisements
 */
uint32_t GetBluetoothStackID()
{
    return BluetoothStackID;
}

/**
 * @func : ble_get_device_mac_address
 * @Desc : gets the device mac address
 */
int32_t ble_get_device_mac_address(uint8_t *mac)
{
    qapi_BLE_BD_ADDR_t Bdaddr;
    uint32_t i;

    if (qapi_BLE_GAP_Query_Local_BD_ADDR(BluetoothStackID, &Bdaddr))
        return -1;

    for (i = 0; i < 6; i++)
        *(mac + i) = *((uint8_t *) &Bdaddr + (5 - i));
    return QCLI_STATUS_SUCCESS_E;
}

 const QCLI_Command_t ble_cmd_list[] =                                           
  { };

const QCLI_Command_Group_t ble_cmd_group =                                      
 {                                                                               
     "BLE",                                                                      
     (sizeof(ble_cmd_list) / sizeof(ble_cmd_list[0])),                           
    ble_cmd_list                                                                
}; 


int bt_Stk_Id()
 {                                                                               
     return BluetoothStackID;                                                     
 } 




/************************************************** Central Role STart ***********************************************/

void BLBDWriteData(uint32_t val)
{
   int attr_len;
   int Result;
   DeviceInfo_t *DeviceInfo;
   attr_len = 4;
  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,Current_Conn_Addr)))
      {
	

         if(bulb_char_handle)
         {

            QCLI_Printf(ble_Group, "Found device\n\n");
            if((Result = qapi_BLE_GATT_Write_Without_Response_Request(BluetoothStackID, 
               DeviceInfo->ConnectionID, bulb_char_handle, attr_len,
               &val)) > 0)
            {
               QCLI_Printf(ble_Group, "BLBDWriteData write success = %u", Result);
            }
            else if (Result == QAPI_BLE_GATT_ERROR_INVALID_CONNECTION_ID)
            {
                  
               DisplayFunctionError("qapi_BLE_GATT_Write_Request", Result);


            }
            else
            {
               DisplayFunctionError("qapi_BLE_GATT_Write_Request", Result);
               QCLI_Printf(ble_Group, "Conn_id = %d", DeviceInfo->ConnectionID);
            }
         }
      
      
               QCLI_Printf(ble_Group, "coudnt find *********************\n\n");
      }

}



   /* The following function is provided to properly print a UUID.      */
static void DisplayUUID(qapi_BLE_GATT_UUID_t *UUID)
{
   if(UUID)
   {
      if(UUID->UUID_Type == QAPI_BLE_GU_UUID_16_E)
         QCLI_Printf(ble_Group, "%02X%02X\n", UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0);
      else
      {
         if(UUID->UUID_Type == QAPI_BLE_GU_UUID_128_E)
         {
            QCLI_Printf(ble_Group, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte12, UUID->UUID.UUID_128.UUID_Byte11, UUID->UUID.UUID_128.UUID_Byte10,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte9,  UUID->UUID.UUID_128.UUID_Byte8,  UUID->UUID.UUID_128.UUID_Byte7,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte6,  UUID->UUID.UUID_128.UUID_Byte5,  UUID->UUID.UUID_128.UUID_Byte4,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte3,  UUID->UUID.UUID_128.UUID_Byte2,  UUID->UUID.UUID_128.UUID_Byte1,
                                                                                                    UUID->UUID.UUID_128.UUID_Byte0);
         }
      }
   }
}




int ConnectLEDevice(uint32_t BluetoothStackID, boolean_t UseWhiteList, qapi_BLE_BD_ADDR_t *BD_ADDR, qapi_BLE_GAP_LE_Address_Type_t AddressType)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      /* Check to see if we need to configure the default Scan          */
      /* Parameters.                                                    */
      if(!(BLEParameters.Flags & BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID))
      {
         /* Configure the default Scan Window and Scan Interval.        */
         BLEParameters.ScanParameters.ScanWindow   = 50;
         BLEParameters.ScanParameters.ScanInterval = 100;

         /* Flag that the scan parameters are valid so that we do not   */
         /* re-configure the defaults un-necessarily.                   */
         BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID;
      }

      /* Check to see if we need to configure the default Connection    */
      /* Parameters.                                                    */
      if(!(BLEParameters.Flags & BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID))
      {
         /* Initialize the default connection parameters.               */
         BLEParameters.ConnectionParameters.Connection_Interval_Min    = 50;
         BLEParameters.ConnectionParameters.Connection_Interval_Max    = 200;
         BLEParameters.ConnectionParameters.Minimum_Connection_Length  = 0;
         BLEParameters.ConnectionParameters.Maximum_Connection_Length  = 10000;
         BLEParameters.ConnectionParameters.Slave_Latency              = 0;
         BLEParameters.ConnectionParameters.Supervision_Timeout        = 6000;

         /* Flag that the connection parameters are valid so that we do */
         /* not re-configure the defaults un-necessarily.               */
         BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID;
      }

      /* Everything appears correct, go ahead and attempt to make the   */
      /* connection.                                                    */
      /* * NOTE * Our local address type will ALWAYS be resolvable      */
      /*          fallback public, in case the Resolving List in the    */
      /*          controller is used for connecting.  It is worth noting*/
      /*          that this demo uses its local/public address as the   */
      /*          public identity address for simplicity.               */
      Result = qapi_BLE_GAP_LE_Create_Connection(BluetoothStackID, BLEParameters.ScanParameters.ScanInterval, BLEParameters.ScanParameters.ScanWindow, (UseWhiteList ? QAPI_BLE_FP_WHITE_LIST_E : QAPI_BLE_FP_NO_FILTER_E), AddressType, BD_ADDR, QAPI_BLE_LAT_RESOLVABLE_FALLBACK_PUBLIC_E, &(BLEParameters.ConnectionParameters), BLBD_GAP_LE_Event_Callback, 0);
      if(!Result)
      {
         QCLI_Printf(ble_Group, "Connection Request successful.\n");
         QCLI_Printf(ble_Group, "Scan Parameters:       Window %u, Interval %u.\n", (unsigned int)BLEParameters.ScanParameters.ScanWindow,
                                                                                     (unsigned int)BLEParameters.ScanParameters.ScanInterval);
         QCLI_Printf(ble_Group, "Connection Parameters: Interval Range %u - %u, Slave Latency %u.\n", (unsigned int)BLEParameters.ConnectionParameters.Connection_Interval_Min,
                                                                                                       (unsigned int)BLEParameters.ConnectionParameters.Connection_Interval_Max,
                                                                                                       (unsigned int)BLEParameters.ConnectionParameters.Slave_Latency);
         QCLI_Printf(ble_Group, "Using White List:      %s.\n", (UseWhiteList ? "Yes" : "No"));
      }
      else
      {
         /* Unable to create connection.                                */
         QCLI_Printf(ble_Group, "Unable to create connection: %d.\n", Result);
      }
   }
   else
      Result = -1;

   return(Result);
}






char *GetLocalNameInAD(qapi_BLE_GAP_LE_Advertising_Data_t *ad_ptr)
{

   int i;
   char *str = 0;

   for(i = 0; i < ad_ptr->Number_Data_Entries; i++)
   {
      if(ad_ptr->Data_Entries[i].AD_Type == AD_TYPE_LOCAL_NAME)
      {
         str = malloc(ad_ptr->Data_Entries[i].AD_Data_Length + 1);
         strncpy(str, (char *)ad_ptr->Data_Entries[i].AD_Data_Buffer, 
            ad_ptr->Data_Entries[i].AD_Data_Length);
         str[ad_ptr->Data_Entries[i].AD_Data_Length] = 0;

      }
   }

   return str;

}

char BD_Addr_Blb[16];



int blbd_start_scan()
{
   int res;
    QCLI_Printf(ble_Group, "inside blbd_start_scan \n");

    res = MscStartScan(BluetoothStackID, QAPI_BLE_FP_NO_FILTER_E, 3);
    if(!res)
    QCLI_Printf(ble_Group, "Scan started successfully\n");
   
    return res ;
}

  /* The following function is responsible for stopping on on-going    */
   /* scan.                                                             */
static int StopScan(uint32_t BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      /* if scan timer is active stop that.                             */
      if(ScanTimerID)
      {
         /* Stop the timer.                                             */
         qapi_BLE_BSC_StopTimer(BluetoothStackID, ScanTimerID);

         ScanTimerID = 0;
      }

      /* Stop the scan.                                                 */
      Result = qapi_BLE_GAP_LE_Cancel_Scan(BluetoothStackID);
      if(!Result)
      {
         QCLI_Printf(ble_Group, "Scan stopped successfully.\n");

         /* Flag that scanning is not in progess.                       */
         ScanInProgress = FALSE;
      }
      else
         QCLI_Printf(ble_Group, "Unable to stop scan: %d\n", Result);
   }
   else
      Result = -1;

   return(Result);
}


void BSC_Timer_Callback(uint32_t BluetoothStackID, uint32_t TimerID, uint32_t CallbackParameter)
{
   /* Verify the input parameters.                                      */
   if(BluetoothStackID)
   {
      QCLI_Printf(ble_Group, "Stopping scan after scanning for %u seconds.\n", CallbackParameter);

      /* Clear the Scan Timer ID.                                       */
      ScanTimerID = 0;

      /* Stop scanning.                                                 */
      StopScan(BluetoothStackID);
#ifdef HOME_AUTOMATION
    
      //BLBDClearTempScanData();
      blb_num_bulbs_found = 0;
      //blbd_scan_permitted = 1;

#endif

   }
}


void Conn_Timer_Callback(uint32_t BluetoothStackID, uint32_t TimerID, uint32_t CallbackParameter)
{


    QCLI_Printf(ble_Group, "In Conn_Timer_Callback\n");
    int ConnResult;
   /* Verify the input parameters.                                      */
   if(BluetoothStackID)
   {
      QCLI_Printf(ble_Group, "Cancel the connection,if no response is received  within %u seconds.\n", CallbackParameter);
      ConnResult = qapi_BLE_GAP_LE_Cancel_Create_Connection(BluetoothStackID);

      if(!ConnResult)
          QCLI_Printf(ble_Group, "cancel Create connection successfull\n");
      else
          QCLI_Printf(ble_Group, "Failed to cancel the existing connection\n");

      //WHY?????????????????????
        //bulb_State = BULB_STATE_DISCONNECTED ;
   
   
   }



}

/* The following function is responsible for starting a scan.        */
int MscStartScan(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, unsigned int ScanDuration)
{
   int Result = 0;

   QCLI_Printf(ble_Group, "Msc scan enter \n");

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      QCLI_Printf(ble_Group, "Msc valid bt stack id %d\n", BluetoothStackID);

      /* Check to see if we need to configure the default Scan          */
      /* Parameters.                                                    */
      if(!(BLEParameters.Flags & BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID))
      {
         /* Configure the default Scan Window and Scan Interval.        */
         BLEParameters.ScanParameters.ScanWindow   = 50;
         BLEParameters.ScanParameters.ScanInterval = 100;

         /* Flag that the scan parameters are valid so that we do not   */
         /* re-configure the defaults un-necessarily.                   */
         BLEParameters.Flags |= BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID;
      }

      QCLI_Printf(ble_Group, "Msc before scan duration \n");
#if 1
      /* See if we should start a timer for this scan.                  */
      if(ScanDuration)
      {
         /* Start a timer for this operation.                           */
         Result = qapi_BLE_BSC_StartTimer(BluetoothStackID, (ScanDuration * 1000), BSC_Timer_Callback, ScanDuration);
         if(Result > 0)
         {
            /* Save the scan timer ID.                                  */
            ScanTimerID = (unsigned int)Result;

            Result      = 0;
         }
      }
      else
         Result = 0;
#endif

      /* Continue if no error occurred.                                 */
      if(!Result)
      {
         /* Not currently scanning, go ahead and attempt to perform the */
         /* scan.                                                       */
         Result = qapi_BLE_GAP_LE_Perform_Scan(BluetoothStackID, QAPI_BLE_ST_ACTIVE_E, BLEParameters.ScanParameters.ScanInterval, BLEParameters.ScanParameters.ScanWindow, QAPI_BLE_LAT_PUBLIC_E, FilterPolicy, TRUE, BLBD_GAP_LE_Event_Callback, 0);
         if(!Result)
            QCLI_Printf(ble_Group, "Msc scan started successfully. Scan Window: %u, Scan Interval: %u.\n", (unsigned int)BLEParameters.ScanParameters.ScanWindow, (unsigned int)BLEParameters.ScanParameters.ScanInterval);
         else
            QCLI_Printf(ble_Group, "Msc unable to perform scan: %d\n", Result);
      }
      else
         QCLI_Printf(ble_Group, "Msc unable to start scan timer: %d\n", Result);
   }
   else
      Result = -1;

   return(Result);
}


static uint32_t cnt = 0; 
void timer_callback()
{


    if((state_flags & SCAN_PERMITTED) && (state_flags & MOBILE_CONNECTED))
    {
        //scan at an interval of 20 secs
        switch (cnt%6)
        {
            case 0:
                //fall through if scan is not needed
#if 1    
            blbd_start_scan();  
#endif
                break;
            default:
                break;
        }

    }
    if (!(cnt%15)) {
        /*TODO: Send Smoke sensor notifiation*/
        notify_Smoke_Data();
        cnt = 0;
    }
    /*Send notification for BULB	*/
    notify_Bulb_State();
    cnt++;
}



void BLE_timer_init()
{

  QCLI_Printf(ble_Group, "in timer_init\n");
  BLBD_Create_Timer_Attr.deferrable     = false;
  BLBD_Create_Timer_Attr.cb_type        = QAPI_TIMER_FUNC1_CB_TYPE;
  BLBD_Create_Timer_Attr.sigs_func_ptr  = timer_callback; 
  BLBD_Create_Timer_Attr.sigs_mask_data = BLBD_PERIODIC_TIMER_SIGNAL_INTR;
  qapi_Timer_Def(&(PeriodicScanTimer), &BLBD_Create_Timer_Attr);

  /* Start the timer for periodic transmissions.     */
  BLBD_Set_Timer_Attr.time                   = 2000;
  BLBD_Set_Timer_Attr.reload                 = true;
  BLBD_Set_Timer_Attr.max_deferrable_timeout = 0; 
  BLBD_Set_Timer_Attr.unit                   = QAPI_TIMER_UNIT_MSEC;
  qapi_Timer_Set(PeriodicScanTimer, &BLBD_Set_Timer_Attr);

  QCLI_Printf(ble_Group, "BLBD Thread started\n");
  return;
}


static void BLBD_ConfigureCapabilities(qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t *Capabilities)
{
    /* Make sure the Capabilities pointer is semi-valid.                 */
    if(Capabilities)
    {
        /* Initialize the capabilities.                                   */
        memset(Capabilities, 0, QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_SIZE);

        /* Configure the Pairing Capabilities structure.                  */
        Capabilities->Bonding_Type                    = QAPI_BLE_LBT_BONDING_E;
        Capabilities->IO_Capability                   = QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E;
        Capabilities->Flags                           = 0;

        if(0)
        {
            Capabilities->Flags                     |= QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_LOCAL_OOB_DATA_VALID;
            Capabilities->LocalOOBData.Flags         = 0;
            Capabilities->LocalOOBData.Confirmation  = LocalOOBConfirmation;
            Capabilities->LocalOOBData.Randomizer    = LocalOOBRandomizer;
        }

        /* ** NOTE ** This application always requests that we use the    */
        /*            maximum encryption because this feature is not a    */
        /*            very good one, if we set less than the maximum we   */
        /*            will internally in GAP generate a key of the        */
        /*            maximum size (we have to do it this way) and then   */
        /*            we will zero out how ever many of the MSBs          */
        /*            necessary to get the maximum size.  Also as a slave */
        /*            we will have to use Non-Volatile Memory (per device */
        /*            we are paired to) to store the negotiated Key Size. */
        /*            By requesting the maximum (and by not storing the   */
        /*            negotiated key size if less than the maximum) we    */
        /*            allow the slave to power cycle and regenerate the   */
        /*            LTK for each device it is paired to WITHOUT storing */
        /*            any information on the individual devices we are    */
        /*            paired to.                                          */
        Capabilities->Maximum_Encryption_Key_Size        = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

        /* This application only demonstrates using the Long Term Key's   */
        /* (LTK) for encryption of a LE Link and the Identity Resolving   */
        /* Key (IRK) for resolving resovable private addresses (RPA's),   */
        /* however we could request and send all possible keys here if we */
        /* wanted to.                                                     */
        Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
        Capabilities->Receiving_Keys.Identification_Key = TRUE;
        Capabilities->Receiving_Keys.Signing_Key        = FALSE;
        Capabilities->Receiving_Keys.Link_Key           = FALSE;

        Capabilities->Sending_Keys.Encryption_Key       = TRUE;
        Capabilities->Sending_Keys.Identification_Key   = TRUE;
        Capabilities->Sending_Keys.Signing_Key          = FALSE;
        Capabilities->Sending_Keys.Link_Key             = FALSE;
    }
}


   /* The following function provides a mechanism for sending a pairing */
   /* request to a device that is connected on an LE Link.              */
static int SendPairingRequest(qapi_BLE_BD_ADDR_t BD_ADDR, boolean_t ConnectionMaster)
{
   int                                             ret_val;
   BoardStr_t                                      BoardStr;
   qapi_BLE_GAP_LE_Extended_Pairing_Capabilities_t ExtendedCapabilities;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Configure the application pairing parameters.                  */
      BLBD_ConfigureCapabilities(&ExtendedCapabilities);

      /* Inform the user we are attempting to pair to the remote device.*/
      BD_ADDRToStr(BD_ADDR, BoardStr);
      QCLI_Printf(ble_Group, "Attempting to Pair to %s.\n", BoardStr);

      DisplayPairingInformation(&ExtendedCapabilities);

      /* Attempt to pair to the remote device.                          */
      if(ConnectionMaster)
      {
         /* Go ahead and store the address of the remote device that we */
         /* are going to attempt to pair.                               */
         /* * NOTE * If this function is called by the                  */
         /*          GAP_LE_Event_Callback() then this will already be  */
         /*          set, however if it is called by PairLE(), then it  */
         /*          will NOT be set.  We MUST only set this if we are  */
         /*          the master of the connection since the master sends*/
         /*          the pairing request.  The slave will set this when */
         /*          the pairing request has been received.             */
         BLBD_SecurityRemoteBD_ADDR = BD_ADDR;

         /* Start the pairing process.                                  */
         if((ret_val = qapi_BLE_GAP_LE_Extended_Pair_Remote_Device(BluetoothStackID, BLBD_SecurityRemoteBD_ADDR, &ExtendedCapabilities, BLBD_GAP_LE_Event_Callback, 0)) == QAPI_BLE_BTPS_ERROR_SECURE_CONNECTIONS_NOT_SUPPORTED)
         {
            /* Since Secure Connections isn't supported go ahead and    */
            /* disable our request for Secure Connections and re-submit */
            /* our request.                                             */
            QCLI_Printf(ble_Group, "Secure Connections not supported, disabling Secure Connections.\n");

            ExtendedCapabilities.Flags &= ~QAPI_BLE_GAP_LE_EXTENDED_PAIRING_CAPABILITIES_FLAGS_SECURE_CONNECTIONS;

            /* Try this again.                                          */
            ret_val = qapi_BLE_GAP_LE_Extended_Pair_Remote_Device(BluetoothStackID, BLBD_SecurityRemoteBD_ADDR, &ExtendedCapabilities, BLBD_GAP_LE_Event_Callback, 0);
         }

         QCLI_Printf(ble_Group, "     GAP_LE_Extended_Pair_Remote_Device returned %d.\n", ret_val);
      }
      else
      {
         /* As a slave we can only request that the Master start the    */
         /* pairing process.                                            */
         ret_val = qapi_BLE_GAP_LE_Extended_Request_Security(BluetoothStackID, BD_ADDR, &ExtendedCapabilities, BLBD_GAP_LE_Event_Callback, 0);

         QCLI_Printf(ble_Group, "     GAP_LE_Request_Security returned %d.\n", ret_val);
      }
   }
   else
   {
      QCLI_Printf(ble_Group, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}


   /* The following function is a utility function that provides a      */
   /* mechanism of populating a BLBD Client Information structure with  */
   /* the information discovered from a BLBD Discovery operation.       */
   /* * NOTE * We will only store characteristc attribute handles that  */
   /*          are supported by this demo.                              */
static void BLBDPopulateHandles(AIOP_Client_Information_t *ClientInfo, 
   qapi_BLE_GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData, DeviceInfo_t *DeviceInfo)
{
   unsigned int                                           Index;
   qapi_BLE_GATT_Characteristic_Information_t            *CharacteristicInfoPtr;
   // BLBD_DEVICE_CHARS                          *InstanceInfoPtr;

   QCLI_Printf(ble_Group, "Inside BLBDPopulateHandles\n");

   //InstanceInfoPtr = blbd_devices[SelectedBLBDDevice].dev_chars;
   //InstanceInfoPtr =  malloc(sizeof(BLBD_DEVICE_CHARS) * 3);

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceDiscoveryData))
   {
     /* Loop through all characteristics discovered in the service and  */
     /* populate the correct entry.                                     */
     QCLI_Printf(ble_Group, "Inside BLBDPopulateHandles matched service uuid\n");

     CharacteristicInfoPtr = ServiceDiscoveryData->CharacteristicInformationList;
     if(CharacteristicInfoPtr)
     {
        //InstanceInfoPtr =  malloc(sizeof(BLBD_DEVICE_CHARS) * ServiceDiscoveryData->NumberOfCharacteristics);
        // InstanceInfoPtr =  malloc(sizeof(BLBD_DEVICE_CHARS) * 1);
        /* Let's loop through the AIOS Characteristic and store their   */
        /* information.                                                 */
        for(Index = 0; Index < ServiceDiscoveryData->NumberOfCharacteristics; Index++)
        {
            //InstanceInfoPtr[Index].Valid =  TRUE;
            /* Store the properties. */
            if(QAPI_BLE_BLBD_COMPARE_CHAR_UUID_TO_UUID_16(CharacteristicInfoPtr[Index].Characteristic_UUID.UUID.UUID_16 )){
                //InstanceInfoPtr[0].Properties = CharacteristicInfoPtr[Index].Characteristic_Properties;
                QCLI_Printf(ble_Group, "Found get/set colour charecteristic\n");
                /* Store the attribute handle information for    */
                /* this Digital Characteristic.                  */
                //InstanceInfoPtr[0].Characteristic_Handle = CharacteristicInfoPtr[Index].Characteristic_Handle;
                bulb_char_handle = CharacteristicInfoPtr[Index].Characteristic_Handle;
                BLBDWriteData(0xFFFFFFFF);
                bulb_State = BULB_STATE_ON;
                DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                QCLI_Printf(ble_Group, "   Handle:        0x%04X\n", CharacteristicInfoPtr[Index].Characteristic_Handle);
                QCLI_Printf(ble_Group, "   Properties:    0x%02X\n", CharacteristicInfoPtr[Index].Characteristic_Properties);
                QCLI_Printf(ble_Group, "   UUID:          0x");
                DisplayUUID(&(CharacteristicInfoPtr[Index].Characteristic_UUID));
                break;

            }
        }

     }
  }

}


static void QAPI_BLE_BTPSAPI GATT_BLBD_Service_Discovery_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, uint32_t CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;
   QCLI_Printf(ble_Group,"Discovred1");
   if((BluetoothStackID) && (GATT_Service_Discovery_Event_Data))
   {
 QCLI_Printf(ble_Group,"Discovred2");
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Current_Conn_Addr)) != NULL)
      {
 QCLI_Printf(ble_Group,"Discovred3");
         switch(GATT_Service_Discovery_Event_Data->Event_Data_Type)
         {
            case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_INDICATION_E:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data)
               {
                  QCLI_Printf(ble_Group, "Service 0x%04X - 0x%04X, UUID: ",
                     GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.Service_Handle, 
                     GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.End_Group_Handle);
                  DisplayUUID(&(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.UUID));
                  QCLI_Printf(ble_Group, "\n");

                  /* Attempt to populate BLBD handles.                  */
                  BLBDPopulateHandles(&(DeviceInfo->AIOPClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data , DeviceInfo);

               }
               break;
            case QAPI_BLE_ET_GATT_SERVICE_DISCOVERY_COMPLETE_E:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data)
               {
                  QCLI_Printf(ble_Group, "Service Discovery Operation Complete, Status 0x%02X.\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status);

                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
                  LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                  
                  //blbd_service_discovery_callback();
               }
               break;
            default:
               break;
         }
      }

   }
}




/* Generic Attribute Profile (GATT) QCLI command functions.          */

   /* The following function is responsible for performing a GATT       */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static QCLI_Command_Status_t DiscoverBLBDServices(qapi_BLE_BD_ADDR_t BD_ADDR)
{
   int                                     Result;
   DeviceInfo_t                           *DeviceInfo;
   QCLI_Command_Status_t                   ret_val;

      /* Lock the Bluetooth stack.                                      */
      if(!qapi_BLE_BSC_LockBluetoothStack(BluetoothStackID))
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, 
            BD_ADDR)) != NULL)
         {
            QCLI_Printf(ble_Group, "inside DiscoverBLBDServices \n");


            /* Verify that no service discovery is outstanding for this */
            /* device.                                                  */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
            {
               /* Start the service discovery process.                  */
               Result = qapi_BLE_GATT_Start_Service_Discovery(BluetoothStackID, DeviceInfo->ConnectionID, 0, NULL, 
                  GATT_BLBD_Service_Discovery_Event_Callback, 0);

               if(!Result)
               {
                  /* Display success message.                           */
                  QCLI_Printf(ble_Group, "qapi_BLE_GATT_Service_Discovery_Start() success.\n");

                  /* Flag that a Service Discovery Operation is         */
                  /* outstanding.                                       */
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
                  LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);

                  ret_val = QCLI_STATUS_SUCCESS_E;
               }
               else
               {
                  /* An error occur so just clean-up.                   */
                  QCLI_Printf(ble_Group, "Error - BLBD GATT_Service_Discovery_Start returned %d.\n", Result);
                  ret_val = QCLI_STATUS_ERROR_E;
               }
            }
            else
            {
//               QCLI_Printf(ble_Group, "BLBD Service Discovery Operation Outstanding for Device. %d\n", deviceIndex);

               ret_val = QCLI_STATUS_ERROR_E;
            }
         }
         else
         {
            QCLI_Printf(ble_Group, "No Device Info.\n");

            ret_val = QCLI_STATUS_ERROR_E;
         }

         /* Un-lock the Bluetooth Stack.                                */
         qapi_BLE_BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
      {
         QCLI_Printf(ble_Group, "Unable to acquire Bluetooth Stack Lock.\n");

         ret_val = QCLI_STATUS_ERROR_E;
      }
   return(ret_val);
}



   /* The following function is provided to allow a mechanism of        */
   /* responding to a request for Encryption Information to send to a   */
   /* remote device.                                                    */
static int EncryptionInformationRequestResponse(qapi_BLE_BD_ADDR_t BD_ADDR, uint8_t KeySize, qapi_BLE_GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
   int      ret_val;
   uint16_t LocalDiv;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the input parameters are semi-valid.                 */
      if((!QAPI_BLE_COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information))
      {
         QCLI_Printf(ble_Group, "   Calling GAP_LE_Generate_Long_Term_Key.\n");

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = qapi_BLE_GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&DHK), (qapi_BLE_Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            QCLI_Printf(ble_Group, "   Encryption Information Request Response.\n");

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = QAPI_BLE_LAR_ENCRYPTION_INFORMATION_E;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = QAPI_BLE_GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               QCLI_Printf(ble_Group, "   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
            }
            else
            {
               QCLI_Printf(ble_Group, "   Error - SM_Generate_Long_Term_Key returned %d.\n", ret_val);
            }
         }
         else
         {
            QCLI_Printf(ble_Group, "   Error - SM_Generate_Long_Term_Key returned %d.\n", ret_val);
         }
      }
      else
      {
         QCLI_Printf(ble_Group, "Invalid Parameters.\n");

         ret_val = -1;
      }
   }
   else
   {
      QCLI_Printf(ble_Group, "Stack ID Invalid.\n");

      ret_val = -1;
   }

   return(ret_val);
}



static void QAPI_BLE_BTPSAPI BLBD_GAP_LE_Event_Callback(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Event_Data_t *GAP_LE_Event_Data, uint32_t CallbackParameter)
{
   boolean_t                                              DisplayPrompt;
   int                                                    Result;
   uint16_t                                               EDIV;
   BoardStr_t                                             BoardStr;
   unsigned int                                           Index;
   DeviceInfo_t                                          *DeviceInfo;
   DeviceInfo_t                                          *DeviceInfo_persistlist;
   qapi_BLE_Random_Number_t                               RandomNumber;
   qapi_BLE_Long_Term_Key_t                               GeneratedLTK;
   qapi_BLE_GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
   qapi_BLE_GAP_LE_Connection_Parameters_t                ConnectionParams;
   qapi_BLE_GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
   qapi_BLE_GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
   qapi_BLE_GAP_LE_Direct_Advertising_Report_Data_t      *DirectDeviceEntryPtr;
   qapi_BLE_GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;
   QCLI_Command_Status_t                                  status;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      DisplayPrompt = true;

      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
#ifdef V2
         case QAPI_BLE_ET_LE_SCAN_TIMEOUT_E:
            QCLI_Printf(ble_Group, "etLE_Scan_Timeout with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            break;
         case QAPI_BLE_ET_LE_PHY_UPDATE_COMPLETE_E:
            QCLI_Printf(ble_Group, "etLE_PHY_Update_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            QCLI_Printf(ble_Group, "  Status:  %d.\n", (int)(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->Status));
            QCLI_Printf(ble_Group, "  Address: 0x%02X%02X%02X%02X%02X%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR5,
                                                                               GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR4,
                                                                               GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR3,
                                                                               GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR2,
                                                                               GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR1,
                                                                               GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->BD_ADDR.BD_ADDR0);
           // QCLI_Printf(ble_Group, "  Tx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->TX_PHY));
          //  QCLI_Printf(ble_Group, "  Rx PHY:  %s.\n", PHYToString(GAP_LE_Event_Data->Event_Data.GAP_LE_Phy_Update_Complete_Event_Data->RX_PHY));

            break;
       
       
         case QAPI_BLE_ET_LE_CHANNEL_SELECTION_ALGORITHM_UPDATE_E:
            QCLI_Printf(ble_Group, "etLE_Channel_Selection_Algorithm_Update with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Display the new CSA.                                     */
            switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Channel_Selection_Algorithm)
            {
               case QAPI_BLE_SA_ALGORITHM_NUM1_E:
                  QCLI_Printf(ble_Group, "  Channel Selection Algorithm:        %s.\n", "CSA #1");
                  break;
               case QAPI_BLE_SA_ALGORITHM_NUM2_E:
                  QCLI_Printf(ble_Group, "  Channel Selection Algorithm:        %s.\n", "CSA #2");
                  break;
               default:
                  QCLI_Printf(ble_Group, "  Channel Selection Algorithm:        %s.\n", "CSA Unkown");
                  break;
            }

            /* Display the Address Type.                                */
            switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address_Type)
            {
               case QAPI_BLE_LAT_PUBLIC_E:
                  QCLI_Printf(ble_Group, "  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                  break;
               case QAPI_BLE_LAT_RANDOM_E:
                  QCLI_Printf(ble_Group, "  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                  break;
               case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                  QCLI_Printf(ble_Group, "  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                  break;
               case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                  QCLI_Printf(ble_Group, "  Connection Address Type:            %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                  break;
               default:
                  QCLI_Printf(ble_Group, "  Connection Address Type:            Invalid.\n");
                  break;
            }

            BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Channel_Selection_Algorithm_Update_Event_Data->Connection_Address, BoardStr);
            QCLI_Printf(ble_Group, "  Connection Address:                 %s.\n", BoardStr);
            break;
 
#endif
         case QAPI_BLE_ET_LE_DATA_LENGTH_CHANGE_E:
            QCLI_Printf(ble_Group, "etLE_Data_Length_Change with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->BD_ADDR, BoardStr);
            QCLI_Printf(ble_Group, "  Connection Address:                 %s.\n", BoardStr);
            QCLI_Printf(ble_Group, "  Max Tx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxOctets);
            QCLI_Printf(ble_Group, "  Max Tx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxTxTime);
            QCLI_Printf(ble_Group, "  Max Rx Octets:                      %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxOctets);
            QCLI_Printf(ble_Group, "  Max Rx Time:                        %u.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Data_Length_Change_Event_Data->MaxRxTime);
            break;


         case QAPI_BLE_ET_LE_ADVERTISING_REPORT_E:
            //QCLI_Printf(ble_Group, "etLE_Advertising_Report with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            //QCLI_Printf(ble_Group, "  %d Responses.\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
                DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

#ifdef HOME_AUTOMATION

                char *str;
                str = GetLocalNameInAD(&(DeviceEntryPtr->Advertising_Data));

                if(!strncmp(str, BLBD_DEVICE_SIGNATURE,8))
                {
                    QCLI_Printf(ble_Group, "BLBD Print - device found %s, Connecting\n", str);
                    /*Call connect API*/
                    StopScan(BluetoothStackID);
                    ConnectLEDevice(BluetoothStackID, FALSE, &(DeviceEntryPtr->BD_ADDR),DeviceEntryPtr->Address_Type);
#endif

                    /* Display the Address Type.                             */
                    switch(DeviceEntryPtr->Address_Type)
                    {
                        case QAPI_BLE_LAT_PUBLIC_E:
                            QCLI_Printf(ble_Group, "  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_E:
                            QCLI_Printf(ble_Group, "  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                            break;
                        case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                            QCLI_Printf(ble_Group, "  Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                            break;
                        case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                            QCLI_Printf(ble_Group, "  Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                            break;
                        default:
                            QCLI_Printf(ble_Group, "  Address Type:        Invalid.\n");
                            break;
                    }

                    /* Display the Device Address.                           */
                    QCLI_Printf(ble_Group, "  Address: 0x%02X%02X%02X%02X%02X%02X.\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0);

                    if(DisplayAdvertisingEventData)
                    {
                        /* Display the packet type for the device             */
                        switch(DeviceEntryPtr->Advertising_Report_Type)
                        {
                            case QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E:
                                QCLI_Printf(ble_Group, "  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_CONNECTABLE_DIRECTED_E:
                                QCLI_Printf(ble_Group, "  Advertising Type: %s.\n", "QAPI_BLE_RT_CONNECTABLE_DIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E:
                                QCLI_Printf(ble_Group, "  Advertising Type: %s.\n", "QAPI_BLE_RT_SCANNABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E:
                                QCLI_Printf(ble_Group, "  Advertising Type: %s.\n", "QAPI_BLE_RT_NON_CONNECTABLE_UNDIRECTED_E");
                                break;
                            case QAPI_BLE_RT_SCAN_RESPONSE_E:
                                QCLI_Printf(ble_Group, "  Advertising Type: %s.\n", "QAPI_BLE_RT_SCAN_RESPONSE_E");
                                break;
                        }

                        QCLI_Printf(ble_Group, "  RSSI: %d.\n", (int)(DeviceEntryPtr->RSSI));
                        QCLI_Printf(ble_Group, "  Data Length: %d.\n", DeviceEntryPtr->Raw_Report_Length);
                    } 
                }
            }
            break;
         case QAPI_BLE_ET_LE_CONNECTION_COMPLETE_E:
            QCLI_Printf(ble_Group, "etLE_Connection_Complete with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
                BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

                QCLI_Printf(ble_Group, "   Status:              0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status);
                QCLI_Printf(ble_Group, "   Role:                %s.\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave");
                switch(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type)
                {
                    case QAPI_BLE_LAT_PUBLIC_E:
                        QCLI_Printf(ble_Group, "   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_E:
                        QCLI_Printf(ble_Group, "   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                        break;
                    case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                        QCLI_Printf(ble_Group, "   Address Type:        %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                        break;
                    case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                        QCLI_Printf(ble_Group, "   Address Type:        %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                        break;
                    default:
                        QCLI_Printf(ble_Group, "   Address Type:        Invalid.\n");
                        break;
                }
                QCLI_Printf(ble_Group, "   BD_ADDR:             %s.\n", BoardStr);

                if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == QAPI_BLE_HCI_ERROR_CODE_NO_ERROR)
                {
                    QCLI_Printf(ble_Group, "   Connection Interval: %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Connection_Interval);
                    QCLI_Printf(ble_Group, "   Slave Latency:       %u.\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Slave_Latency);

                    /* Store the GAP LE Connection information that needs */
                    /* to be stored for the remote device.                */
                    /* * NOTE * These are temporary globals that will hold*/
                    /*          information that needs to be stored for   */
                    /*          the remote device that just connected.    */
                    /* * NOTE * For consistency, this information will NOT*/
                    /*          be stored until the GATT Connection       */
                    /*          Complete event has been received.  This   */
                    /*          event ALWAYS follows the GAP LE Connection*/
                    /*          Complete event.                           */
                    LocalDeviceIsMaster =  GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;
                    RemoteAddressType   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;

                    /* Attempt to find the entry via the address and      */
                    /* address type.  This function will update the entry */
                    /* if it is found to already exist and resolution is  */
                    /* now being done in the controller.                  */
                    if((DeviceInfo = SearchDeviceInfoEntryTypeAddress(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) != NULL)
                    {
                        uint8_t            Peer_Identity_Address_Type;
                        uint8_t            StatusResult;
                        qapi_BLE_BD_ADDR_t Peer_Identity_Address;
                        qapi_BLE_BD_ADDR_t Local_Resolvable_Address;

                        if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_PUBLIC_IDENTITY_E)
                            Peer_Identity_Address_Type = 0x00;
                        else if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == QAPI_BLE_LAT_RANDOM_IDENTITY_E)
                            Peer_Identity_Address_Type = 0x01;
                        else
                            Peer_Identity_Address_Type = 0x02;

                        if(Peer_Identity_Address_Type != 0x02)
                        {
                            Peer_Identity_Address = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;

                            if(!qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(BluetoothStackID, Peer_Identity_Address_Type, Peer_Identity_Address, &StatusResult, &Local_Resolvable_Address))
                            {
                                QCLI_Printf(ble_Group, "   qapi_BLE_HCI_LE_Read_Local_Resolvable_Address(): 0x%02X.\n", StatusResult);
                                if(!StatusResult)
                                {
                                    BD_ADDRToStr(Local_Resolvable_Address, BoardStr);
                                    QCLI_Printf(ble_Group, "   Local RPA:           %s.\n", BoardStr);
                                }
                            }
                        }


                    }

                    memcpy(&Current_Conn_Addr,&(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address),sizeof(qapi_BLE_BD_ADDR_t));
                    /*Initiate Pairing */
                    QCLI_Printf(ble_Group, "Device is master initiating the pairing request \n" );
                    if(!SendPairingRequest(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, TRUE))
                        QCLI_Printf(ble_Group, "pairing request sent successfully \n" );
                    else                                                  
                        QCLI_Printf(ble_Group, "Failed to send the pairing request \n" );

                    qapi_BLE_BSC_StopTimer(BluetoothStackID, ConnTimerID);
                    QCLI_Printf(ble_Group, "Connection timer stopped \n" );
                    //bulb_State = BULB_STATE_CONNECTED;

                    blbd_scan_permitted = 0;
                    state_flags &= ~SCAN_PERMITTED;
                    bulb_State =  BULB_STATE_ON;
                }
            }

            break;

         case QAPI_BLE_ET_LE_DISCONNECTION_COMPLETE_E:
            QCLI_Printf(ble_Group, "etLE_Disconnection_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            
            
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               QCLI_Printf(ble_Group, "   Status: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status);
               QCLI_Printf(ble_Group, "   Reason: 0x%02X.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason);

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               QCLI_Printf(ble_Group, "   BD_ADDR: %s.\n", BoardStr);
               /* Clear the Send Information.                           */
               SendInfo.BytesToSend = 0;
               SendInfo.BytesSent   = 0;
            }
                blbd_scan_permitted = 1;
                state_flags = state_flags | SCAN_PERMITTED;
            bulb_State = BULB_STATE_DISCONNECTED;
	        memset(&Current_Conn_Addr,0,sizeof(qapi_BLE_BD_ADDR_t));
            break;
         case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATE_REQUEST_E:
            QCLI_Printf(ble_Group, "etLE_Connection_Parameter_Update_Request with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
               QCLI_Printf(ble_Group, "   BD_ADDR:                     %s\n", BoardStr);
               QCLI_Printf(ble_Group, "   Connection Interval Minimum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min);
               QCLI_Printf(ble_Group, "   Connection Interval Maximum: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max);
               QCLI_Printf(ble_Group, "   Slave Latency:               %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency);
               QCLI_Printf(ble_Group, "   Supervision Timeout:         %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout);

               ConnectionParams.Connection_Interval_Min    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min;
               ConnectionParams.Connection_Interval_Max    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max;
               ConnectionParams.Slave_Latency              = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency;
               ConnectionParams.Supervision_Timeout        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout;
               ConnectionParams.Minimum_Connection_Length  = 0;
               ConnectionParams.Maximum_Connection_Length  = 10000;

               qapi_BLE_GAP_LE_Connection_Parameter_Update_Response(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, TRUE, &ConnectionParams);
            }
            break;
         case QAPI_BLE_ET_LE_CONNECTION_PARAMETER_UPDATED_E:
            QCLI_Printf(ble_Group, "etLE_Connection_Parameter_Updated with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
               QCLI_Printf(ble_Group, "   BD_ADDR:             %s\n", BoardStr);
               QCLI_Printf(ble_Group, "   Status:              %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status);
               QCLI_Printf(ble_Group, "   Connection Interval: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval);
               QCLI_Printf(ble_Group, "   Slave Latency:       %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency);
               QCLI_Printf(ble_Group, "   Supervision Timeout: %d\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout);
            }
            break;
         case QAPI_BLE_ET_LE_ENCRYPTION_CHANGE_E:
            QCLI_Printf(ble_Group, "etLE_Encryption_Change with size %d.\n",(int)GAP_LE_Event_Data->Event_Data_Size);
            break;
         case QAPI_BLE_ET_LE_ENCRYPTION_REFRESH_COMPLETE_E:
            QCLI_Printf(ble_Group, "etLE_Encryption_Refresh_Complete with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            break;
         case QAPI_BLE_ET_LE_AUTHENTICATION_E:
            QCLI_Printf(ble_Group, "etLE_Authentication with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case QAPI_BLE_LAT_LONG_TERM_KEY_REQUEST_E:
                     QCLI_Printf(ble_Group, "    latKeyRequest: \n");
                     QCLI_Printf(ble_Group, "      BD_ADDR: %s.\n", BoardStr);

                     /* Initialize the authentication response data to  */
                     /* indicate no LTK present (if we find or          */
                     /* re-generate the LTK we will update this         */
                     /* structure accordingly).                         */
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                     /* Initialize some variables to determine if this  */
                     /* is a request for a Long Term Key generated via  */
                     /* Secure Connections (which we must store and can */
                     /* NOT re-generate).                               */
                     memset(&RandomNumber, 0, sizeof(RandomNumber));
                     EDIV = 0;

                     /* Check to see if this is a request for a SC      */
                     /* generated Long Term Key.                        */
                     if((Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV == EDIV) && (QAPI_BLE_COMPARE_RANDOM_NUMBER(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand, RandomNumber)))
                     {
                        /* Search for the entry for this slave to store */
                        /* the information into.                        */
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           /* Check to see if the LTK is valid.         */
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                           {
                              /* Respond with the stored Long Term Key. */
                              GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                              memcpy(&(GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key), &(DeviceInfo->LTK), QAPI_BLE_LONG_TERM_KEY_SIZE);
                           }
                        }
                     }
                     else
                     {
                        /* The other side of a connection is requesting */
                        /* that we start encryption.  Thus we should    */
                        /* regenerate LTK for this connection and send  */
                        /* it to the chip.                              */
                        Result = qapi_BLE_GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (qapi_BLE_Encryption_Key_t *)(&DHK), (qapi_BLE_Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                        if(!Result)
                        {
                           QCLI_Printf(ble_Group, "      GAP_LE_Regenerate_Long_Term_Key Success.\n");

                           /* Respond with the Re-Generated Long Term   */
                           /* Key.                                      */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = QAPI_BLE_LAR_LONG_TERM_KEY_E;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = QAPI_BLE_GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = QAPI_BLE_GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                        }
                        else
                        {
                           QCLI_Printf(ble_Group, "      GAP_LE_Regenerate_Long_Term_Key returned %d.\n",Result);
                        }
                     }

                     /* Send the Authentication Response.               */
                     Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(Result)
                     {
                        QCLI_Printf(ble_Group, "      GAP_LE_Authentication_Response returned %d.\n",Result);
                     }
                     break;
                  case QAPI_BLE_LAT_SECURITY_REQUEST_E:
                     /* Display the data for this event.                */
                     /* * NOTE * This is only sent from Slave to Master.*/
                     /*          Thus we must be the Master in this     */
                     /*          connection.                            */
                     QCLI_Printf(ble_Group, "    latSecurityRequest:.\n");
                     QCLI_Printf(ble_Group, "      BD_ADDR: %s.\n", BoardStr);
                     QCLI_Printf(ble_Group, "      Bonding Type: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == QAPI_BLE_LBT_BONDING_E)?"Bonding":"No Bonding"));
                     QCLI_Printf(ble_Group, "      MITM: %s.\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM)?"YES":"NO"));

                     /* Make sure we are NOT pairing or re-establishing */
                     /* security with another remote device.            */
                     if(QAPI_BLE_COMPARE_NULL_BD_ADDR(BLBD_SecurityRemoteBD_ADDR))
                     {
                        /* Go ahead and store the address of the remote */
                        /* device we are currently pairing or           */
                        /* re-establishing security with.               */
                        BLBD_SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* Determine if we have previously paired with  */
                        /* the device.  If we have paired we will       */
                        /* attempt to re-establish security using a     */
                        /* previously exchanged LTK.                    */
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           /* Determine if a Valid Long Term Key is     */
                           /* stored for this device.                   */
                           if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                           {
                              QCLI_Printf(ble_Group, "Attempting to Re-Establish Security.\n");

                              /* Attempt to re-establish security to    */
                              /* this device.                           */
                              GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                              memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), sizeof(DeviceInfo->LTK));
                              GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                              memcpy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), sizeof(DeviceInfo->Rand));
                              GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                              Result = qapi_BLE_GAP_LE_Reestablish_Security(BluetoothStackID, BLBD_SecurityRemoteBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                              if(Result)
                              {
                                 QCLI_Printf(ble_Group, "GAP_LE_Reestablish_Security returned %d.\n", Result);
                              }
                           }
                           else
                           {
                              /* We do not have a stored Link Key for   */
                              /* this device so go ahead and pair to    */
                              /* this device.                           */
                              SendPairingRequest(BLBD_SecurityRemoteBD_ADDR, TRUE);
                           }
                        }
                        else
                        {
                           /* There is no Key Info Entry for this device*/
                           /* so we will just treat this as a slave     */
                           /* request and initiate pairing.             */
                           SendPairingRequest(BLBD_SecurityRemoteBD_ADDR, TRUE);
                        }
                     }
                     else
                     {
                        /* Inform the user that we cannot accept the    */
                        /* request at this time.                        */
                        QCLI_Printf(ble_Group, "\nSecurity is already in progress with another remote device.\n");

                        /* We are currently pairing/re-establishing     */
                        /* security with another remote device so we    */
                        /* should send the negative authentication      */
                        /* response.                                    */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                        /* Submit the Authentication Response.          */
                        if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                           DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                     }
                     break;
                  case QAPI_BLE_LAT_PAIRING_REQUEST_E:
                     QCLI_Printf(ble_Group, "Pairing Request: %s.\n", BoardStr);
                     DisplayLegacyPairingInformation(&Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);

                     /* Make sure we are NOT pairing or re-establishing */
                     /* security with another remote device.            */
                     if(QAPI_BLE_COMPARE_NULL_BD_ADDR(BLBD_SecurityRemoteBD_ADDR))
                     {
                        /* Go ahead and store the address of the remote */
                        /* device we are currently pairing or           */
                        /* re-establishing security with.               */
                        BLBD_SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* This is a pairing request.  Respond with a   */
                        /* Pairing Response.                            */
                        /* * NOTE * This is only sent from Master to    */
                        /*          Slave.  Thus we must be the Slave in*/
                        /*          this connection.                    */

                        /* Send the Pairing Response.                   */
                        SlavePairingRequestResponse(BLBD_SecurityRemoteBD_ADDR);
                     }
                     else
                     {
                        /* Inform the user that we cannot accept the    */
                        /* request at this time.                        */
                        QCLI_Printf(ble_Group, "\nSecurity is already in progress with another remote device.\n");

                        /* We are currently pairing/re-establishing     */
                        /* security with another remote device so we    */
                        /* should send the negative authentication      */
                        /* response.                                    */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                        /* Submit the Authentication Response.          */
                        if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                           DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                     }
                     break;
                  case QAPI_BLE_LAT_EXTENDED_PAIRING_REQUEST_E:
                     QCLI_Printf(ble_Group, "Extended Pairing Request: %s.\n", BoardStr);
                     DisplayPairingInformation(&(Authentication_Event_Data->Authentication_Event_Data.Extended_Pairing_Request));

                     /* Make sure we are NOT pairing or re-establishing */
                     /* security with another remote device.            */
                     if(QAPI_BLE_COMPARE_NULL_BD_ADDR(BLBD_SecurityRemoteBD_ADDR))
                     {
                        /* Go ahead and store the address of the remote */
                        /* device we are currently pairing or           */
                        /* re-establishing security with.               */
                        BLBD_SecurityRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* This is a pairing request.  Respond with a   */
                        /* Pairing Response.                            */
                        /* * NOTE * This is only sent from Master to    */
                        /*          Slave.  Thus we must be the Slave in*/
                        /*          this connection.                    */

                        /* Send the Pairing Response.                   */
                        SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                     }
                     else
                     {
                        /* Inform the user that we cannot accept the    */
                        /* request at this time.                        */
                        QCLI_Printf(ble_Group, "\nSecurity is already in progress with another remote device.\n");

                        /* We are currently pairing/re-establishing     */
                        /* security with another remote device so we    */
                        /* should send the negative authentication      */
                        /* response.                                    */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_ERROR_E;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;

                        /* Submit the Authentication Response.          */
                        if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                           DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                     }
                     break;
                  case QAPI_BLE_LAT_CONFIRMATION_REQUEST_E:
                     QCLI_Printf(ble_Group, "latConfirmationRequest.\n");

                     /* Check to see what type of confirmation request  */
                     /* this is.                                        */
                     switch(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type)
                     {
                        case QAPI_BLE_CRT_NONE_E:
                           /* Handle the just works request.            */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Handle this differently based on the local*/
                           /* IO Caps.                                  */
                           switch(LE_Parameters.IOCapability)
                           {
                              case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                 QCLI_Printf(ble_Group, "Invoking Just Works.\n");

                                 /* By setting the                      */
                                 /* Authentication_Data_Length to any   */
                                 /* NON-ZERO value we are informing the */
                                 /* GAP LE Layer that we are accepting  */
                                 /* Just Works Pairing.                 */

                                 Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                 if(Result)
                                 {
                                    QCLI_Printf(ble_Group, "qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                 }
                                 break;
                              case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                 QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                 GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                 break;
                              default:
                                 QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                 break;
                           }
                           break;
                        case QAPI_BLE_CRT_PASSKEY_E:
                           /* Inform the user to call the appropriate   */
                           /* command.                                  */
                           QCLI_Printf(ble_Group, "Call LEPasskeyResponse [PASSCODE].\n");
                           break;
                        case QAPI_BLE_CRT_DISPLAY_E:
                           QCLI_Printf(ble_Group, "Passkey: %06u.\n", (unsigned int)(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                           break;
                        default:
                           /* This application doesn't support OOB and  */
                           /* Secure Connections request types will be  */
                           /* handled by the ExtendedConfirmationRequest*/
                           /* event.  So we will simply inform the user.*/
                           QCLI_Printf(ble_Group, "Authentication method not supported.\n");
                           break;
                     }
                     break;
                  case QAPI_BLE_LAT_EXTENDED_CONFIRMATION_REQUEST_E:
                     QCLI_Printf(ble_Group, "latExtendedConfirmationRequest.\n");

                     QCLI_Printf(ble_Group, "   Secure Connections:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_SECURE_CONNECTIONS)?"YES":"NO");
                     QCLI_Printf(ble_Group, "   Just Works Pairing:     %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)?"YES":"NO");
                     QCLI_Printf(ble_Group, "   Keypress Notifications: %s.\n", (Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_KEYPRESS_NOTIFICATIONS_REQUESTED)?"YES":"NO");

                     /* Check to see what type of confirmation request  */
                     /* this is.                                        */
                     switch(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Request_Type)
                     {
                        case QAPI_BLE_CRT_NONE_E:
                           /* Handle the just works request.            */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Handle this differently based on the local*/
                           /* IO Caps.                                  */
                           switch(LE_Parameters.IOCapability)
                           {
                              case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                 QCLI_Printf(ble_Group, "Invoking Just Works.\n");

                                 /* Just Accept Just Works Pairing.     */
                                 Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                 if(Result)
                                 {
                                    QCLI_Printf(ble_Group, "qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                 }
                                 break;
                              case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                 QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                 GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                 break;
                              default:
                                 QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                 /* Submit the Authentication Response. */
                                 if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                    DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                 break;
                           }
                           break;
                        case QAPI_BLE_CRT_PASSKEY_E:
                           /* Inform the user to call the appropriate   */
                           /* command.                                  */
                           QCLI_Printf(ble_Group, "Call LEPasskeyResponse [PASSKEY].\n");
                           break;
                        case QAPI_BLE_CRT_DISPLAY_E:
                           QCLI_Printf(ble_Group, "Passkey: %06u.\n", Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = QAPI_BLE_LAR_PASSKEY_E;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (uint8_t)(sizeof(uint32_t));
                           GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey;

                           /* Since this is in an extended confirmation */
                           /* request we need to respond to the display */
                           /* request.                                  */
                           if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                              DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                           break;
                        case QAPI_BLE_CRT_DISPLAY_YES_NO_E:
                           /* Handle the Display Yes/No request.        */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_CONFIRMATION_E;

                           /* By setting the Authentication_Data_Length */
                           /* to any NON-ZERO value we are informing the*/
                           /* GAP LE Layer that we are accepting Just   */
                           /* Works Pairing.                            */
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = sizeof(uint32_t);

                           /* Check to see if this is Just Works or     */
                           /* Numeric Comparison.                       */
                           if(Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Flags & QAPI_BLE_GAP_LE_EXTENDED_CONFIRMATION_REQUEST_FLAGS_JUST_WORKS_PAIRING)
                           {
                              /* Handle this differently based on the   */
                              /* local IO Caps.                         */
                              switch(LE_Parameters.IOCapability)
                              {
                                 case QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E:
                                    QCLI_Printf(ble_Group, "Invoking Just Works.\n");

                                    /* Just Accept Just Works Pairing.  */
                                    Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                                    if(Result)
                                    {
                                       QCLI_Printf(ble_Group, "qapi_BLE_GAP_LE_Authentication_Response returned %d.\n", Result);
                                    }
                                    break;
                                 case QAPI_BLE_LIC_DISPLAY_ONLY_E:
                                    QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                    GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey;

                                    /* Submit the Authentication        */
                                    /* Response.                        */
                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                                 default:
                                    QCLI_Printf(ble_Group, "Confirmation of Pairing.\n");

                                    /* Submit the Authentication        */
                                    /* Response.                        */
                                    if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                       DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                                    break;
                              }
                           }
                           else
                           {
                              /* This is numeric comparison so go ahead */
                              /* and display the numeric value to       */
                              /* confirm.                               */
                              QCLI_Printf(ble_Group, "Confirmation Value: %ld\n", (unsigned long)Authentication_Event_Data->Authentication_Event_Data.Extended_Confirmation_Request.Display_Passkey);

                              /* Submit the Authentication Response.    */
                              if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                                 DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                              break;
                           }
                           break;
                        case QAPI_BLE_CRT_OOB_SECURE_CONNECTIONS_E:
                           QCLI_Printf(ble_Group, "OOB Data Request.\n");

                           /* Format the response.                      */
                           GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = QAPI_BLE_LAR_OUT_OF_BAND_DATA_E;
                           GAP_LE_Authentication_Response_Information.Authentication_Data_Length = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_SIZE;

                           /* Check to see if we have remote OOB data.  */
                           if(RemoteOOBValid)
                           {
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags        = 0;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Confirmation = RemoteOOBConfirmation;
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Randomizer   = RemoteOOBRandomizer;
                           }
                           else
                              GAP_LE_Authentication_Response_Information.Authentication_Data.Secure_Connections_OOB_Data.Flags = QAPI_BLE_GAP_LE_SECURE_CONNECTIONS_OOB_DATA_FLAGS_OOB_NOT_RECEIVED;

                           /* Submit the Authentication Response.       */
                           if((Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information)) != 0)
                              DisplayFunctionError("qapi_BLE_GAP_LE_Authentication_Response", Result);
                           break;
                        default:
                           /* This application doesn't support OOB so we*/
                           /* will simply inform the user.              */
                           QCLI_Printf(ble_Group, "Authentication method not supported.\n");
                           break;
                     }
                     break;
                  case QAPI_BLE_LAT_SECURITY_ESTABLISHMENT_COMPLETE_E:
                     QCLI_Printf(ble_Group, "Security Re-Establishment Complete: %s.\n", BoardStr);
                     QCLI_Printf(ble_Group, "                            Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status);

                     /* If this failed due to a LTK issue then we should*/
                     /* delete the LTK.                                 */
                     if(Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status == QAPI_BLE_GAP_LE_SECURITY_ESTABLISHMENT_STATUS_CODE_LONG_TERM_KEY_ERROR)
                     {
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           /* Clear the flag indicating the LTK is      */
                           /* valid.                                    */
                           DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LTK_VALID;
                            LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                        }
                     }

                     /* Flag the we are no longer                       */
                     /* pairing/re-establishing security with a remote  */
                     /* device.                                         */
                     QAPI_BLE_ASSIGN_BD_ADDR(BLBD_SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
                     break;
                  case QAPI_BLE_LAT_PAIRING_STATUS_E:
			
            QCLI_Printf(ble_Group, "Pairing Status  w.r.t bulb\n");
            QCLI_Printf(ble_Group, "Pairing Status: %s.\n", BoardStr);
                     QCLI_Printf(ble_Group, "        Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == QAPI_BLE_GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        QCLI_Printf(ble_Group, "        Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);
			
			        if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,Authentication_Event_Data->BD_ADDR)) != NULL) {
		                    DeviceInfo -> device_type = DEVICE_TYPE_BULB;
                            
				            if ((DeviceInfo_persistlist = SearchDeviceInfoEntryByType(&PersistDeviceInfoList,DEVICE_TYPE_BULB)) != NULL) { 
                                if (QAPI_BLE_COMPARE_BD_ADDR(Authentication_Event_Data->BD_ADDR, DeviceInfo_persistlist->RemoteAddress)) {
                                    break;
                                }  
                                memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t));

                            }else{

					            if((DeviceInfo_persistlist = CreateNewDeviceInfoEntry(&PersistDeviceInfoList, Authentication_Event_Data->BD_ADDR )) != NULL)                    
                                {
											
					                memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t));
                                    QCLI_Printf(ble_Group, "Device entry created in the persist info list\n");

			                   }
			
                        }

			}
				
			status = StorePersistentData();
            QCLI_Printf(ble_Group, "status of persistent storage after the bulb connection :%d\n",status);
                        
			DiscoverBLBDServices(Authentication_Event_Data->BD_ADDR);/* perform service discovery */
			//blbd_discover_services()
                    //blbd_connection_result_callback();
                     }
                     else
                     {
                        /* Disconnect the Link.                         */
                        /* * NOTE * Since we failed to pair, the remote */
                        /*          device information will be deleted  */
                        /*          when the GATT Disconnection event is*/
                        /*          received.                           */
                        qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     }

                     /* Flag the we are no longer                       */
                     /* pairing/re-establishing security with a remote  */
                     /* device.                                         */
                     QAPI_BLE_ASSIGN_BD_ADDR(BLBD_SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);



#if 0
                     QCLI_Printf(ble_Group, "Pairing Status: %s.\n", BoardStr);
                     QCLI_Printf(ble_Group, "        Status: 0x%02X.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status);

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == QAPI_BLE_GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        QCLI_Printf(ble_Group, "        Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size);
		            
                        if ((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList,Authentication_Event_Data->BD_ADDR)) != NULL) { 
                             DeviceInfo -> device_type = DEVICE_TYPE_BULB;                           
                             
                                if ((DeviceInfo = SearchDeviceInfoEntryByType(&PersistDeviceInfoList,DEVICE_TYPE_BULB)) != NULL) { 
                                   memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t);
                                 } else {
                                
                                if((DeviceInfo_persistlist = CreateNewDeviceInfoEntry(&PersistDeviceInfoList, Authentication_Event_Data->BD_ADDR )) != NULL)                      {    
                         
                                memcpy(DeviceInfo_persistlist, DeviceInfo , sizeof(DeviceInfo_t);
                                QCLI_Printf(ble_Group, "Device entry created in the persist info list\n");
                            }
                        }      
                    }

                } 
                        //Search Original List for this  address Authentication_Event_Data->BD_ADDR
                        //Search persist list for existing bulb info using device type 
                        // 1: If any bulb device type foudn in persit list, overwrite
                        // else : Create New entry in persist list, add flag for device type  DeviceInfo = CreateNewDeviceInfoEntry(
			            //blbd_discover_services()
                        DiscoverBLBDServices(Authentication_Event_Data->BD_ADDR);/* perform service discovery */
                  
                     }
                     else
                     {
                        /* Disconnect the Link.                         */
                        /* * NOTE * Since we failed to pair, the remote */
                        /*          device information will be deleted  */
                        /*          when the GATT Disconnection event is*/
                        /*          received.                           */
                        qapi_BLE_GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     }

                     /* Flag the we are no longer                       */
                     /* pairing/re-establishing security with a remote  */
                     /* device.                                         */
                    
                     QAPI_BLE_ASSIGN_BD_ADDR(BLBD_SecurityRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

#endif
                     break;
                  case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_REQUEST_E:
                     QCLI_Printf(ble_Group, "Encryption Information Request %s.\n", BoardStr);

                     /* Generate new LTK, EDIV and Rand and respond with*/
                     /* them.                                           */
                     EncryptionInformationRequestResponse(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size, &GAP_LE_Authentication_Response_Information);
                     break;
                  case QAPI_BLE_LAT_IDENTITY_INFORMATION_REQUEST_E:
                     QCLI_Printf(ble_Group, "Identity Information Request %s.\n", BoardStr);

                     /* Store our identity information to send to the   */
                     /* remote device since it has been requested.      */
                     /* * NOTE * This demo will ALWAYS use the local    */
                     /*          public address as the identity address */
                     /*          since it is known to the user.         */
                     GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                            = QAPI_BLE_LAR_IDENTITY_INFORMATION_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data_Length                            = (uint8_t)QAPI_BLE_GAP_LE_IDENTITY_INFORMATION_DATA_SIZE;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address      = LocalBD_ADDR;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.Address_Type = QAPI_BLE_LAT_PUBLIC_IDENTITY_E;
                     GAP_LE_Authentication_Response_Information.Authentication_Data.Identity_Information.IRK          = IRK;

                     /* Send the authentication response.               */
                     Result = qapi_BLE_GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(!Result)
                     {
                        QCLI_Printf(ble_Group, "   qapi_BLE_GAP_LE_Authentication_Response (larEncryptionInformation) success.\n");
                     }
                     else
                     {
                        QCLI_Printf(ble_Group, "   Error - SM_Generate_Long_Term_Key returned %d.\n", Result);
                     }
                     break;
                  case QAPI_BLE_LAT_ENCRYPTION_INFORMATION_E:
                     /* Display the information from the event.         */
                     QCLI_Printf(ble_Group, " Encryption Information from RemoteDevice: %s.\n", BoardStr);
                     QCLI_Printf(ble_Group, "                             Key Size: %d.\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size);

                     /* Search for the entry for this slave to store the*/
                     /* information into.                               */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        memcpy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), sizeof(DeviceInfo->LTK));
                        DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                        memcpy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), sizeof(DeviceInfo->Rand));
                        DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                        DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                        LOG_VERBOSE("flag %02x\n",DeviceInfo->Flags);
                     }
                     else
                     {
                        QCLI_Printf(ble_Group, "No Key Info Entry for this device.\n");
                     }
                     break;

                  case QAPI_BLE_LAT_IDENTITY_INFORMATION_E:
                     /* Display the information from the event.         */
                     QCLI_Printf(ble_Group, " Identity Information from RemoteDevice: %s.\n", BoardStr);

                     /* Search for the entry for this slave to store the*/
                     /* information into.                               */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Store the identity information for the remote*/
                        /* device.                                      */
                        memcpy(&(DeviceInfo->IRK), &(Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK), sizeof(DeviceInfo->IRK));
                        DeviceInfo->IdentityAddressBD_ADDR = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                        DeviceInfo->IdentityAddressType    = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                        DeviceInfo->Flags                 |= DEVICE_INFO_FLAGS_IRK_VALID;

                        /* Setup the resolving list entry to add to the */
                        /* resolving list.                              */
                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address      = DeviceInfo->IdentityAddressBD_ADDR;
                        DeviceInfo->ResolvingListEntry.Peer_Identity_Address_Type = DeviceInfo->IdentityAddressType;
                        DeviceInfo->ResolvingListEntry.Peer_IRK                   = DeviceInfo->IRK;
                        DeviceInfo->ResolvingListEntry.Local_IRK                  = IRK;
                     }
                     else
                     {
                        QCLI_Printf(ble_Group, "No Key Info Entry for this device.\n");
                     }
                     break;
                  default:
                     QCLI_Printf(ble_Group, "Unhandled event: %u\n", Authentication_Event_Data->GAP_LE_Authentication_Event_Type);
                     break;
               }
            }
            break;
         case QAPI_BLE_ET_LE_DIRECT_ADVERTISING_REPORT_E:
            QCLI_Printf(ble_Group, "etLE_Direct_Advertising_Report with size %d.\n", (int)GAP_LE_Event_Data->Event_Data_Size);
            QCLI_Printf(ble_Group, "  %d Responses.\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries);

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DirectDeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Direct_Advertising_Report_Event_Data->Direct_Advertising_Data[Index]);

               /* Display the Address Type.                             */
               switch(DirectDeviceEntryPtr->Address_Type)
               {
                  case QAPI_BLE_LAT_PUBLIC_E:
                     QCLI_Printf(ble_Group, "  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_E");
                     break;
                  case QAPI_BLE_LAT_RANDOM_E:
                     QCLI_Printf(ble_Group, "  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_E");
                     break;
                  case QAPI_BLE_LAT_PUBLIC_IDENTITY_E:
                     QCLI_Printf(ble_Group, "  Address Type: %s.\n", "QAPI_BLE_LAT_PUBLIC_IDENTITY_E");
                     break;
                  case QAPI_BLE_LAT_RANDOM_IDENTITY_E:
                     QCLI_Printf(ble_Group, "  Address Type: %s.\n", "QAPI_BLE_LAT_RANDOM_IDENTITY_E");
                     break;
                  default:
                     QCLI_Printf(ble_Group, "  Address Type: Invalid.\n");
                     break;
               }

               /* Display the Device Address.                           */
               QCLI_Printf(ble_Group, "  Address:      0x%02X%02X%02X%02X%02X%02X.\n", DirectDeviceEntryPtr->BD_ADDR.BD_ADDR5, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR4, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR3, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR2, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR1, DirectDeviceEntryPtr->BD_ADDR.BD_ADDR0);
               QCLI_Printf(ble_Group, "  RSSI:         %d.\n", (int)(DirectDeviceEntryPtr->RSSI));
            }
            break;
         default:
            DisplayPrompt = false;
            break;
      }

      if(DisplayPrompt)
         QCLI_Display_Prompt();
   }
}



/************************************************** Central Role End ***********************************************/


int Initialize_Home_Automation(void)
{

    uint32_t ConnectionTimeout = 15 ; /* wait for 15 sec after the connect request is sent */
    DeviceInfo_t *bulb_device;
    qapi_BLE_GAP_LE_Address_Type_t type;
    QCLI_Command_Status_t status;
    qapi_BLE_BD_ADDR_t BulbAddress;
    memset(&BulbAddress, 0, sizeof(qapi_BLE_BD_ADDR_t));
    ble_Group = QCLI_Register_Command_Group(NULL, &ble_cmd_group);
    if (InitializeBluetooth() != QCLI_STATUS_SUCCESS_E)
    {
        LOG_ERROR("BLE: Error InitializeBluetooth()\n");
        return -1;
    }

    QCLI_Printf(ble_Group, "Before loadin\n\n");
    status = LoadPersistentData(&DeviceInfoList, &BulbAddress,&type);
    QCLI_Printf(ble_Group, "status of loading the persistent data of deviceinfo list : %0x\n",status);
    status = LoadPersistentData(&PersistDeviceInfoList, &BulbAddress,&type);
    QCLI_Printf(ble_Group, "status of loading the persistent data of persistdevice info list : %0x\n",status);

    if (AdvertiseLE(1) != QCLI_STATUS_SUCCESS_E)
    {
        LOG_ERROR("BLE: error AdvertiseLE()\n");
        return -1;
    }
    LOG_ERROR("BLE: AdveiseLE() enabled\n");
    /*Start Central thread */

    BLE_timer_init();

    Sleep(1000);

    QCLI_Printf(ble_Group, "searching the bulb device\n\n");
    if ( BulbAddress.BD_ADDR0 != 0x00 && BulbAddress.BD_ADDR1 != 0x00 ) {
        ConnectLEDevice(BluetoothStackID, FALSE, &BulbAddress, type);

        QCLI_Printf(ble_Group, "Device entry found , sending the connet request\n");

        //  state_flags = state_flags | SCAN_PERMITTED; 
        ConnTimerID = qapi_BLE_BSC_StartTimer(BluetoothStackID, (ConnectionTimeout * 1000) , Conn_Timer_Callback, ConnectionTimeout);
        if(!ConnTimerID)
            QCLI_Printf(ble_Group, "Failed to start the connection timer\n");
        else
            QCLI_Printf(ble_Group, "Connection timer started successfully\n");
    } else {
        LOG_ERROR ("Bulb device not found\n\n");
    }  
    
    while (SMOKE_SENSOR_ENABLED) {
        LOG_ERROR("Smoke detector value %02x\n\n",smoke_Detector_Value);
        
        adc_test1(&smoke_Detector_Value);
        Sleep(1000);
    }
    
    LOG_ERROR("Done\n\n\n\n\n");
    
    return QCLI_STATUS_SUCCESS_E;
}
















