#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"

#include "sx1276.h"

#define RF_FREQUENCY                                486000000 // Hz


#define TX_OUTPUT_POWER                            5        // dBm


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



typedef enum
{
  LOWPOWER,
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 255 // Define the payload size here
#define LED_PERIOD_MS               200


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
                    false, 0, 0, LORA_IQ_INVERSION_ON, 10000);

//  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
//                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
//                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
//                    0, false, 0, 0, LORA_IQ_INVERSION_ON, true);



//  Radio.Rx(RX_TIMEOUT_VALUE);
	PRINTF("sender\n\r");

	PRINTF("LowDatarateOptimize:%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
	
	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
	PRINTF("Private 0x12\r\n");
	
//	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
//	PRINTF("Public 0x34\r\n");
	for(i = 0; i < BufferSize; i++)
	{
//			Buffer[i] = rand() % 255;
		Buffer[i] = 0x31;
	}
	uint16_t j=0;
  while (j<500)
  {
//		PRINTF("send\n");
		Radio.Send(Buffer, BufferSize);  
		
		j++;
		DelayMs(480);
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

