import serial 
import serial.tools.list_ports
import threading
import time


class uart:
    """
    serial client
    """

    def __init__(self):
        self.port = serial.Serial(port='/dev/ttyACM0', baudrate=115200, bytesize=8, parity=serial.PARITY_NONE,
                                  stopbits=serial.STOPBITS_ONE, timeout=1)
        if self.port.isOpen() :
            print("Uart /dev/ttyACM0, open success")
        else :
            print("Uart /dev/ttyACM0, open failed")

    def send_cmd(self, cmd):
        self.port.write(cmd.encode('utf-8'))

    def read_cmd(self):
        getBytes=b''
        while True:
            count = self.port.inWaiting()
            if count > 0:
                data = self.port.read(count)
                return data.decode('utf-8')

    def read_num(self, num):
        response = self.port.read(num)
        return response