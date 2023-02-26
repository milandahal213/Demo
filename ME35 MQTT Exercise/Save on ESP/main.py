import time
import machine
import ubinascii
 
from secrets import Tufts_Wireless as wifi
import mqtt_CBR

topic_sub = 'Rob1/listen'
topic_pub = 'Rob1/tell'
client_id= ubinascii.hexlify(machine.unique_id())
                                          
mqtt_CBR.connect_wifi(wifi)
led = machine.Pin(2, machine.Pin.OUT)  # 6 for 2040
    
def setTopic(robotNo):
    global topic_sub
    global topic_pub
    topic_sub= robotNo +'/listen'
    topic_pub = robotNo +'/tell'
    
def blink(delay = 0.1):
    led.off()
    time.sleep(delay)
    led.on()
    
def whenCalled(topic, msg):
    print("start",(topic.decode(), msg.decode()),"end")
    blink()
    time.sleep(0.5)
    blink()
        
def main(mqtt_broker):
    fred = mqtt_CBR.mqtt_client(client_id, mqtt_broker, whenCalled)
    fred.subscribe(topic_sub)
    old = 0
    i = 0
    while True:
        try:
            fred.check()
            if (time.time() - old) > 5:
                msg = 'iteration %d' % i
                fred.publish(topic_pub, msg)
                old = time.time()
                i += 1
        except OSError as e:
            print(e)
            fred.connect()
        except KeyboardInterrupt as e:
            fred.disconnect()
            print('done')
            break
    