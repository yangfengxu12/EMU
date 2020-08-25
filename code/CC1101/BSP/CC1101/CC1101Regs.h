#ifndef __CC1101_REGS_H__
#define __CC1101_REGS_H__


#define BURST_BASE															0x40
/*!
 * ============================================================================
 * CC1101 Internal registers Address
 * ============================================================================
 */
#define REG_IOCFG2                              0x00
#define REG_IOCFG1                              0x01
#define REG_IOCFG0                              0x02
#define REG_FIFOTHR                             0x03
#define REG_SYNC1                               0x04
#define REG_SYNC0                               0x05
#define REG_PKTLEN                              0x06
#define REG_PKTCTRL1                            0x07
#define REG_PKTCTRL0                            0x08
#define REG_ADDR                               	0x09
#define REG_CHANNR                              0x0A
#define REG_FSCTRL1                             0x0B
#define REG_FSCTRL0                             0x0C
#define REG_FREQ2                               0x0D
#define REG_FREQ1                              	0x0E
#define REG_FREQ0                          		 	0x0F
#define REG_MDMCFG4                             0x10
#define REG_MDMCFG3                             0x11
#define REG_MDMCFG2                             0x12
#define REG_MDMCFG1                             0x13
#define REG_MDMCFG0                             0x14
#define REG_DEVIATN                             0x15
#define REG_MCSM2                               0x16
#define REG_MCSM1                               0x17
#define REG_MCSM0                               0x18
#define REG_FOCCFG                              0x19
#define REG_BSCFG                               0x1A
#define REG_AGCCTRL2                            0x1B
#define REG_AGCCTRL1                            0x1C
#define REG_AGCCTRL0                            0x1D
#define REG_WOREVT1                             0x1E
#define REG_WOREVT0              		            0x1F
#define REG_WORCTRL                  	          0x20
#define REG_FREND1                              0x21
#define REG_FREND0                              0x22
#define REG_FSCAL3                              0x23
#define REG_FSCAL2                              0x24
#define REG_FSCAL1                           	  0x25
#define REG_FSCAL0                           	  0x26
#define REG_RCCTRL1                             0x27
#define REG_RCCTRL0                             0x28
#define REG_FSTEST                         	    0x29
#define REG_PTEST                              	0x2A
#define REG_AGCTEST                             0x2B
#define REG_TEST2                              	0x2C
#define REG_TEST1                              	0x2D
#define REG_TEST0                              	0x2E
#define REG_SRES                           			0x30
#define REG_SFSTXON                          	 	0x31
#define REG_SXOFF                           		0x32
#define REG_SCAL                                0x33
#define REG_SRX                           			0x34
#define REG_STX                              		0x35
#define REG_SIDLE                              	0x36
#define REG_SWOR                              	0x38
#define REG_SPWD                              	0x39
#define REG_SFRX                              	0x3A
#define REG_SFTX                             		0x3B
#define REG_SWORRST                             0x3C
#define REG_SNOP                                0x3D

/*!
 * ============================================================================
 * CC1101 TX FIFO & RX FIFO & PATABLE
 * ============================================================================
 */
#define REG_PATABLE                             0x3E
#define REG_PATABLE_BURST												REG_PATABLE				+		BURST_BASE
#define REG_TX_FIFO                             0x3F
#define REG_TX_FIFO_BURST                       REG_TX_FIFO 			+ 	BURST_BASE
#define REG_RX_FIFO                             REG_TX_FIFO_BURST + 	BURST_BASE
#define REG_RX_FIFO_BURST                       REG_RX_FIFO 			+ 	BURST_BASE

/*!
 * ============================================================================
 * CC1101 Internal registers Address (Read Only)
 * ============================================================================
 */
#define REG_PARTNUM															REG_SRES 					+		BURST_BASE
#define REG_VERSION															REG_SFSTXON				+		BURST_BASE
#define REG_FREQEST 														REG_SXOFF  				+		BURST_BASE
#define REG_LQI   															REG_SCAL 					+		BURST_BASE
#define REG_RSSI   															REG_SRX    				+		BURST_BASE
#define REG_MARCSTATE    												REG_STX    				+		BURST_BASE
#define REG_WORTIME1  													REG_SIDLE  				+		BURST_BASE
#define REG_WORTIME0   													0x37			
#define REG_PKTSTATUS   												REG_SWOR   				+		BURST_BASE			
#define REG_VCO_VC_DAC   												REG_SPWD   				+		BURST_BASE			
#define REG_TXBYTES   													REG_SFRX   				+		BURST_BASE			
#define REG_RXBYTES															REG_SFTX   				+		BURST_BASE			
#define REG_RCCTRL1_STATUS   										REG_SWORRST				+		BURST_BASE			
#define REG_RCCTRL0_STATUS											REG_SNOP   				+		BURST_BASE

#endif 
