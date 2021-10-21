#ifndef __HARDLINK_H_
#define __HARDLINK_H_
#include <stdbool.h>
#include <stdint.h>
#include <ti/drivers/rf/RF.h>
#include "smartrf_settings/smartrf_settings.h"
#include "smartrf_settings/smartrf_settings_predefined.h"
#include <stdlib.h>

#include DeviceFamily_constructPath(rf_patches/rf_patch_cpe_sl_longrange.h)
#include DeviceFamily_constructPath(rf_patches/rf_patch_rfe_sl_longrange.h)
#include DeviceFamily_constructPath(rf_patches/rf_patch_mce_sl_longrange.h)
//! \brief HardLink Status and error codes

#define MAX_SEGMENT_SIZE 64

typedef enum
{
    HardLink_status_Success         = 0, //!< Success
    HardLink_status_Config_Error    = 1, //!< Configuration error
    HardLink_status_Param_Error     = 2, //!< Param error
    HardLink_status_Mem_Error       = 3, //!< Memory Error
    HardLink_status_Cmd_Error       = 4, //!< Memory Error
    HardLink_status_Tx_Error        = 5, //!< Tx Error
    HardLink_status_Rx_Error        = 6, //!< Rx Error
    HardLink_status_Rx_Timeout      = 7, //!< Rx Error
    HardLink_status_Rx_Buffer_Error = 8, //!< Rx Buffer Error
    HardLink_status_Busy_Error      = 9, //!< Busy Error
    HardLink_status_Aborted         = 10 //!< Command stopped or aborted
} HardLink_status;

typedef struct HardLink_packet{
    uint8_t payload[MAX_SEGMENT_SIZE];   // max sending size
    size_t size;                         // actual sending size
}* HardLink_packet_t;

typedef void (* HardLink_tx_cb)(HardLink_status);

extern int HardLink_init();
extern int HardLink_receive();
extern int HardLink_send(HardLink_packet_t packet);
//extern int HardLink_sendAsync(HardLink_packet_t packet,HardLink_tx_cb cb);
extern uint32_t HardLink_getFrequency(void);
extern HardLink_status HardLink_setFrequency(uint32_t ui32Frequency);
extern HardLink_status HardLink_getRfPower(int8_t *pi8TxPowerdBm);
extern HardLink_status HardLink_setRfPower(int8_t i8TxPowerdBm);

#endif
