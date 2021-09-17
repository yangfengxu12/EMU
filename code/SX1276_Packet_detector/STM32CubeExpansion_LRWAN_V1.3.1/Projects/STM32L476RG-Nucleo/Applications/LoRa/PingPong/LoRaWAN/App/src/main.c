#include <string.h>
#include <stdlib.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "sx1276.h"

//#define RF_FREQUENCY                                (433000000 + 200000)// Hz
//#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define RF_FREQUENCY                                435000000 // Hz
#define LORA_SPREADING_FACTOR                       7// [SF7..SF12]

#define TX_OUTPUT_POWER                             14        // dBm


#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]

#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         3         // Symbols
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

#define LEDS_OFF   do{ \
                   LED_Off( LED_BLUE ) ;   \
                   LED_Off( LED_RED ) ;    \
                   LED_Off( LED_GREEN1 ) ; \
                   LED_Off( LED_GREEN2 ) ; \
                   } while(0) ;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t tar_Buffer[BUFFER_SIZE]={'1','1','1','1','1','1','1'};

uint8_t data_1[21] = {0x80, 0x37, 0x00, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x02, 0x79, 0x91, 0x05, 0x55, 0x29, 0x3b, 0x6b, 0xab, 0xd7, 0x47, 0xc0, 0x49};
uint8_t data_2[21] = {0x80, 0x2c, 0x00, 0x42, 0x00, 0x00, 0x01, 0x00, 0x02, 0xbe, 0x52, 0xf5, 0x2c, 0xe3, 0xf6, 0xc0, 0xe4, 0x11, 0xd1, 0xb1, 0x6b};


States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;
									 
									 
long int received_count=0;
long int Rx_Error_Count=0;
uint32_t reg=0xff;
									 

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
	int Rssi_current[3];
  HAL_Init();

  SystemClock_Config();

  //DBG_Init();

  HW_Init();

  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

  /* Led Timers*/
  TimerInit(&timerLed, OnledEvent);
  TimerSetValue(&timerLed, LED_PERIOD_MS);

  TimerStart(&timerLed);

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
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
	
  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
								
	SX1276SetRx(0);
	SX1276SetMaxPayloadLength( MODEM_LORA, 255 );
	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
	printf("Private 0x12\r\n");

//	SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
//	printf("Public 0x34\r\n");


	printf("FREQ:%d,sf:%d\r\n",RF_FREQUENCY,LORA_SPREADING_FACTOR);
	
	reg=SX1276Read(REG_DIOMAPPING1);
	reg=SX1276Read(REG_DIOMAPPING2);
	reg=SX1276Read(REG_LR_IRQFLAGSMASK);
	reg=SX1276Read(REG_LR_MODEMCONFIG2);
	
	reg=(SX1276Read(REG_LR_FEIMSB)<<16)|(SX1276Read(REG_LR_FEIMID)<<8)|SX1276Read(REG_LR_FEILSB);
	
	printf("LowDatarateOptimize:%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
//	while(1);
	
  while (1)
  {
//		reg=SX1276Read(0x18);
//		if((reg & 0x04) == 0x04)
//		{
////			printf("Rx is going\r\n");

//			if((reg & 0x01) == 0x01)
//			{
//				Rssi_current[0]=SX1276Read(0x1B)-164;
//				printf("-----------------\r\n");
//				printf("1.Detected123\tRssi:%d\r\n",Rssi_current[0]);
//				
//				if( (reg & 0x02) == 0x02 )
//				{
//					Rssi_current[1]=SX1276Read(0x1B)-164;
//					printf("2.Synchronized\tRssi:%d\r\n",Rssi_current[1]);
//					
//					if( (reg & 0x08) == 0x08 )
//					{
//						Rssi_current[2]=SX1276Read(0x1B)-164;
//						printf("3.Header info valid\tRssi:%d\r\n",Rssi_current[2]);
//						printf("avg:%d\r\n",(Rssi_current[0]+Rssi_current[1]+Rssi_current[2])/3);
//					}
//					else
//					{
//						Rssi_current[2]=SX1276Read(0x1B)-164;
//						printf("0.Header info not valid\tRssi:%d\r\n",Rssi_current[2]);
//					}
//				}
//				else				
//				{
//					Rssi_current[1]=SX1276Read(0x1B)-164;
//					printf("0.Not Synchronized\tRssi:%d\r\n",Rssi_current[1]);
//				}
//			}
////			else
////			{
//////				printf("Signal Not detected\r\n");
////			}
//		}
//		else
//		{
//			printf("Rx isn't going\r\n");
//			Radio.Rx(RX_TIMEOUT_VALUE);
//		}
//		DelayMs(100);
//		reg=0x00;
  }
}

void OnTxDone(void)
{
//  Radio.Sleep();
  State = TX;
  printf("OnTxDone\n\r");
}

uint8_t temp;

uint16_t Payload_error=0;
uint16_t packet_error=0;
uint16_t Packet_error_statistic[BUFFER_SIZE]={0};

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  uint8_t i;
//	Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  State = RX;
	
//	printf("\n");
//	for(i=0;i<BufferSize;i++)
//	{
//		printf("%x   ",Buffer[i]);
//	}

	printf("\n");
  printf("OnRxDone, PL = %d, CR = 4/%d, CRC %s\n", \
				BufferSize,((SX1276Read(REG_LR_MODEMSTAT) & 0xe0)>>5)+4, \
				((SX1276Read( REG_LR_HOPCHANNEL )&0x40) > 0)?"ON":"OFF");
//	printf("LowDatarateOptimize:%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
//  printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);
	
	received_count++;
	printf("receive packets count=%ld\n",received_count);
	for(i=0;i<BufferSize;i++)
	{
//		temp = rand()%255;
		temp = 0x31;
		if(Buffer[i] != temp)
		{
			Payload_error++;
			printf("%x-->%x %d\n",temp,Buffer[i],i);			
		}
	}
	if(Payload_error != 0)
	{
		packet_error++;
	}
	Packet_error_statistic[Payload_error]++;
	printf("Packet error:%d,Payload_error:%d\r\n",packet_error,Payload_error);
	if(received_count == 1000)
	{
		for(int tt=0;tt<20;tt++)
		{
			printf("%d=%d\n",tt,Packet_error_statistic[tt]);
		}
	}
	Payload_error = 0;
}

void OnTxTimeout(void)
{
//  Radio.Sleep();
  State = TX_TIMEOUT;

  printf("OnTxTimeout\n\r");
}

void OnRxTimeout(void)
{
//  Radio.Sleep();
  State = RX_TIMEOUT;
  printf("OnRxTimeout\n\r");
}

void OnRxError(void)
{
//  Radio.Sleep();
	State = RX_ERROR;
	Rx_Error_Count++;
  printf("\r\nOnRxError,Count:%d\n\r",Rx_Error_Count);
}

static void OnledEvent(void *context)
{
  LED_Toggle(LED_BLUE) ;
  LED_Toggle(LED_RED1) ;
  LED_Toggle(LED_RED2) ;
  LED_Toggle(LED_GREEN) ;

  TimerStart(&timerLed);
}


int fputc(int ch, FILE *f)
{ 	
	while((USART2->ISR &0X40)==0);//Loop until the end of the transmit
	USART2->TDR  = (uint8_t) ch;  
	return ch;
}

