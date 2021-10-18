#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"

#include "sx1276.h"

#define RF_FREQUENCY                                486300000 // Hz


#define TX_OUTPUT_POWER                            14        // dBm


#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define PACKET_TIME																	1000
#define INTEVAL_TIME																3000
#define BUFFER_SIZE                                 10 // Define the payload size here

#define RX_TIMEOUT_VALUE                            1000
#define LED_PERIOD_MS               200

typedef enum
{
  LOWPOWER,
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;

uint8_t data_1[21] = {0x80, 0x37, 0x00, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x02, 0x79, 0x91, 0x05, 0x55, 0x29, 0x3b, 0x6b, 0xab, 0xd7, 0x47, 0xc0, 0x49};
uint8_t data_2[235] = {0x40, 0x4E, 0x00, 0x1D, 0x00, 0x00, 0x02, 0x00, 0x02, 0xBC, 0x78, 0x8B, 0xD3, 0x19, 0x15, 0x85, 0xA6, 0x76, 0x89, 0xD2, 0xFE, 0xE6, 0xFA, 0x57, 0x7E, 0x96, 0x2C, 0x78, 0x74, 0xDF, 0x3B, 0xB3, 0x4A, 0xBA, 0xC5, 0xD4, 0x5C, 0xA2, 0x09, 0xBA, 0xBB, 0xE8, 0x1E, 0x20, 0x1B, 0x70, 0x40, 0x70, 0x78, 0xA9, 0x4B, 0xF9, 0xB2, 0x26, 0xD1, 0x59, 0x7F, 0x96, 0x79, 0x1A, 0x3E, 0xE3, 0xAF, 0xE7, 0x4D, 0x69, 0x8A, 0x16, 0x3C, 0xD5, 0xC8, 0x90, 0xE1, 0xDE, 0xCE, 0xAC, 0x9B, 0x08, 0x73, 0x62, 0xC0, 0xD6, 0xC2, 0x42, 0x9F, 0xCB, 0x9E, 0x3E, 0x63, 0x86, 0xD3, 0x6A, 0x92, 0x4C, 0x3E, 0x43, 0xA8, 0xE9, 0xBE, 0xAD, 0x66, 0x6C, 0x19, 0xED, 0x06, 0x0C, 0xD9, 0xC4, 0xC7, 0x45, 0x3C, 0x99, 0xEB, 0xFC, 0x07, 0x41, 0xF3, 0x70, 0xD7, 0x15, 0xB2, 0xD2, 0xD3, 0x01, 0xD4, 0x23, 0xAB, 0xE1, 0xE4, 0xD7, 0x80, 0x71, 0x27, 0x35, 0xF6, 0x29, 0x0E, 0x30, 0x3F, 0xEA, 0x0C, 0xB9, 0x8D, 0x4A, 0xAC, 0x4F, 0x13, 0xEB, 0xEC, 0x42, 0x65, 0x5B, 0x9F, 0x70, 0x99, 0x34, 0x07, 0x53, 0x65, 0x19, 0x04, 0xA1, 0x0E, 0x99, 0x2A, 0xB1, 0xB0, 0x39, 0xBE, 0x8F, 0xB7, 0x2D, 0x33, 0x61, 0x3A, 0x41, 0xC2, 0xB3, 0xC6, 0x07, 0x32, 0x17, 0x08, 0x07, 0x22, 0x9F, 0xB6, 0x5A, 0xC3, 0x04, 0x1E, 0xA2, 0x2E, 0xC9, 0xFE, 0xC9, 0x5D, 0x11, 0xD3, 0x11, 0xA5, 0x78, 0xE4, 0xC5, 0x17, 0x8E, 0xF5, 0xC9, 0x1C, 0xE1, 0x83, 0xE6, 0xB9, 0xA0, 0x94, 0x0C, 0x8A, 0xB5, 0x75, 0x89, 0xA7, 0x38, 0x04, 0xCB, 0x36, 0xB2, 0x95, 0xF6, 0xC7, 0x6D, 0x5F, 0x2F, 0xD0, 0xC5, 0x1F};

uint32_t Tx_count = 0;

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE] = {0x31};

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  TimerEvent_t timerLed;

/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void);

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout(void);

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout(void);

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError(void);

/*!
 * \brief Function executed on when led timer elapses
 */
static void OnledEvent(void *context);
/**
 * Main application entry point.
 */
 
int main(void)
{
  bool isMaster = true;
  uint8_t i;
	uint32_t Count = 0;

  HAL_Init();

  SystemClock_Config();

  //DBG_Init();

  HW_Init();

  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

//  /* Led Timers*/
////  TimerInit(&timerLed, OnledEvent);
////  TimerSetValue(&timerLed, LED_PERIOD_MS);

////  TimerStart(&timerLed);

  // Radio initialization
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 10000);

//  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
//                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
//                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
//                    0, false, 0, 0, LORA_IQ_INVERSION_ON, true);



//  Radio.Rx(RX_TIMEOUT_VALUE);
	PRINTF("sender\n\r");

	PRINTF("LowDatarateOptimize:%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
	
//	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
//	PRINTF("Private 0x12\r\n");
	
	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
	PRINTF("Public 0x34\r\n");
	for(i = 0; i < BufferSize; i++)
	{
//			Buffer[i] = rand() % 255;
		Buffer[i] = 0x31;
	}
	uint16_t j=0;
  while (j<PACKET_TIME)
  {
//		PRINTF("send\n");
		Radio.Send(data_1, 21);  
		
		j++;
		DelayMs(INTEVAL_TIME);
  }
}

void OnTxDone(void)
{
  Radio.Sleep();
  State = TX;
	Tx_count++;
  PRINTF("OnTxDone,Count:%d\n\r",Tx_count);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
//  Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  State = RX;

  PRINTF("OnRxDone\n\r");
  PRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
}

void OnTxTimeout(void)
{
//  Radio.Sleep();
  State = TX_TIMEOUT;

  PRINTF("OnTxTimeout\n\r");
}

void OnRxTimeout(void)
{
//  Radio.Sleep();
  State = RX_TIMEOUT;
  PRINTF("OnRxTimeout\n\r");
}

void OnRxError(void)
{
//  Radio.Sleep();
  State = RX_ERROR;
  PRINTF("OnRxError\n\r");
}

static void OnledEvent(void *context)
{
  LED_Toggle(LED_BLUE) ;
  LED_Toggle(LED_RED1) ;
  LED_Toggle(LED_RED2) ;
  LED_Toggle(LED_GREEN) ;

  TimerStart(&timerLed);
}

