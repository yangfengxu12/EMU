#include <string.h>
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include <string.h>
#include "sx1276.h"

#include "delay.h"
#include "usart.h"

#define RF_FREQUENCY                                433000000 // Hz


#define TX_OUTPUT_POWER                            14        // dBm


#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
#define LORA_SPREADING_FACTOR                       8         // [SF7..SF12]
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
 
uint32_t airtime_cal(int bw, int sf, int cr, int pktLen, int crc, int ih, int ldo)
{
	uint32_t airTime = 0;
	// Symbol rate : time for one symbol (secs)
	double rs = bw / ( 1 << sf );
	double ts = 1 / rs;
	// time of preamble
	double tPreamble = ( 8 + 4.25 ) * ts;
	// Symbol length of payload and time
	double tmp = ceil( ( 8 * pktLen - 4 * sf +
											 28 + 16 * crc -
											 ( ih ? 20 : 0 ) ) /
											 ( double )( 4 * ( sf -
											 ( ( ldo > 0 ) ? 2 : 0 ) ) ) ) *
											 ( cr + 4 );
	double nPayload = 8 + ( ( tmp > 0 ) ? tmp : 0 );
	double tPayload = nPayload * ts;
	// Time on air
	double tOnAir = tPreamble + tPayload;
	// return ms secs
	airTime = (uint32_t) floor( tOnAir * 1000 + 0.999 );
	return airTime;
}
 
int main(void)
{
	uint16_t len=0;
	int temp=0;
	char * str_header,* substr;
	char *delim = "_";
	
  bool isMaster = true;
  uint8_t i;
	uint32_t Count = 0;

  HAL_Init();

  SystemClock_Config();

  //DBG_Init();

  HW_Init();
	uart_init(115200);
	while(1)
	{
		/*Disbale Stand-by mode*/
		LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

		/* Led Timers*/
	//  TimerInit(&timerLed, OnledEvent);
	//  TimerSetValue(&timerLed, LED_PERIOD_MS);

	//  TimerStart(&timerLed);

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

		int packets_count = 0;
		
		Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
										PC_spread_factor, PC_coding_rate,
										LORA_PREAMBLE_LENGTH, 
										PC_implicit_header,
										PC_CRC, 0, 0, LORA_IQ_INVERSION_ON, 3000);
			
		
		USART_RX_STA=0;
		ENABLE_IRQ();								
		while(1)
		{
			printf("Tx:waiting payload data...\n");
			if(USART_RX_STA&0x8000)
			{
				len=USART_RX_STA&0x3fff;
				if(strstr((char*)USART_RX_BUF,"END") != NULL)
				{
					break;
				}
				else if(strstr((char*)USART_RX_BUF,"PD") != NULL)
				{
					printf((char*)USART_RX_BUF);
					
					str_header = strstr((char*)USART_RX_BUF,"PD");
					str_header += 2;
//					printf("\n");
					for(int j=0;j<PC_payload_length;j++)
					{
						str_header = strtok(str_header, delim);
						len = strlen(str_header);
						temp = 0;
						for(int i=0;i<len;i++)
						{
							temp += (int)((str_header[len-1-i] - '0') * pow(10,i));
						}
						Buffer[j] = temp;
						str_header = str_header+len+1;

					}
					printf("Tx:transmiting packets");
					
					Radio.Send(Buffer, PC_payload_length);
																											
					DelayMs(1000+airtime_cal(125000, PC_spread_factor, PC_coding_rate, PC_payload_length, PC_CRC, PC_implicit_header, PC_lowdatarateoptimize));

					packets_count++;
					printf("Tx:done, count:%d\n",packets_count);
					USART_RX_STA=0;
				}
			}
			DelayMs(1000);
		}

	}
	
	
	
	
	
}

void OnTxDone(void)
{
//  Radio.Sleep();
//  State = TX;
//	Tx_count++;
//  PRINTF("OnTxDone,Count:%d\n\r",Tx_count);
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

