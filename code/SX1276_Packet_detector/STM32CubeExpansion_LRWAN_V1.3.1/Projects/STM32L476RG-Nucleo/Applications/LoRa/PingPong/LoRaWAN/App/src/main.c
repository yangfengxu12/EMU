#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "sx1276.h"

#include "delay.h"
#include "usart.h"

//#define RF_FREQUENCY                                (433000000 + 400000)// Hz
//#define LORA_SPREADING_FACTOR                       8         // [SF7..SF12]
#define RF_FREQUENCY                                433000000 // Hz
#define LORA_SPREADING_FACTOR                       7        // [SF7..SF12]

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

int PC_center_freq = 433000000;
int PC_tx_power = 14;
int PC_bandwidth = 125;
int PC_preamble_length = 8;
int PC_invert_iq = 0;
		
int PC_sync_words = 1234;
int PC_spread_factor = 7;
int PC_coding_rate = 1;
int PC_CRC = 0;
int PC_implicit_header =  0;
int PC_lowdatarateoptimize = 0;
		
int PC_packets_times = 0;
int PC_payload_length = 0;
									 
uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t tar_Buffer[BUFFER_SIZE]={'1','1','1','1','1','1','1'};

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
  uint16_t len=0;
	int temp=0;
	char * str_header,* substr;
	char *delim = "_";
	
	
	bool isMaster = true;
  uint8_t i;

  HAL_Init();

  SystemClock_Config();

  //DBG_Init();

  HW_Init();
	
//	delay_init(80);
	uart_init(115200);

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
	
	while(1)
	{
		while(1)
		{
			printf("Tx:waiting connection\n");
			if(USART_RX_STA&0x8000)
			{
				if(strstr((char*)USART_RX_BUF,"PC:Hello") != NULL)
				{
					printf("Tx:Hi!\n");
					USART_RX_STA=0;
					break;
				}
				else
				{
					printf("There is no Hello!\n");
				}
				USART_RX_STA=0;
			}
			DelayMs(2000);
		}
			
		DelayMs(500);
		memset(USART_RX_BUF, 0, USART_REC_LEN);
		USART_RX_STA=0;
		while(1)
		{
			printf("Tx:waiting settings...\n");
			if(USART_RX_STA&0x8000)
			{
				if(strstr((char*)USART_RX_BUF,"PL") != NULL)
				{
					printf((char*)USART_RX_BUF);
					USART_RX_STA=0;
					break;
				}
				USART_RX_STA=0;
			}
			DelayMs(2000);
		}
		DelayMs(1000);
		temp = 0;
		str_header = strstr((char*)USART_RX_BUF,"PL");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_payload_length = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"SF");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_spread_factor = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"CR");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-2;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_coding_rate = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"CRC");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_CRC = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"IH");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_implicit_header = temp;
		
		temp = 0;
		str_header = strstr((char*)substr,"LDO");
		substr = strtok(str_header, delim);
		len = strlen(substr);
		for(int i=0;i<len-3;i++)
		{
			temp += (int)((substr[len-1-i] - '0') * pow(10,i));
		}
		PC_lowdatarateoptimize = temp;
		
		printf("\nPayload length:%d,SF:%d,CR:%d,CRC:%d,IH:%d,LDO:%d\n",PC_payload_length,PC_spread_factor,PC_coding_rate,PC_CRC,PC_implicit_header,PC_lowdatarateoptimize);

		
		Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
										LORA_SPREADING_FACTOR, LORA_CODINGRATE,
										LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
										true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
	
		Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, PC_spread_factor,
											LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
											LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
											0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
									
		SX1276SetRx(0);
		SX1276SetMaxPayloadLength( MODEM_LORA, 255 );
		SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
		printf("Private 0x12\n");
		printf("FREQ:%d,sf:%d\n",RF_FREQUENCY,LORA_SPREADING_FACTOR);
		PRINTF("LowDatarateOptimize:%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
		
		USART_RX_STA=0;
		while(1)
		{
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"END") != NULL)
				{
					USART_RX_STA=0;
					break;
				}
			}
			DelayMs(1000);
		}
		
	}
}

void OnTxDone(void)
{
//  Radio.Sleep();
  State = TX;
  PRINTF("OnTxDone\n\r");
}

uint8_t reg_v[10];

uint16_t Payload_error=0;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
  uint8_t i;
//	Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  State = RX;
	
	printf("\n");
	for(i=0;i<BufferSize;i++)
	{
		printf("%x,",Buffer[i]);
	}

	printf("\n");
  printf("OnRxDone, PL=%d-, CR=4/%d-, CRC=%s-\n", \
				BufferSize,((SX1276Read(REG_LR_MODEMSTAT) & 0xe0)>>5)+4, \
				((SX1276Read( REG_LR_HOPCHANNEL )&0x40) > 0)?"ON":"OFF");
//	PRINTF("LowDatarateOptimize=%s\n",((SX1276Read( REG_LR_MODEMCONFIG3 )&0x8) > 0)?"ON":"OFF");
  printf("RssiValue=%ddBm, SnrValue=%ddB\n", rssi, snr);
	
	received_count++;
	printf("receive packets count=%ld\n",received_count);
	for(i=0;i<BufferSize;i++)
	{
		if(Buffer[i] != '1')
		{
			Payload_error++;
//			printf("%d,%x,%x\n",i,Buffer[i],(uint8_t)('1' + i));			
			break;
		}
	}
	printf("Payload error! Count=%d\r\n",Payload_error);
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
	Rx_Error_Count++;
  PRINTF("\r\nOnRxError,Count:%d\n\r",Rx_Error_Count);
}

static void OnledEvent(void *context)
{
  LED_Toggle(LED_BLUE) ;
  LED_Toggle(LED_RED1) ;
  LED_Toggle(LED_RED2) ;
  LED_Toggle(LED_GREEN) ;

  TimerStart(&timerLed);
}


//int fputc(int ch, FILE *f)
//{ 	
//	while((USART2->ISR &0X40)==0);//Loop until the end of the transmit
//	USART2->TDR  = (uint8_t) ch;  
//	return ch;
//}

