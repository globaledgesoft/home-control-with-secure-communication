/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
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
#include <stdint.h>
#include <string.h>
#include "ble_server.h"
#include "qapi.h"
#include "malloc.h"
#include "qurt_signal.h"
#include "qurt_timer.h"
#include "qurt_mutex.h"
#include "qurt_error.h"
#include "qcli_api.h"
#include "qurt_thread.h"
#include "string.h"
#include "adc_demo.h"
#include "pwm_demo.h"
#include "home_automation.h"
// HOME AUTOMATION

typedef char BoardStr_t[16]; 
uint32_t service_id;
uint32_t connection_id;
uint16_t SmokeDetectorValue = 0;
uint8_t BulbState = BULB_STATE_DISCONNECTED;
volatile uint32_t    notify_thread_flag = 0;
uint32_t ble_stack_id = 0;

qapi_BLE_GATT_Server_Event_Data_t *gatt_sever;

static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr);

static void lock();

static void unlock();

static uint16_t get_smokedetector_value();

extern QCLI_Group_Handle_t ble_group;               /* Handle for our main QCLI Command*/

extern uint32_t BT_Stk_Id();

static void GATT_ServerEventCallback_Home_Automation(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter);


static void BD_ADDRToStr(qapi_BLE_BD_ADDR_t Board_Address, BoardStr_t BoardStr) 
{     
 snprintf((char *)BoardStr, (sizeof(BoardStr_t)/sizeof(char)), "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4      , Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}             


static const qapi_BLE_GATT_Primary_Service_128_Entry_t BLE_LOCK_Service_UUID =
{
   BLE_LOCK_DEC_SERVICE_UUID_CONSTANT
} ;


   /* The Custom Characteristic Declaration. */
static const qapi_BLE_GATT_Characteristic_Declaration_128_Entry_t BLE_LOCK_CHAR_Declaration =
{
   (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE),
   BLE_LOCK_DEC_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_128_Entry_t BLE_SMO_DET_CHAR_Declaration =
{
   (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY ),
   BLE_SMO_DEC_CHARACTERISTIC_UUID_CONSTANT
} ;

static const qapi_BLE_GATT_Characteristic_Declaration_128_Entry_t BLE_BULB_CHAR_Declaration =
{
   (QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_READ | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_WRITE | QAPI_BLE_GATT_CHARACTERISTIC_PROPERTIES_NOTIFY),
   BLE_BULB_DEC_CHARACTERISTIC_UUID_CONSTANT
} ;

   /* The Custom Characteristic Value. */
static const qapi_BLE_GATT_Characteristic_Value_128_Entry_t  BLE_LOCK_Value =
{
   BLE_LOCK_DEC_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_128_Entry_t  BLE_SMO_DET_Value =
{
   BLE_SMO_DEC_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;

static const qapi_BLE_GATT_Characteristic_Value_128_Entry_t  BLE_BULB_Value =
{
   BLE_BULB_DEC_CHARACTERISTIC_UUID_CONSTANT,
   0,
   NULL
} ;
   /* Client Characteristic Configuration Descriptor. */
static qapi_BLE_GATT_Characteristic_Descriptor_16_Entry_t Lock_Client_Characteristic_Configuration =
{
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};

   /* Client Characteristic Configuration Descriptor. */
static qapi_BLE_GATT_Characteristic_Descriptor_16_Entry_t Smo_Det_Client_Characteristic_Configuration =
{
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};

static qapi_BLE_GATT_Characteristic_Descriptor_16_Entry_t Bulb_Client_Characteristic_Configuration =
{
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   QAPI_BLE_GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};


   /* The following defines the HOME AUTOMATION service that is registered with   */
   /* the qapi_BLE_GATT_Register_Service function call.               */
   /* * NOTE * This array will be registered with GATT in the call to */
   /*          qapi_BLE_GATT_Register_Service.                        */
const qapi_BLE_GATT_Service_Attribute_Entry_t BLE_IO_Service[] =
{
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_PRIMARY_SERVICE_128_E,            (uint8_t *)&BLE_LOCK_Service_UUID           },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_128_E, (uint8_t *)&BLE_LOCK_CHAR_Declaration            },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_128_E,       (uint8_t *)&BLE_LOCK_Value                  },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E,   (uint8_t *)&Lock_Client_Characteristic_Configuration},
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_128_E, (uint8_t *)&BLE_SMO_DET_CHAR_Declaration            },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_128_E,       (uint8_t *)&BLE_SMO_DET_Value                  },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E,   (uint8_t *)&Smo_Det_Client_Characteristic_Configuration},
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE,          QAPI_BLE_AET_CHARACTERISTIC_DECLARATION_128_E, (uint8_t *)&BLE_BULB_CHAR_Declaration            },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_VALUE_128_E,       (uint8_t *)&BLE_BULB_Value                  },
   {QAPI_BLE_GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, QAPI_BLE_AET_CHARACTERISTIC_DESCRIPTOR_16_E,   (uint8_t *)&Bulb_Client_Characteristic_Configuration},

} ;

// Smoke detector module

static uint16_t get_smokedetector_value()
{
    return home_automation_adc(0, 0);
}

// modules to conrol the lock
static void lock()
{
    QCLI_Parameter_t param[5];
    param[0].Integer_Value = 5000;
    param[0].Integer_Is_Valid = true;
    param[1].Integer_Value = 1200;
    param[1].Integer_Is_Valid = true;
    param[2].Integer_Value = 2000;
    param[2].Integer_Is_Valid = true;
    param[3].Integer_Value = 1;
    param[3].Integer_Is_Valid = true;
    param[4].Integer_Value = 1;
    param[4].Integer_Is_Valid = true;
    pwm_driver_test(5, param);
}

static void unlock()
{
    QCLI_Parameter_t param[5];
    param[0].Integer_Value = 5000;
    param[0].Integer_Is_Valid = true;
    param[1].Integer_Value = 500;
    param[1].Integer_Is_Valid = true;
    param[2].Integer_Value = 2000;
    param[2].Integer_Is_Valid = true;
    param[3].Integer_Value = 1;
    param[3].Integer_Is_Valid = true;
    param[4].Integer_Value = 1;
    param[4].Integer_Is_Valid = true;
    pwm_driver_test(5, param);
}


int BLE_IOService()
{
   qapi_BLE_GATT_Attribute_Handle_Group_t ServiceHandleGroup;
   int Result = 0;

   /* Initialize the Service Handle Group to 0 since we do not    */
   /* require a specific location in the service table.           */
   ServiceHandleGroup.Starting_Handle = 0;
   ServiceHandleGroup.Ending_Handle   = 0;


   if(!qapi_BLE_BSC_LockBluetoothStack(BT_Stk_Id()))
   {
        /* Register the HOME AUTOMATION Service. */
        Result = qapi_BLE_GATT_Register_Service(BT_Stk_Id(), 
           QAPI_BLE_GATT_SERVICE_FLAGS_LE_SERVICE, 
           BLE_LOCK_SERVICE_ATTRIBUTE_COUNT, 
           (qapi_BLE_GATT_Service_Attribute_Entry_t *)BLE_IO_Service, 
           &ServiceHandleGroup, GATT_ServerEventCallback_Home_Automation, 0);
        if(Result > 0)
        {
           //success
           QCLI_Printf(ble_group, "Home Automation service registered.");
        }
        else
        {      
           //failed to register gatt service
           QCLI_Printf(ble_group, "Home Automation service couldn't be registered.");
        }

        /* Unlock the stack. */
        qapi_BLE_BSC_UnLockBluetoothStack(BT_Stk_Id());
   }
   else
      //failed to initiate ble;
       QCLI_Printf(ble_group, "Home Automation-BLE Initialization failed.");
   return Result;

}



void notify_smoke_data(){
    uint16_t Smoke_Result = 0;
    if (notify_thread_flag) {
        ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Smoke_Result, SmokeDetectorValue);
        qapi_BLE_GATT_Handle_Value_Notification(ble_stack_id, service_id, connection_id, BLE_SMO_DET_ATTRIBUTE_OFFSET, sizeof(Smoke_Result), (uint8_t *)&Smoke_Result);
        QCLI_Printf(ble_group, "Smoke data sent: %d\r\n",SmokeDetectorValue);
    } else {
        //QCLI_Printf(ble_group, "Bulb Notification Not enabled/ Device is not connected\r\n");
    }
}

void notify_bulb_state(){
    uint8_t LittleEndianValueBulb = 0;
    if (notify_thread_flag) {
        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueBulb, BulbState);
        qapi_BLE_GATT_Handle_Value_Notification(ble_stack_id, service_id, connection_id, BLE_BULB_ATTRIBUTE_OFFSET, sizeof(LittleEndianValueBulb), (uint8_t *)&LittleEndianValueBulb);
        QCLI_Printf(ble_group, "bulb Data Notification Sent\r\n");
    }else {
        //QCLI_Printf(ble_group, "Smoke Data Notification Not enabled/ Device is not connected\r\n");
    }
}


void notify_thread() 
{    
               SmokeDetectorValue = get_smokedetector_value();
               Sleep(1000);
}

void smoke_sensor_init()
{
    qurt_thread_attr_t Thread_Attribte;
    qurt_thread_t      Thread_Handle;
    int                Result;

      /* Start the main demo thread. */
      qurt_thread_attr_init(&Thread_Attribte);
      qurt_thread_attr_set_name(&Thread_Attribte, "BLE-Notify Thread");
      qurt_thread_attr_set_priority(&Thread_Attribte, PAL_THREAD_PRIORITY);      
      qurt_thread_attr_set_stack_size(&Thread_Attribte, PAL_THREAD_STACK_SIZE);
      Result = qurt_thread_create(&Thread_Handle, &Thread_Attribte, notify_thread, NULL);
      if(Result != QURT_EOK)
      {
         QCLI_Printf(ble_group, "Something went wrong with running the BLE-Notify Thread.");
      }
}


static void GATT_ServerEventCallback_Home_Automation(uint32_t BluetoothStackID, qapi_BLE_GATT_Server_Event_Data_t *GATT_ServerEventData, uint32_t CallbackParameter)
{
    static uint8_t                      LockState;  // to store the state of lock
    uint8_t                             LittleEndianValueLock = 0;  // convertion LittleEndian format
    uint16_t                             AttributeOffset;
    uint8_t                             LittleEndianValueBulb = 0;  // convertion LittleEndian format
    uint32_t                             Value;
    static uint32_t                      blb_notification_flag = 0;
    BoardStr_t                           RemoteDeviceAddress;
    
   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_ServerEventData))
   {
        QCLI_Printf(ble_group, " In GATT_ServerEventCallback_Home_Automation \n  ");
      switch(GATT_ServerEventData->Event_Data_Type)
      {

        case QAPI_BLE_ET_GATT_CONNECTION_DEVICE_CONNECTION_E :
                QCLI_Printf(ble_group, " In QAPI_BLE_ET_GATT_CONNECTION_DEVICE_CONNECTION_E \n  ");
                /* conversion to string and print for debugging purpose */
                BD_ADDRToStr (GATT_ServerEventData->Event_Data.GATT_Device_Connection_Data->RemoteDevice,RemoteDeviceAddress);
                QCLI_Printf(ble_group, "   Remote Device Address:   %s.\n", RemoteDeviceAddress); 
                
                break;

         case QAPI_BLE_ET_GATT_SERVER_READ_REQUEST_E:
                /* Verify that the Event Data is valid.                  */
                if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
                {
                    if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset == 0)
                    {
                        /* Determine which request this read is coming for.*/
                        switch(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset)
                        {                     
                            case BLE_LOCK_ATTRIBUTE_OFFSET:
                                if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->RemoteDevice) == -1) { 
                                    qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                                    break;
                                }
                                ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueLock, LockState);
                                qapi_BLE_GATT_Read_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, sizeof(LittleEndianValueLock), (uint8_t *)&LittleEndianValueLock);
                                QCLI_Printf(ble_group, "Reading Lock Status.");
                                break;
                            case BLE_BULB_ATTRIBUTE_OFFSET:
                                if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->RemoteDevice) == -1) { 
                                    qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                                    break;
                                }
                                ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueBulb, BulbState);
                                qapi_BLE_GATT_Read_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, sizeof(LittleEndianValueBulb), (uint8_t *)&LittleEndianValueBulb);
                                QCLI_Printf(ble_group, "Reading the bulb status.");
                                break;
                            default:
                                qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED);
                        }
                    }
                    else
                        qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);

                }
                break;
         case QAPI_BLE_ET_GATT_SERVER_WRITE_REQUEST_E:
            /* Verify that the Event Data is valid.                  */
            if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
            {   

               if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset == 0)
               {
                  /* Cache the Attribute Offset.                     */
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;

                  /* Handle a control point write.  */
                  switch(AttributeOffset)
                  {
                      case BLE_LOCK_ATTRIBUTE_OFFSET:
                          if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice) == -1) {
                              qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                              break;                                      
                          } 
                        LockState = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                        if (LockState != 0) {
                            lock();
                            QCLI_Printf(ble_group, "Locked.\n");
                            LockState = 1;
                            if (BulbState != BULB_STATE_DISCONNECTED) {
                                Sleep(1000);
                                if (BulbState != BULB_STATE_OFF) {
                                    QCLI_Printf(ble_group, "Bulb is turned off.\n");  
                                    BulbState = BULB_STATE_OFF;
                                    BLBDWriteData(0X00000000);
                                } else {
                                    QCLI_Printf(ble_group, "Bulb is already turned off.\n");
                                }
                            } else {
                                    QCLI_Printf(ble_group, "Bulb is not connected.\n");
                            }
                        } else {
                            unlock();
                            QCLI_Printf(ble_group, "Unlocked.\n");
                            LockState = 0;
                            if (BulbState != BULB_STATE_DISCONNECTED) {
                                Sleep(1000);
                                if (BulbState != BULB_STATE_ON) {
                                    QCLI_Printf(ble_group, "Bulb is turned on.\n");  
                                    BulbState = BULB_STATE_ON;
                                    BLBDWriteData(0XBBBBBBBB);
                                } else {
                                    QCLI_Printf(ble_group, "Bulb is already turned on.\n");
                                }
                            } else {
                                    QCLI_Printf(ble_group, "Bulb is not connected.\n");
                            }
                        }
                        //Notify about bulb
                        if(blb_notification_flag) {
                              ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueBulb, BulbState);
                              qapi_BLE_GATT_Handle_Value_Notification(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ServiceID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID, BLE_BULB_ATTRIBUTE_OFFSET, sizeof(LittleEndianValueBulb), (uint8_t *)&LittleEndianValueBulb);
                        } else {
                              QCLI_Printf(ble_group, "Bulb Notification is disabled.\n");
                        }
                        qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        break;
                     case BLE_SMO_DET_CCD_ATTRIBUTE_OFFSET:

                          QCLI_Printf(ble_group, "In BLE_SMO_DET_CCD_ATTRIBUTE_OFFSET\n");
                          if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice) == -1) {
                              qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                              
                            QCLI_Printf(ble_group, "In If condition , encryption is not enabled\n");
                              break;                                      
                          } 
                        /* Handle a CCD write. */
                        Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                        if(Value != QAPI_BLE_GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE) {
                           QCLI_Printf(ble_group, "Smoke Detector - Disabled the notify.");
                           notify_thread_flag = 0;
                        }
                        else {
                              QCLI_Printf(ble_group, "Smoke Detector - Enabled the notify.");
                              ble_stack_id = BluetoothStackID;
                              service_id = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ServiceID;
                              connection_id = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID;
                              notify_thread_flag = 1;
                              //run_thread();
                        }                
                        QCLI_Printf(ble_group, "Reading Smoke Detector Value.");
                        QCLI_Printf(ble_group, "sending the write response for the smoke detector notification write req\n");
                        qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        break;
                     case BLE_BULB_ATTRIBUTE_OFFSET:
                          if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice) == -1) {
                              qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                              break;                                      
                          } 
                        if(BulbState != BULB_STATE_DISCONNECTED) {
                              BulbState = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                              if (BulbState != BULB_STATE_OFF) {
                                    QCLI_Printf(ble_group, "Bulb is turned on.");
                                    BLBDWriteData(0XFFFFFFFF);
                                    BulbState = BULB_STATE_ON;
                              }
                              else {
                                    QCLI_Printf(ble_group, "Bulb is turned off.");
                                    BulbState = BULB_STATE_OFF;
                                    BLBDWriteData(0X00000000);
                              }
                        } else {
                              QCLI_Printf(ble_group, "Bulb is not connected");
                        }
                        //Notify about bulb
                        if(blb_notification_flag) {
                              ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueBulb, BulbState);
                              qapi_BLE_GATT_Handle_Value_Notification(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ServiceID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID, BLE_BULB_ATTRIBUTE_OFFSET, sizeof(LittleEndianValueBulb), (uint8_t *)&LittleEndianValueBulb);
                        } else {
                              QCLI_Printf(ble_group, "Bulb Notification is Disabled.\n");
                        }
                        qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        break;
                     case BLE_BULB_CCD_ATTRIBUTE_OFFSET:
                          QCLI_Printf(ble_group, "In BLE_BULB_CCD_ATTRIBUTE_OFFSET\n");

                          if (CheckEncryptionStatus(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice) == -1) {
                              qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                            QCLI_Printf(ble_group, "In If condition , encryption is not enabled\n");
                              
                              break;                                      
                          } 
                        Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                        if(Value != QAPI_BLE_GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE) {
                              QCLI_Printf(ble_group, "Bulb Notification Disabled.\n");
                              blb_notification_flag = 0;
                        } else {
                              blb_notification_flag = 1;
                              QCLI_Printf(ble_group, "Bulb Notification Enabled. State: %d\n", BulbState);
                              ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&LittleEndianValueBulb, BulbState);
                        }

                        QCLI_Printf(ble_group, "sending the write response for the bulb notification write req\n");
                        qapi_BLE_GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        break;
                     default:
                        qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED);
                        break;
                  }
               }
               else
                  qapi_BLE_GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, QAPI_BLE_ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
           // }
        }
            break;
         default:
            break;
      }
   }
}

