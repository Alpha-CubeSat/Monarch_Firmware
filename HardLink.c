#include <ti/drivers/rf/RF.h>
#include <stdio.h>
#include "HardLink.h"
#include "RFQueue.h"
#include "Board.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)
extern PIN_Handle pinHandle;
#define RF_convertUsToRatTicks(microseconds) \
    ((uint32_t)(microseconds) * 4)

// Convert milliseconds into RAT ticks.
#define RF_convertMsToRatTicks(milliseconds) \
    RF_convertUsToRatTicks((milliseconds) * 1000)

#define ms_To_RadioTime(ms) (ms*(4000000/1000))
#define bytes_per_raw_bit 64
#define RF_TxPowerTable_INVALID_VALUE 0x3fffff

/* Packet RX Configuration*/
#define DATA_ENTRY_HEADER_SIZE 8  // Constant header size of a generic data entry
#define MAX_LENGTH             30 // Max length byte the radio will accept
#define NUM_DATA_ENTRIES       2  // only two ????
#define NUM_APPENDED_BYTES     2  // 1 byte header + 1 byte status

// RF related
#define HARDLINK_RF_EVENT_MASK  ( RF_EventLastCmdDone | \
             RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled | \
             RF_EventCmdPreempted )

static RF_Object rfObject;
static RF_Handle rfHandle;
static RF_Params rfParams;

/* Receive dataQueue for RF core to fill in data */
static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;
static uint8_t packet[MAX_LENGTH + NUM_APPENDED_BYTES - 1];
static uint8_t
rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                  MAX_LENGTH,
                                                  NUM_APPENDED_BYTES)];

//Indicating that the API is initialized
static uint8_t configured = 1;
//Indicating that the API suspended
static uint8_t suspended = 0;

// local commands, contents will be defined by modulation type
//static rfc_CMD_FS_t HardLink_cmdFs;
/*
static Semaphore_Struct RF_Busy;
static Semaphore_Handle RF_Busy_Handle;
*/
HardLink_tx_cb sendAsync_cb;

    volatile unsigned char prn_0[64] = {
                                        0x0, 0x6e, 0xb5, 0x6a, 0xe8, 0x5e, 0x1c, 0xd1,
                                        0xc9, 0x8d, 0x8c, 0x64, 0x54, 0xef, 0xca, 0xd6,
                                        0x7a, 0xff, 0x60, 0xe2, 0x42, 0x4a, 0xd7, 0x23,
                                        0xb0, 0x64, 0xca, 0x92, 0x77, 0x70, 0xb7, 0x4f,
                                        0xe0, 0x49, 0x2e, 0x48, 0xbd, 0x18, 0x51, 0xd4,
                                        0xd5, 0x31, 0x7d, 0x70, 0xe0, 0xbb, 0x17, 0x78,
                                        0x19, 0xaa, 0x1d, 0x16, 0xf2, 0xfb, 0x9c, 0xc6,
                                        0xd3, 0x5d, 0xfa, 0x24, 0x5b, 0x1, 0xa, 0x7d
      /*0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,*/
    };

volatile  unsigned char prn_1[64] = {
                                     0xff, 0x76, 0xa1, 0xdd, 0x38, 0xbf, 0xfc, 0x63,
                                     0x3, 0xf5, 0x17, 0xbb, 0xc9, 0xdc, 0x46, 0x48,
                                     0xbf, 0xfb, 0x28, 0x3, 0x8b, 0xc4, 0xc4, 0x2a,
                                     0x9, 0x9a, 0x80, 0xfe, 0x72, 0x43, 0x12, 0xed,
                                     0xfc, 0x3c, 0x44, 0x88, 0xa3, 0xc9, 0xcb, 0x70,
                                     0xc, 0x69, 0x63, 0x88, 0x46, 0xc6, 0xbf, 0x2,
                                     0x26, 0x85, 0xe4, 0xe2, 0xf7, 0xe1, 0x5, 0xbd,
                                     0xbe, 0xa, 0xb1, 0x33, 0x1a, 0x59, 0x3d, 0xf9
                                     /*0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff*/
};

volatile  unsigned char preamble[64] = {
                                        0x80, 0x7a, 0x2b, 0x86, 0xd0, 0xcf, 0x8c, 0x3a,
                                        0x66, 0x49, 0xda, 0x54, 0x7, 0xc5, 0x0, 0x87,
                                        0xdd, 0xf9, 0x8c, 0xf3, 0xef, 0x3, 0x4d, 0xae,
                                        0x55, 0xe5, 0xa5, 0x48, 0xf0, 0xda, 0x40, 0xbc,
                                        0x72, 0x6, 0x71, 0xe8, 0x2c, 0xa1, 0x86, 0xa2,
                                        0x60, 0x45, 0x6c, 0xf4, 0x95, 0xf8, 0xeb, 0xbf,
                                        0xb9, 0x12, 0x98, 0x18, 0xf5, 0x6c, 0xc9, 0x0,
                                        0x8, 0xa1, 0x14, 0xb8, 0x3a, 0xf5, 0x26, 0x3b
};

rfc_CMD_PROP_TX_t RF_cmdTx[8];
rfc_CMD_PROP_TX_t RF_preamble;

uint8_t packet_data[MAX_SEGMENT_SIZE];
size_t packet_length;
size_t packet_current;



HardLink_status HardLink_init(){

/*
    Semaphore_construct(&RF_Busy, 1, &semparams);
    RF_Busy_Handle = Semaphore_handle(&RF_Busy);*/
    //Set tx call back to NULL
    sendAsync_cb = NULL;

    RF_Params_init(&rfParams);
    //rfParams.nInactivityTimeout = ms_To_RadioTime(1);

    rfHandle = RF_open(&rfObject, &RF_prop,
                &RF_cmdPropRadioDivSetup, &rfParams);

    /* RX initial part*/
    // initialize data queue including the first data entry
    if(RFQueue_defineQueue(&dataQueue, 
                           rxDataEntryBuffer,
                           sizeof(rxDataEntryBuffer),
                           NUM_DATA_ENTRIES, 
                           MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        // failed to allocate space for all data entries
        while(1);
    }

    /* Modify CMD_PROP_RX command for applications needs */
    // Set the data entry queue for received data
    RF_cmdPropRx.pQueue = &dataQueue;
    // Discard ignored packets from Rx queue
    RF_cmdPropRx.rxConf.bAutoFlushIgnored = 1;
    // Discard packets with CRC error from RX queue
    RF_cmdPropRx.rxConf.bAutoFlushCrcErr = 1;
    // Implement packet length filtering to avoid 
    RF_cmdPropRx.maxPktLen = MAX_LENGTH;
    // go back to sync research 
    RF_cmdPropRx.pktConf.bRepeatOk = 1;
    RF_cmdPropRx.pktConf.bRepeatNok = 1;

    return HardLink_status_Success;
}

void pack_commands(){
    uint8_t byte = packet_data[packet_current];
    uint8_t digit;
    uint16_t i;

    // every command only sends 1 bit raw data
    for(i=0;i<8;i++){
               RF_cmdTx[i].commandNo = 0x3801;
               RF_cmdTx[i].status = 0x0000;
               RF_cmdTx[i].pNextOp = 0; // INSERT APPLICABLE POINTER: (uint8_0x00000000t*)&xxx
               RF_cmdTx[i].startTrigger.triggerType = TRIG_NOW;
               RF_cmdTx[i].startTrigger.bEnaCmd = 0x0;
               RF_cmdTx[i].startTrigger.triggerNo = 0x0;
               RF_cmdTx[i].startTrigger.pastTrig = 0x1;
               RF_cmdTx[i].condition.rule = 0x0;
               RF_cmdTx[i].condition.nSkip = 0x0;
               RF_cmdTx[i].pktConf.bFsOff = 0x0;
               RF_cmdTx[i].pktConf.bUseCrc = 0x0;
               RF_cmdTx[i].pktConf.bVarLen = 0x0;
               RF_cmdTx[i].pktLen = bytes_per_raw_bit; // SET APPLICATION PAYLOAD LENGTH
               RF_cmdTx[i].syncWord = 0x930B51DE;
               RF_cmdTx[i].pPkt = 0;
        }
        RF_cmdTx[0].startTrigger.triggerType = TRIG_NOW;
        RF_cmdTx[7].condition.rule = 0x1;
        for(i=0;i<7;i++){
            RF_cmdTx[i].pNextOp = &RF_cmdTx[i+1];
            /** RF_cmdTx[i].pNextOp = 0; */
        }
        for(digit=0;digit<8;digit++){
        if(byte & 1 << digit){
            RF_cmdTx[digit].pPkt = prn_1;
        }
        else{
            RF_cmdTx[digit].pPkt = prn_0;
        }
    }
    packet_current+=1;
}

HardLink_status HardLink_send(HardLink_packet_t packet){
    //malloc has a risk of memory leaking
    RF_EventMask terminationReason;

    if(!packet){
        return -1;
    }

    if(packet->size > MAX_SEGMENT_SIZE){
        return -1;
    }

    packet_length=packet->size;
    packet_current=0;
    memcpy(packet_data,packet->payload,packet->size);

    RF_preamble.commandNo = 0x3801;
      RF_preamble.status = 0x0000;
      RF_preamble.pNextOp = 0;
      RF_preamble.startTrigger.triggerType = TRIG_NOW;
      RF_preamble.startTrigger.bEnaCmd = 0x0;
      RF_preamble.startTrigger.triggerNo = 0x0;
      RF_preamble.startTrigger.pastTrig = 0x1;
      RF_preamble.condition.rule = 0x1;
      RF_preamble.condition.nSkip = 0x0;
      RF_preamble.pktConf.bFsOff = 0x0;
      RF_preamble.pktConf.bUseCrc = 0x0;
      RF_preamble.pktConf.bVarLen = 0x0;
      RF_preamble.pPkt = preamble;
      RF_preamble.pktLen = bytes_per_raw_bit; // SET APPLICATION PAYLOAD LENGTH
      RF_preamble.syncWord = 0x930B51DE;

    terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_preamble,RF_PriorityNormal, NULL, 0);

    if(terminationReason != RF_EventLastCmdDone){
        while(1);
    }
    while(packet_current < packet_length){
        pack_commands();

        terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdTx[0],RF_PriorityNormal, NULL, 0);

        if(terminationReason != RF_EventLastCmdDone){
            while(1);
        }
    }
    //Semaphore_post(RF_Busy_Handle);
    return HardLink_status_Success;
}

uint32_t HardLink_getFrequency(void)
{
    uint32_t freq_khz;
    if((!configured) || suspended)
    {
        return HardLink_status_Config_Error;
    }

    freq_khz = RF_cmdFs.frequency * 1000000;
    // recover the fractional part, see setFrequency for definition
    freq_khz += (((uint64_t)RF_cmdFs.fractFreq * 1000000) / 65536);

    return freq_khz;
}

HardLink_status HardLink_setFrequency(uint32_t ui32Frequency)
{
    HardLink_status  status= HardLink_status_Cmd_Error;
    uint16_t centerFreq, fractFreq;

    if((!configured) || suspended)
    {
        return HardLink_status_Config_Error;
    }

    // set the frequency
    // get center frequency
    centerFreq = (uint16_t)(ui32Frequency / 1000000);
    // first calculate the fractional part of the frequency, for example, 
    // ui32Frequency = 1200000, then 200000 should be the fractional part 
    // then use 65536 (2^16) to enlarge the fractional part to avoid being 0
    fractFreq = (uint16_t)(((uint64_t)ui32Frequency - 
                            ((uint64_t)centerFreq * 1000000)) *
                            65536 / 1000000);

    RF_cmdFs.frequency = centerFreq;
    RF_cmdFs.fractFreq = fractFreq;

    // run command to set the frequency, returns RF_EventMask(termination reason)
    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs,
                RF_PriorityNormal, 0, 0);

    // check status 
    if((result & RF_EventLastCmdDone) && (RF_cmdFs.status == DONE_OK))
    {
        status = HardLink_status_Success;
    }
    return status;
}

HardLink_status HardLink_getRfPower(int8_t *pi8TxPowerdBm)
{
    // set a initial value
    int8_t txPowerdBm = 0xff;
    if((!configured) || suspended)
    {
        return HardLink_status_Config_Error;
    }

    uint8_t rfPowerTableSize = 0;
    RF_TxPowerTable_Entry *rfPowerTable = NULL;
    RF_TxPowerTable_Value currValue = RF_getTxPower(rfHandle);

    // value not valid
    if(currValue.rawValue == RF_TxPowerTable_INVALID_VALUE)
    {
        return HardLink_status_Config_Error;
    }
    else
    {
        // if valid, found power of dBm unit in the look up table 
        if(currValue.paType == RF_TxPowerTable_DefaultPA)
        {
            rfPowerTable = (RF_TxPowerTable_Entry *)PROP_RF_txPowerTable;
            rfPowerTableSize = PROP_RF_txPowerTableSize;
        }
        txPowerdBm = RF_TxPowerTable_findPowerLevel(rfPowerTable, currValue);
    }

    *pi8TxPowerdBm = txPowerdBm;
    return HardLink_status_Success;
}

HardLink_status HardLink_setRfPower(int8_t i8TxPowerdBm)
{
    HardLink_status status = HardLink_status_Cmd_Error;

    if((!configured) || suspended)
    {
        return HardLink_status_Config_Error;
    }

    RF_TxPowerTable_Value newValue;

    newValue = RF_TxPowerTable_findValue((RF_TxPowerTable_Entry*)PROP_RF_txPowerTable, i8TxPowerdBm);

    // didn't found a valid value
    if(newValue.rawValue == RF_TxPowerTable_INVALID_VALUE)
    {
        return HardLink_status_Config_Error;
    }

    RF_Stat rfStatus = RF_setTxPower(rfHandle, newValue);
    if(rfStatus == RF_StatSuccess)
    {
        status = HardLink_status_Success;
    }
    else 
    {
        status = HardLink_status_Config_Error;
    }
    return status;
}
