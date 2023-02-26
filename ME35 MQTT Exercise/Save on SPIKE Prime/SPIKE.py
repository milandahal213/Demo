from spike import PrimeHub, LightMatrix, Button, StatusLight, ForceSensor, MotionSensor, Speaker, ColorSensor, App, DistanceSensor, Motor, MotorPair
from spike.control import wait_for_seconds, wait_until, Timer
from math import *
import math

hut = PrimeHub()
from spike import MotorPair
motor_pair = MotorPair('C', 'D')
hut.light_matrix.show_image('SAD')
motor_pair.start(speed=0)

hut.light_matrix.show_image('HAPPY')
import hub
import time
A=hub.port.A
A.mode(hub.port.MODE_FULL_DUPLEX)
time.sleep(0.2)
A.baud(115200)

#change these two items before starting
ip='10.0.0.141'
robotNo='One'

def waittill(keyword):
    ret=''
    ret=A.read(1)
    while not ret.find(keyword)>0:
        ret+=A.read(1)
    print(ret)
    return ret

A.write('\x03')
A.write('\r\n')
waittill(b'>>>')
print("statge1")

A.write('import main\r\n')
waittill(b'>>>')
print("statge2")
A.write('main.setTopic("'+robotNo+'")\r\n')
waittill(b'>>>')
print("statge3")
A.write('main.main("'+ip+'")\r\n')
print("statge4")
waittill(b'listen')
print("begin")
while True:
    waittill(b'start')
    val=waittill(b'end')
    print(val)
    index= val.find(b'\') end')
    val=val[index-1:index]

    if(val==b'0'):
        hut.light_matrix.show_image('HAPPY')
        pass
    elif(val==b'1'):
        motor_pair.move(1, 'seconds', steering=0)
        print("Go forward")
    elif(val==b'2'):
        motor_pair.move(30, 'degrees', steering=100)
        print("Go Left")
    elif(val==b'3'):
        motor_pair.move(30, 'degrees', steering=-100)
        print("Go Right")
    elif(val==b'4'):
        motor_pair.move(-5, 'cm', steering=0)
        print("Go LED")
