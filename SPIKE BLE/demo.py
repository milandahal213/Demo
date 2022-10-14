import mBLE
import json

#function to read all the ports and their types

def getStat():
	a=hub.port.A.info()["type"]
	b=hub.port.B.info()["type"]
	c=hub.port.C.info()["type"]
	d=hub.port.D.info()["type"]
	e=hub.port.E.info()["type"]
	f=hub.port.F.info()["type"]
	portStat=hub.status()["port"]

	state={"A":{"t":a,"data":portStat["A"]},"B":{"t":b,"data":portStat["B"]},"C":{"t":c,"data":portStat["C"]},"D":{"t":d,"data":portStat["D"]},"E":{"t":e,"data":portStat["E"]},"F":{"t":f,"data":portStat["F"]}}
	return state

import time
while True:
	mBLE.receiver.send(json.dumps(getStat()))
	time.sleep(2)