#include <ti/drivers/rf/RF.h>
#include <stdio.h>
#include "HardLink.h"
#include "RFQueue.h"
#include "Board.h"
#include <ti/sysbios/knl/Semaphore.h>

#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

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
static rfc_CMD_FS_t HardLink_cmdFs;
/*
static Semaphore_Struct RF_Busy;
static Semaphore_Handle RF_Busy_Handle;
*/
HardLink_tx_cb sendAsync_cb;

unsigned char prs_0[64] = {
  0b00000001, 0b01011110, 0b11010100, 0b01100001, 0b00001011, 0b11110011, 0b00110001, 0b01011100,
  0b01100110, 0b10010010, 0b01011011, 0b00101010, 0b11100000, 0b10100011, 0b00000000, 0b11100001,
  0b10111011, 0b10011111, 0b00110001, 0b11001111, 0b11110111, 0b11000000, 0b10110010, 0b01110101,
  0b10101010, 0b10100111, 0b10100101, 0b00010010, 0b00001111, 0b01011011, 0b00000010, 0b00111101,
  0b01001110, 0b01100000, 0b10001110, 0b00010111, 0b00110100, 0b10000101, 0b01100001, 0b01000101,
  0b00000110, 0b10100010, 0b00110110, 0b00101111, 0b10101001, 0b00011111, 0b11010111, 0b11111101,
  0b10011101, 0b01001000, 0b00011001, 0b00011000, 0b10101111, 0b00110110, 0b10010011, 0b00000000,
  0b00010000, 0b10000101, 0b00101000, 0b00011101, 0b01011100, 0b10101111, 0b01100100, 0b11011010
};

unsigned char prs_1[64] = {
  0b11111101, 0b00111110, 0b01110111, 0b11010101, 0b00100101, 0b11101111, 0b00101100, 0b01101001,
  0b00101010, 0b11101001, 0b00111100, 0b11000100, 0b00000111, 0b10010011, 0b11000101, 0b00000111,
  0b00110111, 0b00011111, 0b01111011, 0b11010001, 0b10111010, 0b00000111, 0b10010000, 0b00110111,
  0b11011111, 0b01011010, 0b11101101, 0b11001000, 0b10001100, 0b01101001, 0b10010111, 0b00101001,
  0b10101100, 0b11011001, 0b11010110, 0b00011010, 0b11010110, 0b10101000, 0b00000101, 0b11010011,
  0b01101010, 0b11001011, 0b11010110, 0b01010010, 0b00111111, 0b11100111, 0b10000010, 0b10000110,
  0b01101110, 0b10011010, 0b01100101, 0b10100110, 0b00101110, 0b01010100, 0b11110100, 0b01111010,
  0b11001011, 0b00101110, 0b01100011, 0b10111111, 0b01010100, 0b11000100, 0b11010100, 0b01010100
};

rfc_CMD_PROP_TX_t RF_cmdTx[8];
uint8_t packet_data[MAX_SEGMENT_SIZE];
size_t packet_length;
size_t packet_current;

int HardLink_init(){
    uint16_t i;
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

    return 0;
}

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    // if state of RX data entry is finished
    if(e & RF_EventRxEntryDone)
    {
        // get current unhandled data entry
        currentDataEntry = RFQueue_getDataEntry();
        
        /* Handle the packet data, located at &currentDataEntry->data:
         * - Length is the first byte with the current configuration
         * - Data starts from the second byte */    
        packetLength      = *(uint8_t*)(&currentDataEntry->data);
        packetDataPointer = (uint8_t*)(&currentDataEntry->data + 1);

        /* Copy the payload + the status byte to the packet variable */
        memcpy(packet, packetDataPointer, (packetLength + 1));

        RFQueue_nextEntry();
    }
    return;
}

void HardLink_sendSync_cb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e){

}

int HardLink_receive()
{
    RF_EventMask terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTx,
                                               RF_PriorityNormal, &callback,
                                               RF_EventRxEntryDone);
    while(1);
}

int HardLink_sendAsync(HardLink_packet_t packet){
    //malloc has a risk of memory leaking

    if(!packet || !packet->payload){
        return -1;
    }

    size_t i;
    for(i=0; i<packet->size;i++){
        uint8_t byte = packet->payload[i];
        uint8_t digit;
        for(digit=0;digit<8;digit++){
            if(byte & 1 << digit){
                RF_cmdTx[8*i+digit].pPkt = prs_1;
            }
            else{
                RF_cmdTx[8*i+digit].pPkt = prs_0;
            }
            RF_cmdTx[8*i+digit].pNextOp=&RF_cmdTx[8*i+digit+1];
        }
    }
    RF_cmdTx[8*i-1].pNextOp=0;
    //Semaphore_pend(RF_Busy_Handle);
    RF_EventMask terminationReason = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdTx[0],RF_PriorityNormal, NULL, HARDLINK_RF_EVENT_MASK);

    return 0;
}
uint8_t asd[8];
void pack_commands(){
    uint8_t byte = packet_data[packet_current];
    uint8_t digit;
    uint16_t i;

    for(i=0;i<8;i++){
               RF_cmdTx[i].commandNo = 0x3801;
               RF_cmdTx[i].status = 0x0000;
               RF_cmdTx[i].pNextOp = 0; // INSERT APPLICABLE POINTER: (uint8_0x00000000t*)&xxx
               RF_cmdTx[i].startTime = RF_convertMsToRatTicks(10);
               RF_cmdTx[i].startTrigger.triggerType = TRIG_REL_PREVEND;
               RF_cmdTx[i].startTrigger.bEnaCmd = 0x0;
               RF_cmdTx[i].startTrigger.triggerNo = 0x0;
               RF_cmdTx[i].startTrigger.pastTrig = 0x0;
               RF_cmdTx[i].condition.rule = 0x0;
               RF_cmdTx[i].condition.nSkip = 0x0;
               RF_cmdTx[i].pktConf.bFsOff = 0x0;
               RF_cmdTx[i].pktConf.bUseCrc = 0x1;
               RF_cmdTx[i].pktConf.bVarLen = 0x1;
               RF_cmdTx[i].pktLen = 1;//bytes_per_raw_bit; // SET APPLICATION PAYLOAD LENGTH
               RF_cmdTx[i].syncWord = 0x930B51DE;
               RF_cmdTx[i].pPkt = 0;
        }
        RF_cmdTx[0].startTrigger.triggerType = TRIG_REL_SUBMIT;
        for(i=0;i<7;i++){
            RF_cmdTx[i].pNextOp = &RF_cmdTx[i+1];
        }
        for(digit=0;digit<8;digit++){
        asd[digit] = 8*packet_current+ digit;
        if(byte & 1 << digit){
            RF_cmdTx[digit].pPkt = prs_1;
        }
        else{
            RF_cmdTx[digit].pPkt = prs_0;
        }
    }


     packet_current+=1;
}

int HardLink_send(HardLink_packet_t packet){
    //malloc has a risk of memory leaking

    if(!packet){
        return -1;
    }

    if(packet->size > MAX_SEGMENT_SIZE){
        return -1;
    }

    packet_length=packet->size;
    packet_current=0;
    memcpy(packet_data,packet->payload,packet->size);

    while(packet_current < packet_length){
        printf("Sending %dth byte\n",packet_current);
        pack_commands();
        //Semaphore_pend(RF_Busy_Handle);
        RF_EventMask terminationReason = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdTx[0],RF_PriorityNormal, NULL, 0);
    }
    //Semaphore_post(RF_Busy_Handle);
    return 0;
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
