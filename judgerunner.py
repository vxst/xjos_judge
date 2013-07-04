#!/usr/bin/python
from time import sleep
from subprocess import check_call
from os import getpid
from time import time;

f=open('jr.pid','w')
f.write(str(getpid()))
f.close()
while(1):
	t=time();
	print('Check')
	check_call("./judge")
	dltt=time()-t
	if(dltt<0.2):
		sleep(0.25)
