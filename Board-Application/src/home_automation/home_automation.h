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

#ifndef _HOMEAUTOMATIONH__
#define __HOMEAUTOMATIONH__

#define QAPI_USE_BLE

#include "qapi.h"
#include "qapi_ble_bttypes.h"
#include "qcli_api.h"
#include "home_automation_types.h"
#include "qapi_persist.h"



#define DEVICE_TYPE_BULB            1
#define DEVICE_TYPE_MOBILE          2

#define SMOKE_SENSOR_ENABLED        1

#define BLB_NUM_BULBS 1
#define BLBD_THREAD_STACK_SIZE      (2048)
#define BLBD_THREAD_PRIORITY        (12)
#define BLBD_THREAD_STOP            (1<<5)

#define BLBD_SCAN_RESULT_SIGNAL_INTR        (1)
#define BLBD_CONNECTION_SUCCESS_SIGNAL_INTR     (2)
#define BLBD_SERVICE_DISCOVERY_SIGNAL_INTR (3)
#define BLBD_PERIODIC_TIMER_SIGNAL_INTR (4)
#define BLBD_SCAN_STOPPED_SIGNAL_INTR (5)

#define BLBD_SCAN_RESULT        (6)
#define BLBD_CONNECTION_RESULT      (7)
#define BLBD_SERVICE_DISCOVERY_RESULT (8)
#define BLBD_DISCONNECTION_RESULT (9)
#define BLBD_CONNECTION_FAILED_SIGNAL_INTR (10)
#define BLBD_CONNECTION_TICKS 6

#define BLBD_DEVICE_SIGNATURE "HOME-BLB"
#define BLB_MAX_CHARS 3
#define AD_TYPE_LOCAL_NAME 0x09
#define BLBD_SERVICE_UUID { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00 }

#define QAPI_BLE_BLBD_COMPARE_SERVICE_UUID_TO_UUID_16(_x)  QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0xFF, 0x01)
#define QAPI_BLE_BLBD_COMPARE_CHAR_UUID_TO_UUID_128(_x)  QAPI_BLE_COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x00, 0x00, 0xFF, 0xFB, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)
#define QAPI_BLE_BLBD_COMPARE_SERVICE_UUID_TO_UUID_128(_x)  QAPI_BLE_COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x00, 0x00, 0xFF, 0x10, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB)
#define QAPI_BLE_BLBD_COMPARE_CHAR_UUID_TO_UUID_16(_x)  QAPI_BLE_COMPARE_BLUETOOTH_UUID_16_TO_CONSTANT((_x), 0xFF, 0xFC)



#define HOME_AUTOMATION 1

#ifndef V2
   #define QAPI_BLE_LAT_ANONYMOUS_E                   255
#endif

   /* Some MACROs for accessing little-endian unaligned values.         */
#define READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x)  (((uint8_t *)(_x))[0])
#define READ_UNALIGNED_WORD_LITTLE_ENDIAN(_x)  ((uint16_t)((((uint16_t)(((uint8_t *)(_x))[1])) << 8) | ((uint16_t)(((uint8_t *)(_x))[0]))))

#define ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(_x, _y)        \
{                                                                       \
  ((uint8_t *)(_x))[0] = ((uint8_t)(((uint16_t)(_y)) & 0xFF));          \
  ((uint8_t *)(_x))[1] = ((uint8_t)((((uint16_t)(_y)) >> 8) & 0xFF));   \
}

#define CONVERT_TO_BASEBAND_SLOTS(_x)                             ((unsigned long)((((8000L * ((unsigned long)(_x))) / 500L) + 5L)/10L))

   /* Determine the Name we will use for this compilation.              */
#define DEVICE_FRIENDLY_NAME                       "QCA"

   /* The following MACRO is used to convert an ASCII character into the*/
   /* equivalent decimal value.  The MACRO converts lower case          */
   /* characters to upper case before the conversion.                   */
#define ToInt(_x)                                  (((_x) > 0x39)?(((_x) & ~0x20)-0x37):((_x)-0x30))

   /* Generic Access Profile (GAP) Constants.                           */

#define DEFAULT_IO_CAPABILITY      (QAPI_BLE_LIC_NO_INPUT_NO_OUTPUT_E)
                                                         /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Pairing.*/

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define DEFAULT_SECURE_CONNECTIONS               (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Secure        */
                                                         /* Connections used  */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

   /* Automation IO Service (AIOS) Constants.                           */

#define AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS   (2)   /* Denotes the number*/
                                                         /* of                */
                                                         /* characteristics   */
                                                         /* supported by the  */
                                                         /* AIOS Server.      */

#define AIOP_NUMBER_OF_SUPPORTED_INSTANCES         (2)   /* Denotes the number*/
                                                         /* of                */
                                                         /* instances for each*/
                                                         /* Characteristic    */
                                                         /* supported by the  */
                                                         /* AIOS Server.      */

#define AIOP_DEFAULT_INPUT_CHARACTERISTIC_PROPERTY_FLAGS  (QAPI_BLE_AIOS_INPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY)
                                                         /* Denotes the default*/
                                                         /* input              */
                                                         /* Characteristic     */
                                                         /* Property Flags.    */

#define AIOP_DEFAULT_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS (QAPI_BLE_AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_WRITE | \
                                                           QAPI_BLE_AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_READ  | \
                                                           QAPI_BLE_AIOS_OUTPUT_CHARACTERISTIC_PROPERTY_FLAGS_NOTIFY)
                                                         /* Denotes the default*/
                                                         /* output             */
                                                         /* Characteristic     */
                                                         /* Property Flags.    */

   /* Battery Alert Service (BAS) Constants.                            */

#define MAX_SUPPORTED_BATTERY_INSTANCES            (1)   /* Denotes the       */
                                                         /* maximum number of */
                                                         /* Battery Service   */
                                                         /* Instances that are*/
                                                         /* supported by this */
                                                         /* application.      */


   /* HID over GATT Service (HOGP) Constants.                           */


#define MAX_NUM_DEVICES                (1)   /* Denotes the       */
                                                         /* maximum number of */
                                                         /* Instances that are*/
                                                         /* supported by this */
                                                         /* application.      */

   /* Serial Port Profile over LE (SPPLE) Constants.                    */

#ifndef SPPLE_DATA_BUFFER_LENGTH

#define SPPLE_DATA_BUFFER_LENGTH    (517)
                                                         /* Defines the length*/
                                                         /* of a SPPLE Data   */
                                                         /* Buffer.           */
#endif

#define SPPLE_DATA_CREDITS        (SPPLE_DATA_BUFFER_LENGTH*3)
                                                         /* Defines the       */
                                                         /* number of credits */
                                                         /* in an SPPLE Buffer*/




   /* The following bit mask values may be used for the Flags field of  */
   /* the BLEParameters_t structure.                                    */
#define BLE_PARAMETERS_FLAGS_ADVERTISING_PARAMETERS_VALID   0x00000001
#define BLE_PARAMETERS_FLAGS_SCAN_PARAMETERS_VALID          0x00000002
#define BLE_PARAMETERS_FLAGS_CONNECTION_PARAMETERS_VALID    0x00000004


   /* Remote Device Information structure.                              */

   /* The following bit mask values may be used for the Flags field of  */
   /* the DeviceInfo_t structure.                                       */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x01
#define DEVICE_INFO_FLAGS_ENCRYPTED                         0x02
#define DEVICE_INFO_FLAGS_SPPLE_CLIENT                      0x04
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x08
#define DEVICE_INFO_FLAGS_IRK_VALID                         0x10
#define DEVICE_INFO_FLAGS_ADDED_TO_WHITE_LIST               0x20
#define DEVICE_INFO_FLAGS_ADDED_TO_RESOLVING_LIST           0x40



/* Persistent storage flags */

#define PERSISTENT_REMOTE_DEVICE_DATA_FLAG_LTK_VALID        0x01
#define PERSISTENT_REMOTE_DEVICE_DATA_FLAG_IDENTITY_VALID   0x02




enum
{
    AET_DISABLE_E,
    AET_ENABLE_ALL_E,
    AET_ENABLE_CHANNEL_37_E,
    AET_ENABLE_CHANNEL_38_E,
    AET_ENABLE_CHANNEL_39_E
};


typedef struct BLBD_Q_s
{
    int event_type;
    void *data;
} BLBD_Q_t;

/* Generic Access Profile (GAP) structures.*/

/* Structure used to hold all of the GAP LE Parameters.*/


typedef struct _tagGAPLE_Parameters_t
{
   qapi_BLE_GAP_LE_Connectability_Mode_t ConnectableMode;
   qapi_BLE_GAP_Discoverability_Mode_t   DiscoverabilityMode;
   qapi_BLE_GAP_LE_IO_Capability_t       IOCapability;
   boolean_t                             MITMProtection;
   boolean_t                             SecureConnections;
   boolean_t                             OOBDataPresent;
} GAPLE_Parameters_t;

#define GAPLE_PARAMETERS_DATA_SIZE                       (sizeof(GAPLE_Parameters_t))


   /* The following structure is used to hold the Scan Window and       */
   /* Interval parameters for LE Scanning.                              */
typedef struct _tagBLEScanParameters_t
{
   uint16_t ScanInterval;
   uint16_t ScanWindow;
} BLEScanParameters_t;

   /* The following structure is used to hold information on the        */
   /* configured Scan/Advertising/Connection Parameters.                */
typedef struct _tagBLEParameters_t
{
   unsigned long                            Flags;
   qapi_BLE_GAP_LE_Advertising_Parameters_t AdvertisingParameters;
   qapi_BLE_GAP_LE_Connection_Parameters_t  ConnectionParameters;
   BLEScanParameters_t                      ScanParameters;
} BLEParameters_t;

   /* Automation IO Service (AIOS) structures.                          */

   /* The following structure contains the information for an AIOS      */
   /* Characteristic instance that the AIOS Server will need to store.  */
   /* * NOTE * The Instance_Entry below will need to be copied to the   */
   /*          qapi_BLE_AIOS_Characteristic_Entry_t structure, a        */
   /*          sub-structure of qapi_BLE_AIOS_Initialize_Data_t         */
   /*          structure, that is expected as a parameter to            */
   /*          qapi_BLE_AIOS_Initialize_Service().  This is REQUIRED to */
   /*          intialize the service and allows us to retain the        */
   /*          information that we used to initialize the service.      */
   /* * NOTE * Some fields of this structure will not be used.  The     */
   /*          fields depend on the optional AIOS Characteristic        */
   /*          descriptors included for this Characteristic instance    */
   /*          specified by the Instance_Entry field and whether this   */
   /*          instance is a Digital or Analog Characteristic.          */
   /* * NOTE * The AIOS Server will support 8 digital signals (2 octets)*/
   /*          for each Digital Characteristic for simplicity.          */
typedef struct _tagAIOP_Server_Instance_Data_t
{
   qapi_BLE_AIOS_Characteristic_Instance_Entry_t Instance_Entry;

   union
   {
      uint8_t                                    Digital[2];
      uint16_t                                   Analog;
   } Data;

   uint16_t                                      Client_Configuration;
   qapi_BLE_AIOS_Presentation_Format_Data_t      Presentation_Format;
   uint8_t                                       Number_Of_Digitals;
} AIOP_Server_Instance_Data_t;

#define AIOP_SERVER_INSTANCE_DATA_SIZE                   (sizeof(AIOP_Server_Instance_Data_t))

   /* The following structure contains the information for each AIOS    */
   /* Digital/Analog Characteristc that the AIOS Server will need to    */
   /* store.  Information for each AIOS Characteristic instance will be */
   /* stored by the Instances field.                                    */
   /* * NOTE * The Characteristic_Entry field below will need to be     */
   /*          copied to the qapi_BLE_AIOS_Initialize_Data_t structure  */
   /*          that is expected as a parameter to                       */
   /*          qapi_BLE_AIOS_Initialize_Service().  This is REQUIRED to */
   /*          initialize the service and allows us to retain the       */
   /*          information that we used to intialize the service.       */
   /* * NOTE * The AIOS Server will support two instances of each AIOS  */
   /*          Characteristic (Digital and Analog) for simplicity.      */
typedef struct _tagAIOP_Server_Characteristic_Data_t
{
   qapi_BLE_AIOS_Characteristic_Entry_t Characteristic_Entry;
   AIOP_Server_Instance_Data_t          Instances[AIOP_NUMBER_OF_SUPPORTED_INSTANCES];
} AIOP_Server_Characteristic_Data_t;

#define AIOP_SERVER_CHARACTERISTIC_DATA_SIZE             (sizeof(AIOP_Server_Characteristic_Data_t))

   /* The following structure contains the AIOS Server information.     */
   /* This information (and sub structures) are needed to initialize the*/
   /* AIOS Server with a call to qapi_BLE_AIOS_Initialize_Service().    */
   /* This structure will also hold the information needed to process   */
   /* AIOS Server events and will retain the values for AIOS            */
   /* Characteristics and descriptors.                                  */
   /* * NOTE * Some fields below will need to be copied to the          */
   /*          qapi_BLE_AIOS_Initialize_Data_t structure that is        */
   /*          expected as a parameter to                               */
   /*          qapi_BLE_AIOS_Initialize_Service().  This is REQUIRED to */
   /*          initialize the service and allows us to retain the       */
   /*          information that we used to intialize the service.       */
   /* * NOTE * The AIOS Server will support two characteristics: the    */
   /*          Digital and Analog Characteristics, for simplicity.      */
typedef struct _tagAIOP_Server_Information_t
{
   AIOP_Server_Characteristic_Data_t Characteristic[AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS];
} AIOP_Server_Information_t;

#define AIOP_SERVER_INFORMATION_DATA_SIZE                (sizeof(AIOP_Server_Information_t))

   /* The following enumeration will be used to determine the correct   */
   /* Attribute Handle to select for an AIOS Characteristic or          */
   /* Descriptor.                                                       */
typedef enum
{
   ahtCharacteristic,
   ahtClientCharacteristicConfig,
   ahtPresentationFormat,
   ahtNumberOfDigitals
} AIOP_Attribute_Handle_Type_t;

   /* The following structure holds the request information that the    */
   /* AIOP Client MUST store before issuing a GATT request to the AIOS  */
   /* Server.  This is so that we can easily handle the response.       */
   /* * NOTE * The Type and ID fields MUST be valid for all requests    */
   /*          since this information is required to quickly look up the*/
   /*          AIOS Characteristic Instance's information associated    */
   /*          with the request in the GATT_ClientEventCallback_AIOS()  */
   /*          when the response is received.                           */
   /* * NOTE * The AttributeHandleType field (Mandatory) allows us to   */
   /*          specify the type of the attribute handle we are expecting*/
   /*          in the response.  This way with the Type and ID fields,  */
   /*          we can quickly locate the correct attribute handle to    */
   /*          verify.  Otherwise we would need to check every attribute*/
   /*          handle for a match to know how to process the response.  */
typedef struct _tagAIOP_Client_Request_Info_t
{
   qapi_BLE_AIOS_Characteristic_Type_t Type;
   uint16_t                            ID;
   AIOP_Attribute_Handle_Type_t        AttributeHandleType;
} AIOP_Client_Request_Info_t;

   /* The following structure contains the information that needs to be */
   /* stored by an AIOS Client for each AIOS Characteristic instance    */
   /* discovered during service discovery.  This struture also stores   */
   /* the information that the AIOP Client needs to store when          */
   /* read/writing AIOS Characteristic instances.                       */
   /* * NOTE * The Properties field will simply be used to store the    */
   /*          Characteristic instance properties found during service  */
   /*          discovery.                                               */
   /* * NOTE * The Number_Of_Digitals will hold the number of digitals  */
   /*          that has been automatically read by the AIOS Client if   */
   /*          the Aggregate Characteristic is discoverd and after      */
   /*          service discovery has been peformed.  This is REQUIRED   */
   /*          since in order to decode the Aggregate Characteristic we */
   /*          MUST know how many digitals are included for each Digital*/
   /*          Characteristic that is part of the Aggregate             */
   /*          Characteristic.                                          */
   /* * NOTE * Either the Digital_Characteristic_Handle or              */
   /*          Analog_Charactersitic_Handle will be cached.  Only one   */
   /*          will be cached for this instance and can be determined by*/
   /*          the Characteristic type (Type field) of the parent       */
   /*          structure below.                                         */
   /* * NOTE * The AIOS_Number_Of_Digitals_Handle will only be cached if*/
   /*          the instance is for a Digital Characteristic.            */
typedef struct _tagAIOP_Client_Instance_Info_t
{
   boolean_t               Valid;
   uint8_t                 Properties;
   qapi_BLE_AIOS_IO_Type_t IOType;
   uint8_t                 Number_Of_Digitals;

   uint16_t                Analog_Charactersitic_Handle;
   uint16_t                Digital_Characteristic_Handle;
   uint16_t                CCCD_Handle;
   uint16_t                Presentation_Format_Handle;
   uint16_t                Number_Of_Digitals_Handle;
} AIOP_Client_Instance_Info_t;

#define AIOP_CLIENT_INSTANCE_INFO_SIZE                   (sizeof(AIOP_Client_Instance_Info_t))

   /* The following structure contains the information that needs to be */
   /* stored by an AIOS Client for a specified AIOS Characteristic type */
   /* and all of its instances that may be cached by an AIOP Client     */
   /* during service discovery.                                         */
typedef struct _tagAIOP_Client_Characteristic_Info_t
{
   qapi_BLE_AIOS_Characteristic_Type_t Type;
   AIOP_Client_Instance_Info_t         Instances[AIOP_NUMBER_OF_SUPPORTED_INSTANCES];
} AIOP_Client_Characteristic_Info_t;

#define AIOP_CLIENT_CHARACTERISTIC_INFO_SIZE             (sizeof(AIOP_Client_Characteristic_Info_t))

   /* The following structure contains the information that will need to*/
   /* be cached by a AIOS Client in order to only do service discovery  */
   /* once.  This structure also contains the information that needs to */
   /* be stored by an AIOP Client when read/writing AIOS Characteristic */
   /* instances.                                                        */
   /* ** NOTE ** This demo will only support the demo's AIOS Server.  If*/
   /*            it is used to against another AIOS Server, then        */
   /*            optional Characteristics and descriptors, the Aggregate*/
   /*            Characteristic, and more Digital and Analog            */
   /*            Characteristics instances greater than the maximum     */
   /*            supported by the demo's AIOS Server will not be cached */
   /*            by the demo's AIOS Client.  This constraint applies to */
   /*            all sub structures.                                    */
   /* * NOTE * The Characteristics field may only be valid for a Digital*/
   /*          or Analog Characteristic.                                */
   /* * NOTE * The Number_Digital_Characteristics_In_Aggregate field    */
   /*          will be used to quickly determine how many Digital       */
   /*          Characteristics are included in the Aggregate            */
   /*          Characteristic during service discovery.  We can use this*/
   /*          information to automatically issue GATT read requests    */
   /*          (after service discovery has been peformed) for the      */
   /*          Number Of Digitals descriptor for each Digital           */
   /*          Characteristic included in the Aggregate that needs to be*/
   /*          cached in order to decode the Aggregate Characteristic.  */
typedef struct _tagAIOP_Client_Information_t
{
   AIOP_Client_Characteristic_Info_t Characteristics[AIOP_NUMBER_OF_SUPPORTED_CHARACTERISTICS];
   AIOP_Client_Request_Info_t        Client_Request_Info;
} AIOP_Client_Information_t;

#define AIOP_CLIENT_INFORMATION_DATA_SIZE                (sizeof(AIOP_Client_Information_t))

   /* HID over GATT (HIDS) structures.                                  */

   /* The following structure defines a key mapping.                    */
typedef struct _tagKeyMapping_t
{
   char    Ascii;
   uint8_t HID;
   uint8_t Modifiers;
} KeyMapping_t;

   /* Serial Port Profile over LE (SPPLE) structures.                   */

   /* The following structure holds status information about a send     */
   /* process.                                                          */
typedef struct _tagSend_Info_t
{
   uint32_t BytesToSend;
   uint32_t BytesSent;
} Send_Info_t;

   /* The following defines the format of a SPPLE Data Buffer.          */
typedef struct __tagSPPLE_Data_Buffer_t
{
   unsigned int  InIndex;
   unsigned int  OutIndex;
   unsigned int  BytesFree;
   unsigned int  BufferSize;
   uint8_t       Buffer[SPPLE_DATA_BUFFER_LENGTH*3];
} SPPLE_Data_Buffer_t;

   /* Generic Access Profile Service (GAPS) structures.                 */

   /* The following structure represents the information we will store  */
   /* on a Discovered GAP Service.                                      */
typedef struct _tagGAPS_Client_Info_t
{
   uint16_t DeviceNameHandle;
   uint16_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   uint16_t  Appearance;
   char     *String;
} GAPS_Device_Appearance_Mapping_t;


/* The following structure is used to track the sending and receiving*/
/* of data for the throughput test.                                  */
typedef struct _tagXferInfo_t
{
   uint64_t  RxCount;
   boolean_t TimingStarted;
   uint64_t  FirstTime;
   uint64_t  LastTime;
} XferInfo_t;

   /* The following structure holds the information that needs to be    */
   /* stored for a connected remote device.                             */
   /* * NOTE * If the local device pairs with the remote device, then   */
   /*          the LTK MUST ve valid, and the remote device information */
   /*          MUST persist between connections.  If the local device   */
   /*          does NOT pair with the remote device, then the LTK will  */
   /*          NOT be valid, and the remote device information will be  */
   /*          deleted when the remote device is disconnected.          */
   /* * NOTE * The ConnectionID will be used to indicate that a remote  */
   /*          device is currently connected.  Otherwise it will be set */
   /*          to zero to indicate that the remote device is currently  */
   /*          disconnected.                                            */
   /* * NOTE * The SelectedRemoteBD_ADDR will correspond to the         */
   /*          RemoteAddress field of the remote device that is         */
   /*          currently connected.                                     */
typedef struct _tagDeviceInfo_t
{
   uint8_t                                Flags;
   uint8_t                                BulbFlag;
   qapi_BLE_BD_ADDR_t                     BulbAddress;
   uint8_t                                device_type;
   unsigned int                           ConnectionID;
   boolean_t                              RemoteDeviceIsMaster;
   qapi_BLE_BD_ADDR_t                     RemoteAddress;
   qapi_BLE_GAP_LE_Address_Type_t         RemoteAddressType;
   qapi_BLE_GAP_LE_Address_Type_t         IdentityAddressType;
   qapi_BLE_BD_ADDR_t                     IdentityAddressBD_ADDR;
   uint8_t                                EncryptionKeySize;
   qapi_BLE_Long_Term_Key_t               LTK;
   qapi_BLE_Encryption_Key_t              IRK;
   qapi_BLE_Random_Number_t               Rand;
   uint16_t                               EDIV;
   qapi_BLE_GAP_LE_White_List_Entry_t     WhiteListEntry;
   qapi_BLE_GAP_LE_Resolving_List_Entry_t ResolvingListEntry;
   AIOP_Client_Information_t              AIOPClientInfo;
   uint16_t                               AIOPServerConfiguration;
   qapi_BLE_BAS_Client_Information_t      BASClientInfo[MAX_SUPPORTED_BATTERY_INSTANCES];
   qapi_BLE_BAS_Server_Information_t      BASServerInfo[MAX_SUPPORTED_BATTERY_INSTANCES];
   GAPS_Client_Info_t                     GAPSClientInfo;
   qapi_BLE_SCPS_Client_Information_t     SCPSClientInfo;
   qapi_BLE_SCPS_Server_Information_t     SCPSServerInfo;
   
   qapi_BLE_HRS_Client_Information_t      HRSClientInfo;
   SPPLE_Client_Info_t                    ClientInfo;
   SPPLE_Server_Info_t                    ServerInfo;
   unsigned int                           TransmitCredits;
   SPPLE_Data_Buffer_t                    ReceiveBuffer;
   SPPLE_Data_Buffer_t                    TransmitBuffer;
   XferInfo_t                             XferInfo;
   boolean_t                              ThroughputModeActive;
   struct _tagDeviceInfo_t               *NextDeviceInfoInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

typedef struct _tagPersistentRemoteDeviceData_t
{
   uint8_t                        Flags;
   uint8_t                        device_type;
   qapi_BLE_BD_ADDR_t             LastAddress;
   qapi_BLE_GAP_LE_Address_Type_t LastAddressType;
   qapi_BLE_BD_ADDR_t             IdentityAddress;
   qapi_BLE_GAP_LE_Address_Type_t IdentityAddressType;
   uint8_t                        EncryptionKeySize;
   qapi_BLE_Long_Term_Key_t       LTK;
   qapi_BLE_Encryption_Key_t      IRK;
} PersistentRemoteDeviceData_t;

#define PERSISTENT_REMOTE_DEVICE_DATA_SIZE               (sizeof(PersistentRemoteDeviceData_t))


typedef struct _tagPersistentData_t
{
   qapi_BLE_BD_ADDR_t LocalAddress;
   uint8_t NumberRemoteDevices;
   uint8_t BulbFlag;
   qapi_BLE_BD_ADDR_t BulbAddress;
   qapi_BLE_GAP_LE_Address_Type_t BulbAddressType;
   PersistentRemoteDeviceData_t RemoteDevices;
} PersistentData_t;

#define PERSISTENT_DATA_SIZE(_x)                         (QAPI_BLE_BTPS_STRUCTURE_OFFSET(PersistentData_t, RemoteDevices) + (PERSISTENT_REMOTE_DEVICE_DATA_SIZE * (_x)))


typedef struct blbd_device_chars
{
   uint8_t                 Properties;
   uint16_t                Characteristic_Handle;
   uint16_t                CCCD_Handle;
}BLBD_DEVICE_CHARS;

typedef struct blbd_dev_Instance_Info_t
{
   qapi_BLE_GAP_LE_Advertising_Report_Data_t * scan_data;
   DeviceInfo_t *connection_info;
   BLBD_DEVICE_CHARS *dev_chars;
   int valid;

}BLBD_Device;

typedef struct blbd_temp_dev_Instance_Info_t
{
   qapi_BLE_GAP_LE_Advertising_Report_Data_t scan_data;
   int valid;

}BLBD_Temp_Device;


/**
 * @func : Initialize_Home_Automation
 * @Desc : The following function initializes the bluetooth , timer and sensor functionalities */

int Initialize_Home_Automation(void);


/**
 * @func : BLBDWriteData
 * @Desc : The following function performs the write characteristic value */


void BLBDWriteData(uint32_t val);

/**
 * @func : GetBluetoothStackID
 * @Desc : The following function returns the current Bluetooth Stack ID  */

uint32_t GetBluetoothStackID(void);

/**
 * @func : AdvertiseLE
 * @Desc : The following function performs BLE advertising  */

int AdvertiseLE(uint32_t enable);

/* returns the connection ID of a remote device or zero if it does not exist. */

/**
 * @func : GetConnectionID
 * @Desc : The following function returns the connection ID of a remote device or zero if it does not exist   */

unsigned int GetConnectionID(qapi_BLE_BD_ADDR_t RemoteDevice);

/**
 * @func : StrToBD_ADDR
 * @Desc :The following function is responsible for conversion of the specified string    */
/*          into data of type BD_ADDR  */
               
void StrToBD_ADDR(char *BoardStr, qapi_BLE_BD_ADDR_t *Board_Address);

/**
 * @func : GetConnectionID
 * @Desc : The following function checks the encryption status of the connection */

int CheckEncryptionStatus (qapi_BLE_BD_ADDR_t RemoteDevice);

/**
 * @func : GetConnectionID
 * @Desc : The following function performs the BLE scan   */

int MscStartScan(uint32_t BluetoothStackID, qapi_BLE_GAP_LE_Filter_Policy_t FilterPolicy, unsigned int ScanDuration);

/**
 * @func : GetConnectionID
 * @Desc : The following function performs the LE create connection   */

int ConnectLEDevice(uint32_t BluetoothStackID, boolean_t UseWhiteList, qapi_BLE_BD_ADDR_t *BD_ADDR, qapi_BLE_GAP_LE_Address_Type_t AddressType);

void Conn_Timer_Callback(uint32_t BluetoothStackID, uint32_t TimerID, uint32_t CallbackParameter);


#endif
