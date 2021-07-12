#include "bsp.h"
#include "spi.h"
//以下宏定义用于配置接收和发射。为0时是发射方，为1时是接收方。两个板子分别配置烧录。OLED将显示TX和RX字样。
#define  APP_TX_RX      1   //0: TX, 1: RX
//#include "si4463.h"
#include "si446x.h"
/*
1、APP_TX_RX配置为0时为发射程序。配置为非0时为接收程序。

2、发射程序功能：发一个数据包，并等待接收方返回应答，如果成功收到应答，显示计数+1

3、接收程序功能：一直运行在接收模式，等待发射方的数据包，收到后检查是否是0-9，若是，则显示计数+1，
并产生一个应答数据包。

4、程序默认空中速率为1K，频率为433M，模块有两种晶振，可以互通。一种为30M，一种为26M，生成的头文件不同。
请在SI446x.c第八行更改。    建议使用26M晶振版本，我司逐渐将淘汰30M晶体，原因是一些技术设计细节用26M更好。

5、关于WDS用法，请上www.sliconlabs.com。非大批量客户，我司不提供相关支持。

6、天线对无线传输非常非常重要，市面上90%以上天线，都是不合格的，建议用较好的天线，可以用网络分析仪测试。
批量使用务必请厂家提供测试数据，如驻波比，增益等。驻波比越小越好，增益越大越好，增益越大，方向性越强。
较小的驻波比和较大的增益，能带来更远的传输距离。天线一般是垂直极化，使用时请与地表垂直。

7、空中无线电波是复杂的，不可避免的会收到干扰信号，此时，可以使用软件纠错、数据压缩等算法，配合应答重传，
以提高通信可靠性和传输距离。我司串口模块具备这些特性，比SPI型模块性能优异得多。

8、请务必保证发射瞬间电源文波小于100mV。电源品质对无线传输也是非常重要的。

9、请注意：无线产品是静电敏感型，请不要触摸元器件，否则可能被击坏，焊接时烙铁必须良好接地。

10、其他未尽事宜，请与我司技术人员联系。
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
        //初始化数组，向接收方发送0-9
        for( i = 0; i < PACKET_LENGTH; i ++ )      { buffer[i] = i; }
        //发射数据
        SI446X_SEND_PACKET( buffer, PACKET_LENGTH, 0, 0 );
        //等待发射完成
        do{        //Wait for finish a transmitt
            SI446X_INT_STATUS( TxBuffer );
        }while( !( TxBuffer[3] & ( 1<<5 ) ) );
        //进入接收模式，等待应答信号

        SI446X_START_RX( 0, 0, PACKET_LENGTH,8,8,8 );
        //等待应答超时限制为1000
        dly = 1000;
        while( dly-- )
        {
            //读取中断寄存器，查看是否收到一个数据包
            SI446X_INT_STATUS( buffer );
            if( buffer[3] & ( 1<<4 ) )//如果该位为1，说明收到数据包
            {
                length = SI446X_READ_PACKET( buffer );//读取收到的数据，并返回字节数
                for( i = 0, error = 0; i < 10; i ++ )//判断数据是否有误，应答信号应该为10-19
                {
                    if( buffer[i] != i + 10 )
                    {
                        error = 1;
                    }
                }
                if( length == 10 && error == 0 )  //10字节应答信号, 且数据无误，则刷新显示内容
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
        //读取中断状态，看是否收到一个数据包
        SI446X_INT_STATUS( buffer );
        if( buffer[3] & ( 1<<4 ) )
        {
            //收到一个数据包，翻转LED
            length = SI446X_READ_PACKET( buffer );
for(i=0;i<length;i++)
printf("%d  ",buffer[i]);
printf("\r\n");

//            //收到的数据是0-9，比较是否收到正确。
//            for( i = 0, error = 0; i < 10; i ++ )
//            {
//                if( buffer[i] != i )
//                {
//                    error = 1;      //错误标记
//                    break;
//                }
//            }
            //数据长度和内容都正确，进入IF
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
                //应答数据为10-19
                for( i = 0; i < 10; i ++ )
                {
                    buffer[i] = i + 10;
                }
                //返回应答信号,等待发射方成功后，再次进入接收模式
                SI446X_SEND_PACKET( buffer, PACKET_LENGTH, 0, 0 );

                do{              //等待发射完成（中断产生）
                    SI446X_INT_STATUS( buffer );
                }while( !( buffer[3] & ( 1<<5 ) ) );

            }
            //回到接收模式，继续等待下一包数据
            SI446X_START_RX( 0, 0, PACKET_LENGTH,8,8,8 );
        }
    }
}