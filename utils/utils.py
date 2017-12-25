#!/bin/python3
from optparse import OptionParser 
import os
import subprocess
import signal
import time
import sys
import json



if __name__ == '__main__':
	result = dict()
	print("Current argv :",sys.argv)
	parser = OptionParser(usage="%prog [options]")
	parser.add_option('','--fix',action='store',dest='fix',help='fix imcs')
	(options, args) = parser.parse_args()
	if options.fix:
		os.system('killall server')
		os.system('killall client')
		time.sleep(.5)
		os.system('ipcrm -M '+str(options.fix))
		exit()
