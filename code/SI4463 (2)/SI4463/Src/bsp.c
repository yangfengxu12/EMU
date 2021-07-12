#include "bsp.h"
#include "spi.h"
//���º궨���������ý��պͷ��䡣Ϊ0ʱ�Ƿ��䷽��Ϊ1ʱ�ǽ��շ����������ӷֱ�������¼��OLED����ʾTX��RX������
#define  APP_TX_RX      1   //0: TX, 1: RX
//#include "si4463.h"
#include "si446x.h"
/*
1��APP_TX_RX����Ϊ0ʱΪ�����������Ϊ��0ʱΪ���ճ���

2����������ܣ���һ�����ݰ������ȴ����շ�����Ӧ������ɹ��յ�Ӧ����ʾ����+1

3�����ճ����ܣ�һֱ�����ڽ���ģʽ���ȴ����䷽�����ݰ����յ������Ƿ���0-9�����ǣ�����ʾ����+1��
������һ��Ӧ�����ݰ���

4������Ĭ�Ͽ�������Ϊ1K��Ƶ��Ϊ433M��ģ�������־��񣬿��Ի�ͨ��һ��Ϊ30M��һ��Ϊ26M�����ɵ�ͷ�ļ���ͬ��
����SI446x.c�ڰ��и��ġ�    ����ʹ��26M����汾����˾�𽥽���̭30M���壬ԭ����һЩ�������ϸ����26M���á�

5������WDS�÷�������www.sliconlabs.com���Ǵ������ͻ�����˾���ṩ���֧�֡�

6�����߶����ߴ���ǳ��ǳ���Ҫ��������90%�������ߣ����ǲ��ϸ�ģ������ýϺõ����ߣ���������������ǲ��ԡ�
����ʹ������볧���ṩ�������ݣ���פ���ȣ�����ȡ�פ����ԽСԽ�ã�����Խ��Խ�ã�����Խ�󣬷�����Խǿ��
��С��פ���Ⱥͽϴ�����棬�ܴ�����Զ�Ĵ�����롣����һ���Ǵ�ֱ������ʹ��ʱ����ر�ֱ��

7���������ߵ粨�Ǹ��ӵģ����ɱ���Ļ��յ������źţ���ʱ������ʹ�������������ѹ�����㷨�����Ӧ���ش���
�����ͨ�ſɿ��Ժʹ�����롣��˾����ģ��߱���Щ���ԣ���SPI��ģ����������öࡣ

8������ر�֤����˲���Դ�Ĳ�С��100mV����ԴƷ�ʶ����ߴ���Ҳ�Ƿǳ���Ҫ�ġ�

9����ע�⣺���߲�Ʒ�Ǿ��������ͣ��벻Ҫ����Ԫ������������ܱ�����������ʱ�����������ýӵء�

10������δ�����ˣ�������˾������Ա��ϵ��
*/
INT8U  buffer[64] = {'0','0','0','0','0',0};
volatile INT32U dly;
INT8U i,TxBuffer[100];
INT8U length, error;
INT16U tx_conter = 0, itmp;
INT16U rx_conter = 0;

uint8_t SPI_ExchangeByte(uint8_t input)
{
  uint8_t output;
  while(HAL_SPI_TransmitReceive(&hspi1,&input,&output,1,1000)!=HAL_OK);
   return output;
}

void SI446x_TX_Test(void)
{
    printf("SI446X:TX " );
    while(1)
    {
        //��ʼ�����飬����շ�����0-9
        for( i = 0; i < PACKET_LENGTH; i ++ )      { buffer[i] = i; }
        //��������
        SI446X_SEND_PACKET( buffer, PACKET_LENGTH, 0, 0 );
        //�ȴ��������
        do{        //Wait for finish a transmitt
            SI446X_INT_STATUS( TxBuffer );
        }while( !( TxBuffer[3] & ( 1<<5 ) ) );
        //�������ģʽ���ȴ�Ӧ���ź�

        SI446X_START_RX( 0, 0, PACKET_LENGTH,8,8,8 );
        //�ȴ�Ӧ��ʱ����Ϊ1000
        dly = 1000;
        while( dly-- )
        {
            //��ȡ�жϼĴ������鿴�Ƿ��յ�һ�����ݰ�
            SI446X_INT_STATUS( buffer );
            if( buffer[3] & ( 1<<4 ) )//�����λΪ1��˵���յ����ݰ�
            {
                length = SI446X_READ_PACKET( buffer );//��ȡ�յ������ݣ��������ֽ���
                for( i = 0, error = 0; i < 10; i ++ )//�ж������Ƿ�����Ӧ���ź�Ӧ��Ϊ10-19
                {
                    if( buffer[i] != i + 10 )
                    {
                        error = 1;
                    }
                }
                if( length == 10 && error == 0 )  //10�ֽ�Ӧ���ź�, ������������ˢ����ʾ����
                {
                    tx_conter ++;
                    itmp = tx_conter;
                    //Display the received count
                    buffer[0] = ( itmp / 10000 ) + '0';
                    itmp %= 10000;
                    buffer[1] = ( itmp / 1000 ) + '0';
                    itmp %= 1000;
                    buffer[2] = ( itmp / 100 ) + '0';
                    itmp %= 100;
                    buffer[3] = ( itmp / 10 ) + '0';
                    itmp %= 10;
                    buffer[4] = itmp + '0';
                    buffer[5] = 0;
                    printf( "%d" ,*buffer );
                    break;
                }
            }
        }
    }
}

void SI446x_RX_Test(void)
{
   printf("SI446X:RX " );
    while( 1 )
    {
        //��ȡ�ж�״̬�����Ƿ��յ�һ�����ݰ�
        SI446X_INT_STATUS( buffer );
        if( buffer[3] & ( 1<<4 ) )
        {
            //�յ�һ�����ݰ�����תLED
            length = SI446X_READ_PACKET( buffer );
for(i=0;i<length;i++)
printf("%d  ",buffer[i]);
printf("\r\n");

//            //�յ���������0-9���Ƚ��Ƿ��յ���ȷ��
//            for( i = 0, error = 0; i < 10; i ++ )
//            {
//                if( buffer[i] != i )
//                {
//                    error = 1;      //������
//                    break;
//                }
//            }
            //���ݳ��Ⱥ����ݶ���ȷ������IF
            if( length == 10 )//&& error == 0 )
            {
                rx_conter ++;
                itmp = rx_conter;

                //Display the received count
                buffer[0] = ( itmp / 10000 ) + '0';
                itmp %= 10000;
                buffer[1] = ( itmp / 1000 ) + '0';
                itmp %= 1000;
                buffer[2] = ( itmp / 100 ) + '0';
                itmp %= 100;
                buffer[3] = ( itmp / 10 ) + '0';
                itmp %= 10;
                buffer[4] = itmp + '0';
                buffer[5] = 0;
                for(i=0;i<5;i++)
                {
                printf("%c",buffer[i] );
                }
                printf("\r\n");
                //Ӧ������Ϊ10-19
                for( i = 0; i < 10; i ++ )
                {
                    buffer[i] = i + 10;
                }
                //����Ӧ���ź�,�ȴ����䷽�ɹ����ٴν������ģʽ
                SI446X_SEND_PACKET( buffer, PACKET_LENGTH, 0, 0 );

                do{              //�ȴ�������ɣ��жϲ�����
                    SI446X_INT_STATUS( buffer );
                }while( !( buffer[3] & ( 1<<5 ) ) );

            }
            //�ص�����ģʽ�������ȴ���һ������
            SI446X_START_RX( 0, 0, PACKET_LENGTH,8,8,8 );
        }
    }
}