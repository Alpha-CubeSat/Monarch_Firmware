# ChipSat Radio Lib API References

* ## Overview
This document lists and explains the application programmable interface of ChipSat Radio Lib, a lib aimed to provide a high fault tolerant data transmission interface on Ti CC1310 chip.

* ## Global Parameters
Communication frequency  = 915 MHZ

Communication power level = 14

Modulation type: GMSK

BytesAmountOneBitEncodedTo = 64

* ## Structures

### HardLink_status
```
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
```
#### Description: 

This enumeration defines the possible return values of the API functions.

### HardLink_packet_t
```
typedef struct HardLink_packet{
    uint8_t payload[MAX_SEGMENT_SIZE];
    size_t size;
}* HardLink_packet_t;
```
#### Description: 
This structure defines the packet to be transmitted. Note that the payload buffer is pre-allocated.

* ## API References
### HardLink_status HardLink_init(void)

#### Description: 

Initialize HardLink lib. Including RF core software init, command chain init. Does not physically turn on RF device. Setting frequency is necessary after initialization in order to send packets properly. 

#### Parameters:

None

#### Return value:
HardLink_status_Success for a successful init.

### HardLink_status HardLink_send(HardLink_packet_t packet)

#### Description:

Encode and send a packet, blocking. The time it takes is approximately (10+82\*packet_size) ms.

#### Parameters:

packet: HardLink_packet_t, pointer to the HardLink_packet to be sent. It can be freed after return. The data size limit is MAX_SEGMENT_SIZE. However, MAX_SEGMENT_SIZE itself can be changed if needed. Please be aware that execution time may be long for large packets.

#### Return value:

HardLink_status_Success for a successful transmission.

### uint32_t HardLink_getFrequency(void)

#### Description:

Return the current frequency of the RF core. Unit is Hz.

#### Parameters:

None

#### Return value:

Current frequency of the RF core.

### HardLink_status HardLink_setFrequency(uint32_t ui32Frequency)
#### Description:

Set the frequency of the RF core. Unit is Hz.

#### Parameters:

ui32Frequency: the frequency to be set.

#### Return value:
HardLink_status_Success for a successful set.

Other HardLink_status for a failed set.

### HardLink_status HardLink_getRfPower(int8_t *pi8TxPowerdBm)
#### Description:

Return the current RF power of the RF core. Unit is dBm.

#### Parameters:

pi8TxPowerdBm: pointer to the variable to store the current RF power.

#### Return value:
HardLink_status_Success for a successful read.

Other HardLink_status for a failed read.


### HardLink_status HardLink_setRfPower(int8_t i8TxPowerdBm)
#### Description:

Set the RF power of the RF core. Unit is dBm.

#### Parameters:

i8TxPowerdBm: the RF power to be set.

#### Return value:
HardLink_status_Success for a successful set.

Other HardLink_status for a failed set.




