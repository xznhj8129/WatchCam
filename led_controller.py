#!/usr/bin/python2
import serial
import time
import os
from time import localtime, strftime

def readfile(filename):
    with open(filename, "r") as file:
        x=file.read()
    return x

if not os.path.isfile('.led'):
    print 'Creating control file'
    with open(".led", "w") as file:
        file.write("1")

def timestamp():
    return strftime("%d %b %Y %H:%M:%S", localtime())

serialdevice = "/dev/ttyUSB0"

try:
    ser = serial.Serial(
    port=serialdevice,\
    baudrate=9600,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
    timeout=0)
    line = []
    for c in ser.read():
        if c != '\n':
            line.append(c)
        if c== '\n':
            strline=''
            for i in line:
                strline=strline+i
            print strline
    print 'LED alarm connected'
except Exception as e:
    serial_alert = False
    print 'No serial device found'
    print e
    exit(1)

lastled = 1
while 1:
    try:
        time.sleep(0.5)
        if not os.path.isfile('.led'):
            exit(1)
        ledc = readfile(".led")
        if ledc == "": 
            ledc="1"
        if (ledc[0] == "1") and lastled!= 1:
            try:
                ser.write(b'1\0')
                lastled = 1
                print timestamp(), "Led Off"
            except serial.serialutil.SerialException:
                pass
        elif ledc[0] == "2" and lastled != 2:
            try:
                ser.write(b'2\0')
                lastled = 2
                print timestamp(), "Led On"
            except serial.serialutil.SerialException:
                pass
        elif ledc[0] == "3" and lastled != 3:
            try:
                ser.write(b'3\0')
                lastled = 3
                print timestamp(), "Led Flash"
            except serial.serialutil.SerialException:
                pass

    except KeyboardInterrupt:
        break

ser.write(b'1\0')
ser.close()
#os.remove('.led')
