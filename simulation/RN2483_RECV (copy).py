#!/usr/bin/env python
# -*- coding:utf-8 -*-
#encoding=utf-8
"""
Class for controling a RN2483 with python's Serial module
"""

import serial
from time import sleep
import numpy as np
import datetime
import binascii
# import sys
# reload(sys)
# sys.setdefaultencoding('utf8')

receive_count = 0
success_count = 0
error_count = 0
class RN2483(object):
    def __init__(self, port):
        self._ser = serial.Serial(port, 115200, timeout=1)
        self.reset()
        print(self.command('sys get ver'))
        receive_count = 0
        success_count = 0

    def command(self, cmdIn, check=False):
        self._ser.write((cmdIn+'\r\n').encode())
        result = self._ser.readline().splitlines()[0]
        if check and result != b'ok':
            raise Exception('Cmd "%s" responded with "%s"'%(cmdIn, result))
        return result

    def reset(self):
        self.command('sys reset')

    def configLoRa(self,
        mod = 'lora',
        freq = None,
        pwr = -3, #-3 to 15
        sf = 'sf7', #7-12
        crc = 'on', #checksum
        iqi = 'off', #invert
        cr = '4/5', #4/5, 6, 7, 8
        wdt = 0, #0 is disabled
        sync = 0x12, #hex byte sync word
        bw = 125, #125, 250, 500 kHz bandwidth
    ):

        assert(mod == 'lora')
        assert(pwr >= -3 and pwr <= 15)
        assert(sf in ['sf7', 'sf8', 'sf9', 'sf10', 'sf11', 'sf12'])
        assert(crc in ['on', 'off'])
        assert(iqi in ['on', 'off'])
        assert(cr in ['4/5', '4/6', '4/7', '4/8'])
        assert(bw in [125, 250, 500])

        self.command('radio set mod %s'%mod, check=True)
        if freq is not None:
            self.command('radio set freq %d'%int(freq), check=True)
            assert(int(self.command('radio get freq')) == int(freq))
        self.command('radio set pwr %d'%pwr, check=True)
        self.command('radio set sf %s'%sf, check=True)
        self.command('radio set crc %s'%crc, check=True)
        self.command('radio set iqi %s'%iqi, check=True)
        self.command('radio set cr %s'%cr, check=True)
        self.command('radio set wdt %d'%int(wdt), check=True)
        self.command('radio set sync %s'%hex(sync)[2:], check=True)
        self.command('radio set bw %d'%int(bw), check=True)

    def enableCW(self):
        """
        Enable CW, remeber to reset after to use LoRa again.
        """
        self.command('radio cw on')

    def crc32_in_python(self,crc, p, len):
            crc = 0xffffffff & ~crc
            for i in range(len):
                crc = crc ^ p[i]
                for j in range(8):
                    crc = (crc >> 1) ^ (0xedb88320 & -(crc & 1))
            return 0xffffffff & ~crc

    def transmit(self, s):
        self.command('mac pause')
        out = ''.join(['%02x'%ord(c) for c in s])
        return self.command('radio tx %s'%out) == 'ok'


    def receive(self, 
        s,
        freq = None,
        sf = 'sf7', #7-12
        cr = '4/5', #4/5, 6, 7, 8
        bw = 125 #125, 250, 500 kHz bandwidth
        ):
        global receive_count 
        global success_count
        global error_count
        f1 = open("c:/Users/yangf/Desktop/test_noise.txt",'w')
        print("Freq: {}Hz, SF: {}, BW: {}KHz, CR: {} \r\n".format(freq,sf,bw,cr))
        f1.write("Freq: {}Hz, SF: {}, BW: {}KHz, CR: {} \r\n".format(freq,sf,bw,cr))
        self.command('mac pause')
        self.command('radio set wdt 0')
        BRR_incorrect_count = 0
        BRR_total_count = 0
        byte_position_count = np.zeros(255)
        while True:
            if self.command('radio rx 0') == b'ok':
                # print('ok')
                while True:
                    # wait until we RX anything
                    if self._ser.readable():
                        r = self._ser.readline()
                        # print(r)
                        if len(r):
                            # print('<< prx {r}'.format(r=r[:-2]))
                            snr = self.command('radio get snr')
                            receive_count += 1
                            s = r.decode()
                            if s.startswith('radio_rx'):
                                # print(datetime.datetime.utcnow())
                                # we got some data
                                # l = len(s)
                                # print s
                                # bytearray.fromhex("7061756c").decode()
                                h = s[10:][:-2]
                                # #hex to dec
                                def hex2dec(string_num):
                                    return str(int(string_num.upper(),16))

                                print("Rxdone")
                                length = len(h)
                                error_ocr = False
                                BRR_total_count +=length/2
                                # print dec
                                for i in range(0,length,2):
                                    rx_data = hex2dec(h[i]+h[i+1])

                                    if rx_data != '49':
                                        error_ocr = True
                                        byte_position_count[int(i/2)] +=1
                                        BRR_incorrect_count +=1
                                    print(rx_data,end=' ')
                                    rx_data =rx_data + " "
                                    f1.write(rx_data)
                                if error_ocr == True:
                                    error_count += 1
                                    print('error packet')
                                else:
                                    success_count += 1
                                    print('correct packet')
                                print('\n')

                                # d_decoded=''
                                # for i in range(0,length,2):
                                #     tmp = ("{}{}".format(h[i],h[i+1]))   
                                #     rx_data =tmp + " "
                                #     f1.write(rx_data)                                             
                                #     d_decoded +=tmp.decode('hex')

                                f1.write("\n")

                                

                                # # print len(d_decoded)
                                # # print d_decoded
                                # try:
                                #     crc_result = self.crc32_in_python(0, list(bytearray(d_decoded.encode())), len(d_decoded)-8)
                                # except UnicodeDecodeError:
                                #     print("UnicodeDecodeError")
                                #     error_count += 1
                                #     f1.write("UnicodeDecodeError \n")

                                # else:
                                #     crc_result = str(hex(crc_result)[2:])
                                #     while(len(crc_result) < 8):
                                #         crc_result = "0"+crc_result
                                #     # print crc_result

                                #     in_payload__crc = ''
                                #     for i in range(length-16,length,2):
                                #         tmp = ("{}{}".format(h[i],h[i+1]))
                                #         in_payload__crc +=tmp.decode('hex')


                                
                                
                                #     if in_payload__crc == crc_result:
                                        
                                        
                                #         f1.write("Success\n")

                                        

                                        
                                #         # # # print dec
                                #         # for i in range(0,length,2):
                                #         #     rx_data = hex2dec(h[i]+h[i+1])
                                #         #     print(rx_data),
                                #         #     rx_data =rx_data + " "
                                #         #     f1.write(rx_data)

                                        
                                        
                                        
                            
                                #         # ## print hex
                                #         for i in range(0,length,2):
                                #             rx_data = "{}{}".format(h[i],h[i+1])                                                
                                #             # print(rx_data),
                                #             rx_data =rx_data + " "
                                #             print(rx_data)
                                #             f1.write(rx_data)
                
                                        
                                #         # print 
                                #         # f1.write("\n")
            
                                #         success_count += 1
                                #     else:
                                #         print("in_payload__crc: ",in_payload__crc) 
                                #         print("crc_result: ",crc_result) 


                                #         error_count += 1
                                #         f1.write("CRC_Error \n")


                                print("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}".format(length/2,snr,receive_count,success_count,error_count))
                                f1.write("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}\n".format(length/2,snr,receive_count,success_count,error_count))
                                
                                
                                if(receive_count == 1000):
                                    
                                    print(1-BRR_incorrect_count/BRR_total_count)
                                    for l in byte_position_count:
                                        f1.write("{}\n".format(l))
                                    print(byte_position_count)
                                    f1.close()
                                    exit(0)
                                break
                            

                            elif s.startswith('busy'):
                                print('unexpected, the receiver is still busy')
                                break
                                
                            elif s.startswith('invalid_param'):
                                error_count += 1
                                

                                print('unexpected protocol param error. '),
                                print("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}".format(length/2,snr,receive_count,success_count,error_count))
                                f1.write("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}\n".format(length/2,snr,receive_count,success_count,error_count))
                                
                                break
                                
                            elif s.startswith('ok'):
                                pass
                                print('all good')
                                # break
                            else:
                                
                                
                                error_count += 1
                                print('really unexpected stuff. ')
                                print("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}".format(length/2,snr,receive_count,success_count,error_count))
                                f1.write("Size= {}, SNR= {}, receive_count= {}, success_count= {}, error_count= {}\n".format(length/2,snr,receive_count,success_count,error_count))
                                
                                break

        f1.close()
        exit(0)                    

    #TODO receive one...


if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("--port", type="string", dest="port", help="TTY device node", default='COM7')
    parser.add_option("--freq", type="float", dest="freq", help="Tranceiver frequency in Hz", default=868.1e6)
    parser.add_option("--bw", type="float", dest="bw", help="Tranceiver operational BW in Hz", default=125e3)
    parser.add_option("--pwr", type="int", dest="pwr", help="Transmit power in dB [-3 to 15]", default=-3)
    parser.add_option("--sf", type="int", dest="sf", help="Spread factor [7 to 12]", default=7)
    parser.add_option("--crc", action="store_true", dest="crc", help="Specify to use CRC")
    parser.add_option("--cr", type="string", dest="cr", help="Coding rate", default='4/5')
    parser.add_option("--sync", type="string", dest="sync", help="Sync word", default='0x12')
    parser.add_option("--cw", action="store_true", dest="cw", help="Transmit continuous wave")
    parser.add_option("--tx", type="string", dest="tx", help="Transmit the specified message", default=None)
    parser.add_option("--rx", type="string", dest="rx", help="Transmit the specified message", default=None)
    # parser.add_option("--repeat", action="store_true", dest="repeat", help="Repeat transmission")
    (options, args) = parser.parse_args()

    rn2483 = RN2483(options.port)
    rn2483.configLoRa(
        freq=options.freq,
        bw=int(options.bw/1e3),
        pwr=options.pwr,
        sf='sf%d'%options.sf,
        crc='on' if options.crc else 'off',
        cr=options.cr,
        sync=eval(options.sync))

    if options.cw:
        rn2483.enableCW()
        exit(0)

    # if options.tx:
    #     rn2483.transmit(options.tx) #transmit once
    #     while options.repeat: rn2483.transmit(options.tx)

    if options.rx:
        # while True:
        rn2483.receive(
        options.rx,
	    freq=options.freq,
        bw=int(options.bw/1e3),
        sf=options.sf,
        cr=options.cr) 
        

        
