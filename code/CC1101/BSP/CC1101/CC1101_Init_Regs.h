#ifndef __CC1101_INIT_REGS_H__
#define __CC1101_INIT_REGS_H__

#include "stm32l4xx_hal.h"
// Channel spacing = 199.951172 
// Preamble count = 4 
// Packet length = 255 
// Data format = Asynchronous serial mode 
// Data rate = 249.939 
// CRC enable = false 
// Sync word qualifier mode = No preamble/sync 
// Manchester enable = false 
// Whitening = false 
// TX power = 10 
// CRC autoflush = false 
// Deviation = 5.157471 
// Modulation format = ASK/OOK 
// Channel number = 0 
// Packet length mode = Infinite packet length mode 
// Address config = No address check 
// Base frequency = 432.999817 
// Modulated = false 
// Carrier frequency = 432.999817 
// Device address = 0 
// RX filter BW = 58.035714 
// PA ramping = false 
// Rf settings for CC1101
static uint8_t  CC1101_Init_Setting[] = {
    0x0B,  // IOCFG2        GDO2 Output Pin Configuration  0x0B DATA_CLK  0x0A PLL LOCKED
    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
    0x0D,  // IOCFG0        GDO0 Output Pin Configuration	 0x0C for sync  0x0D for async
    0x47,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
    0xD3,  // SYNC1         Sync Word, High Byte
    0x91,  // SYNC0         Sync Word, Low Byte
    0xFF,  // PKTLEN        Packet Length
    0x04,  // PKTCTRL1      Packet Automation Control
    0x32,  // PKTCTRL0      Packet Automation Control
    0x00,  // ADDR          Device Address
    0x00,  // CHANNR        Channel Number
    0x06,  // FSCTRL1       Frequency Synthesizer Control
    0x00,  // FSCTRL0       Frequency Synthesizer Control
    0x10,  // FREQ2         Frequency Control Word, High Byte
    0xB1,  // FREQ1         Frequency Control Word, Middle Byte
    0x3B,  // FREQ0         Frequency Control Word, Low Byte
    0xFC,  // MDMCFG4       Modem Configuration
    0x3B,  // MDMCFG3       Modem Configuration
    0x30,  // MDMCFG2       Modem Configuration
    0x22,  // MDMCFG1       Modem Configuration
    0xF8,  // MDMCFG0       Modem Configuration
    0x15,  // DEVIATN       Modem Deviation Setting
    0x07,  // MCSM2         Main Radio Control State Machine Configuration
    0x30,  // MCSM1         Main Radio Control State Machine Configuration
    0x18,  // MCSM0         Main Radio Control State Machine Configuration
    0x14,  // FOCCFG        Frequency Offset Compensation Configuration
    0x6C,  // BSCFG         Bit Synchronization Configuration
    0x03,  // AGCCTRL2      AGC Control
    0x40,  // AGCCTRL1      AGC Control
    0x92,  // AGCCTRL0      AGC Control
    0x87,  // WOREVT1       High Byte Event0 Timeout
    0x6B,  // WOREVT0       Low Byte Event0 Timeout
    0xFB,  // WORCTRL       Wake On Radio Control
    0x56,  // FREND1        Front End RX Configuration
    0x11,  // FREND0        Front End TX Configuration
    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
    0x2A,  // FSCAL2        Frequency Synthesizer Calibration
    0x00,  // FSCAL1        Frequency Synthesizer Calibration
    0x1F,  // FSCAL0        Frequency Synthesizer Calibration
    0x41,  // RCCTRL1       RC Oscillator Configuration
    0x00,  // RCCTRL0       RC Oscillator Configuration
    0x59,  // FSTEST        Frequency Synthesizer Calibration Control
    0x7F,  // PTEST         Production Test
    0x3F,  // AGCTEST       AGC Test
    0x81,  // TEST2         Various Test Settings
    0x35,  // TEST1         Various Test Settings
    0x09,  // TEST0         Various Test Settings
};

static uint8_t  CC1101_PA_Table_Settings[] = {
		0x00,
		0xC0,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
};


#endif
