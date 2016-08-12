import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import os
import plotSearchData

logFolder = sys.argv[1]

# we specifically look for folders 0, 4, 5 (more added as needed)
searchModes = [0, 4, 5, 6]

for i in searchModes:
	dirs = os.listdir(logFolder + str(i))
	for dir in dirs:
		dpath = logFolder + str(i) + "/" + dir + "/"
		print "Processing: " + dpath
		plotSearchData.main(dpath)