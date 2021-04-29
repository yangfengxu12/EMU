import argparse
import serial 
import serial.tools.list_ports
import random
import time
from datetime import datetime
import csv
import numpy as np
import re

# private
import uart

# code version
Version_Tx = 0.1
fixed_path = "/home/yfx/test_data/sensitivity/OK-LoRa/Rx/"
##### LoRa parameters init #######
center_freq = 433000000
tx_power = 14
bandwidth = 125
preamble_length = 8
invert_iq = 0

sync_words = 1234
spread_factor = 7 
coding_rate = 1
CRC = 0
implicit_header =  0
lowdatarateoptimize = 0

packets_times = 10
payload_length = 10

######  get LoRa Parameters
parser = argparse.ArgumentParser()
parser.add_argument('-v','--version', help='Get version')

parser.add_argument('--pt', type=int, default = packets_times, help='Total number of packets, 10 (defalut) (0<= x <= 1000 )')
parser.add_argument('--pl', type=int, default = payload_length, help='Payload length, 10 (defalut) (0<= x <= 255 Bytes)')

parser.add_argument('--freq', type=int, default = center_freq, help='Center frequency(not support for modified), defalut: 433e6(Hz,default)')
parser.add_argument('--tx_p', type=int, default = tx_power, help='Tx power (In OOK, not support for modified)), -2 to 14(default) dbm')
parser.add_argument('--bw', type=int, default = bandwidth, help='Bandwidth(not support for modified), 125 (kHz, default)')
parser.add_argument('--preamble_len', type=int, default = preamble_length, help='Preamble length(not support for modified), 8 (default)')
parser.add_argument('--iq', type=int, default = invert_iq, help='Invert IQ ON/OFF (not support for modified), 0:OFF(default), 1:ON')
parser.add_argument('--sync_wd', type=int, default = sync_words, help='Sync words(not support for modified), 0x12, 0x34(default)')

parser.add_argument('--sf', type=int, default = spread_factor, help='Spreading Factor, 7(default),8,9,10,11,12')
parser.add_argument('--cr', type=int, default = coding_rate, help='Coding Rate,1:(4/5,default), 2(4/6), 3(4/7), 4(4/8)')
parser.add_argument('--crc', type=int, default = CRC, help='CRC ON/OFF, 0:OFF(default), 1:ON')
parser.add_argument('--ih', type=int, default = implicit_header, help='Explicit or implicit header, 0:OFF(explicit,default), 1:ON(implicit)')
parser.add_argument('--ldo', type=int, default = lowdatarateoptimize, help='Low data rate optimize ON/OFF, 0:OFF(default), 1:ON')

args = parser.parse_args()

# packets_times
packets_times = args.pt
if packets_times > 1000:
    print("packets_times is " + packetspackets_times_numbers + ", more than 1000. Set it to 1000")
    packets_times = 1000
elif tx_power < 0:
    print("packets_times is " + packets_times + ", less than 0. Set it to 0")
    packets_times = 0

# payload_length
payload_length = args.pl
if payload_length > 255:
    print("payload_length is " + payload_length + ", more than 255. Set it to 255")
    payload_length = 255
elif tx_power < 0:
    print("payload_length is " + payload_length + ", less than 0. Set it to 0")
    payload_length = 0

# center_freq
center_freq = args.freq
# center_freq = 433e6

#tx power
tx_power = args.tx_p
if tx_power > 14:
    print("Tx power is " + tx_power + ", more than 14. Set it to 14")
    tx_power = 14
elif tx_power < -4:
    print("Tx power is " + tx_power + ", less than -4. Set it to -4")
    tx_power = -4

# bandwidth
bandwidth = args.bw
# bandwidth = 125

# preabmle length
preamble_length = args.preamble_len
# preamble_length = 8

# invert_IQ
invert_iq = args.iq
# invert_IQ = 0

# sync_words
sync_words = args.sync_wd
# sync_words = 1234

# spread factor
spread_factor = args.sf
if spread_factor > 12:
    print("SF is " + spread_factor + ", more than 12. Set it to 12")
    spread_factor = 12
elif spread_factor < 7:
    print("SF is " + spread_factor + ", less than 7. Set it to 7")
    spread_factor = 7
    
# coidng rate
coding_rate = args.cr
if coding_rate > 4:
    print("coding_rate is " + coding_rate + ", more than 4. Set it to 4")
    coding_rate = 4
elif coding_rate < 1:
    print("coding_rate is " + coding_rate + ", less than 1. Set it to 1")
    coding_rate = 1

# CRC
CRC = args.crc
if CRC != 1 and CRC != 0:
    print("CRC is " + str(args.crc) + ", is not 0 or 1. Set it to 0")
    CRC = 0

# implicit_header
implicit_header = args.ih
if implicit_header != 0 and implicit_header != 1:
    print("implicit_header is " + str(implicit_header) + ", is not 0 or 1. Set it to 0")
    implicit_header = 0

# lowdatarateoptimize
lowdatarateoptimize = args.ldo
if lowdatarateoptimize != 0 and lowdatarateoptimize != 1:
    print("lowdatarateoptimize is " + str(lowdatarateoptimize) + ", is not 0 or 1. Set it to 0")
    lowdatarateoptimize = 0

####################################################################
#
#
#   Connect and handshake Tx devices 
#
#
####################################################################
print('')
print('checking devive connection....')


baunRate = 115200

Tx = uart.uart()
Tx.port.reset_input_buffer()
print('handshaking....')
time.sleep(1)
while True:
    Tx.send_cmd('PC:Hello\r\n')
    time.sleep(1)
    str_from_device = Tx.read_cmd()
    print(str_from_device, end = "")
    if str(str_from_device).find('Tx:Hi!') != -1:
        print('received')
        break
time.sleep(1)
print('sending settings and waiting confirmed....')
time.sleep(2)
while True:
    cmd = f'PL{payload_length}_SF{spread_factor}_CR{coding_rate}_CRC{CRC}_IH{implicit_header}_LDO{lowdatarateoptimize}'
    print(cmd)
    Tx.send_cmd(cmd + '\r\n')
    time.sleep(1)
    str_from_device = Tx.read_cmd()
    print(Tx.read_cmd())
    if str_from_device.find(cmd) != -1:
        print('LoRa parameters passed')
        break
    else:
        print('LoRa parameters NOT passed') 

time.sleep(2)


print('')
####################################################################
#
#
#   Write testing paremeters to csv file
#   e.g LoRa paremeters, payloadlength
#       
#
####################################################################
print('')
attenuation_level = 10
# attenuation_level = input('please enter the attenuation level:')

print('attenuation level:' ,attenuation_level, 'dbm')

print('')


csv_file_name = f'AL{attenuation_level}_PL{payload_length}_CF{center_freq/1000000}_TP{tx_power}_SF{spread_factor}_CR{coding_rate}_CRC{CRC}_IH{implicit_header}_LDO{lowdatarateoptimize}.csv'

print('file path:',fixed_path+csv_file_name)

with open(fixed_path+csv_file_name,'w',newline='') as f:
    tx_file = csv.writer(f)
    tx_file.writerow([datetime.now().strftime('%Y-%m-%d %H:%M:%S')])

    tx_file.writerow(['attenuation_level',        attenuation_level ])
    tx_file.writerow([])

    tx_file.writerow(['center_freq',        center_freq ])
    tx_file.writerow(['tx_power',            tx_power ])
    tx_file.writerow(['bandwidth',          bandwidth ])
    tx_file.writerow(['preamble_length',    preamble_length ])
    tx_file.writerow(['invert_iq',          ("ON" if invert_iq else "OFF") ])
    tx_file.writerow(['sync_words',         sync_words ])

    tx_file.writerow(['spread_factor',      spread_factor ])
    tx_file.writerow(['coding_rate',       coding_rate ])
    tx_file.writerow(['CRC',              ("ON" if CRC else "OFF") ])
    tx_file.writerow(['implicit_header',      ("ON" if implicit_header else "OFF") ])
    tx_file.writerow(['lowdatarateoptimize',  ("ON" if lowdatarateoptimize else "OFF") ])

    tx_file.writerow([])
    tx_file.writerow([ 'Received data (Bytes)', 'Payload length', 'coding rate', 'CRC', 'rssi', 'snr', 'packet NO','Received data (list)' ])


print("\n-----------------------------------------------")
print("-----------------------------------------------")
print("attenuation_level:\t"  + str(attenuation_level))

print("center_freq:\t\t"        + str(center_freq))
print("tx_power:\t\t"           + str(tx_power))
print("bandwidth:\t\t"          + str(bandwidth))
print("preamble_length:\t"      + str(preamble_length))
print("invert_iq:\t\t"          + ("ON" if invert_iq else "OFF"))
print("sync_words:\t\t"         + str(sync_words))

print("spread_factor:\t\t"      + str(spread_factor))
print("coding_rate:\t\t"        + str(coding_rate))
print("CRC:\t\t\t"              + ("ON" if CRC else "OFF"))
print("implicit_header:\t"      + ("ON" if implicit_header else "OFF"))
print("lowdatarateoptimize:\t"  + ("ON" if lowdatarateoptimize else "OFF"))
print('')
print("packets_times:\t\t"        + str(packets_times))
print("payload_length:\t\t"       + str(payload_length))

print('file path:',fixed_path+csv_file_name)
print("-----------------------------------------------")
print("-----------------------------------------------\n")
####################################################################
#
#
#   Rx random packets
#
#
####################################################################
packets_counts = 0
Tx.port.reset_input_buffer()
while True:
    serial_data = Tx.port.read_until(expected='Payload',size=None)
    if serial_data != b'':
        # print(serial_data)
        serial_data = list(filter(None,serial_data.split(b'\n')))
        print(serial_data)
        rx_payload_data = serial_data[0].decode('utf-8').split(',')

        rx_payload_length = int(re.findall(r"PL=(.+?)-",serial_data[1].decode('utf-8').split(',')[1])[0])
        rx_coding_rate = int(re.findall(r"/(.+?)-",serial_data[1].decode('utf-8').split(',')[2])[0]) - 4
        rx_crc = 1 if re.findall(r"=(.+?)-",serial_data[1].decode('utf-8').split(',')[3])[0] == 'ON' else 0
        rx_rssi = int(re.findall(r"=(.+?)dBm",serial_data[2].decode('utf-8').split(',')[0])[0])
        rx_snr = int(re.findall(r"=(.+?)dB",serial_data[2].decode('utf-8').split(',')[1])[0])
        with open(fixed_path+csv_file_name,'a+',newline='') as f:
            tx_file = csv.writer(f)

            tx_file.writerow([list(filter(None,rx_payload_data)), rx_payload_length, rx_coding_rate, rx_crc, rx_rssi, rx_snr, packets_counts] + rx_payload_data)
            packets_counts = packets_counts + 1



            
    

cmd = f'END'
Tx.send_cmd(cmd + '\r\n')

print("\n-----------------------------------------------")
print("-----------------------------------------------")
print("attenuation_level:\t"  + str(attenuation_level))

print("center_freq:\t\t"        + str(center_freq))
print("tx_power:\t\t"           + str(tx_power))
print("bandwidth:\t\t"          + str(bandwidth))
print("preamble_length:\t"      + str(preamble_length))
print("invert_iq:\t\t"          + ("ON" if invert_iq else "OFF"))
print("sync_words:\t\t"         + str(sync_words))

print("spread_factor:\t\t"      + str(spread_factor))
print("coding_rate:\t\t"        + str(coding_rate))
print("CRC:\t\t\t"              + ("ON" if CRC else "OFF"))
print("implicit_header:\t"      + ("ON" if implicit_header else "OFF"))
print("lowdatarateoptimize:\t"  + ("ON" if lowdatarateoptimize else "OFF"))
print('')
print("packets_times:\t\t"        + str(packets_times))
print("payload_length:\t\t"       + str(payload_length))

print('file path:',fixed_path+csv_file_name)
print("-----------------------------------------------")
print("-----------------------------------------------\n")