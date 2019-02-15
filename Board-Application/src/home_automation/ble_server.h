
#ifndef __BLE_SERVER__
#define __BLE_SERVER__

#define QAPI_USE_BLE

#include "qapi.h"

#define PAL_THREAD_STACK_SIZE                           (3072)
#define PAL_THREAD_PRIORITY                             (10)

#define BLE_OTA_TIMEOUT                               (qurt_timer_convert_time_to_ticks(180000, QURT_TIME_MSEC))

#define BLE_LOCK_SERVICE_ATTRIBUTE_COUNT                  (sizeof(BLE_IO_Service)/sizeof(qapi_BLE_GATT_Service_Attribute_Entry_t))

#define READ_UNALIGNED_BYTE_LITTLE_ENDIAN(_x)   (((uint8_t *)(_x))[0])
#define READ_UNALIGNED_WORD_LITTLE_ENDIAN(_x)   ((uint16_t)((((uint16_t)(((uint8_t *)(_x))[1])) << 8) | ((uint16_t)(((uint8_t *)(_x))[0]))))
#define READ_UNALIGNED_DWORD_LITTLE_ENDIAN(_x)  ((uint32_t)((((uint32_t)(((uint8_t *)(_x))[3])) << 24) | (((uint32_t)(((uint8_t *)(_x))[2])) << 16) | (((uint32_t)(((uint8_t *)(_x))[1])) << 8) | ((uint32_t)(((uint8_t *)(_x))[0]))))

#define ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(_x, _y)       \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(_y));                              \
}

#define ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(_x, _y)       \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(((uint16_t)(_y)) & 0xFF));         \
  ((uint8_t *)(_x))[1] = ((uint8_t)((((uint16_t)(_y)) >> 8) & 0xFF));  \
}

#define ASSIGN_HOST_DWORD_TO_LITTLE_ENDIAN_UNALIGNED_DWORD(_x, _y)     \
{                                                                      \
  ((uint8_t *)(_x))[0] = ((uint8_t)(((uint32_t)(_y)) & 0xFF));         \
  ((uint8_t *)(_x))[1] = ((uint8_t)((((uint32_t)(_y)) >> 8) & 0xFF));  \
  ((uint8_t *)(_x))[2] = ((uint8_t)((((uint32_t)(_y)) >> 16) & 0xFF)); \
  ((uint8_t *)(_x))[3] = ((uint8_t)((((uint32_t)(_y)) >> 24) & 0xFF)); \
}

#define BLE_LOCK_DEC_SERVICE_UUID_CONSTANT          { 0xA8, 0xF3, 0x11, 0x98, 0x82, 0x84, 0x4F, 0x78, 0x86, 0x54, 0x14, 0x66, 0x60, 0xEC, 0xDC, 0x91 }
#define BLE_LOCK_DEC_CHARACTERISTIC_UUID_CONSTANT   { 0x51, 0x71, 0x89, 0x44, 0x98, 0xDA, 0x4B, 0xA8, 0xAB, 0x94, 0x78, 0xE8, 0x71, 0xB6, 0xF9, 0xAA }
#define BLE_SMO_DEC_CHARACTERISTIC_UUID_CONSTANT    { 0x51, 0x71, 0x89, 0x44, 0x98, 0xDA, 0x4B, 0xA8, 0xAB, 0x94, 0x78, 0xE8, 0x71, 0xB6, 0xF9, 0xAB }
#define BLE_BULB_DEC_CHARACTERISTIC_UUID_CONSTANT    { 0x51, 0x71, 0x89, 0x44, 0x98, 0xDA, 0x4B, 0xA8, 0xAB, 0x94, 0x78, 0xE8, 0x71, 0xB6, 0xF9, 0xAC }


#define BLE_LOCK_ATTRIBUTE_OFFSET           2
#define BLE_LOCK_CCD_ATTRIBUTE_OFFSET       3

#define BLE_SMO_DET_ATTRIBUTE_OFFSET        5
#define BLE_SMO_DET_CCD_ATTRIBUTE_OFFSET    6

#define BLE_BULB_ATTRIBUTE_OFFSET        8
#define BLE_BULB_CCD_ATTRIBUTE_OFFSET    9


#define PWM_PHASE_HIGH          1
#define PWM_PHASE_FREQ_HIGH     5000
#define PWM_PHASE_FREQ_MED_1    2000
#define PWM_PHASE_FREQ_MED_2    1200
#define PWM_PHASE_FREQ_LOW      500
#define PWM_PHASE_VALUE_TRUE    true

#define Sleep(msec)    do { \
                              qurt_time_t qtime;\
                              qtime = qurt_timer_convert_time_to_ticks(msec, QURT_TIME_MSEC);\
                              qurt_thread_sleep(qtime);\
                          } while (0)


/**
 * @func : BLE_IOService
 * @Desc : The following function registers the home automation service and its respective characteristics and descriptors */


int BLE_IOService();

/**
 * @func : notify_Smoke_Data
 * @Desc : The following function notifies the smoke data at a specific intervals of time */


void notify_Smoke_Data();

/**
 * @func : notify_Bulb_State
 * @Desc : The following function notifies the bulb state at a specific intervals of time */


void notify_Bulb_State();


enum BULB_STATES {
    
    BULB_STATE_OFF,
    BULB_STATE_ON,
    BULB_STATE_DISCONNECTED,
};

#endif// __BLE_SERVER__
