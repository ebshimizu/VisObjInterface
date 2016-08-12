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

# collect averages of other data points of interest
avgLab = dict()
avgMinLab = dict()
minTimeToNLab = dict()
minSampleToNLab = dict()
minTimeToNLabCount = dict()
timeHitNLab = dict()

for i in searchModes:
	dirs = os.listdir(logFolder + str(i))

	avgLab[i] = 0
	avgMinLab[i] = 0
	minSampleToNLab[i] = {}
	minTimeToNLab[i] = {}
	minTimeToNLabCount[i] = {}
	timeHitNLab[i] = {}

	for dir in dirs:
		dpath = logFolder + str(i) + "/" + dir + "/"
		print "Processing: " + dpath
		plotSearchData.main(dpath)

		# extract data from stats.csv
		filename = dpath + "stats.csv"
		with open(filename, 'rb') as csvfile:
			freader = csv.reader(csvfile, delimiter=",")
			int rowIndex = 0;
			for row in freader:
				if (rowIndex == 0):
					avgLab[i] = avgLab[i] + float(row[1])
				if (rowIndex == 2):
					avgMinLab[i] = avgMinLab[i] + float(row[1])
				if (rowIndex >= 5): # this may change when additional data is added
					minTimeToNLab[i][int(row[0])] = minTimeToNLab[i][int(row[0])] + float(row[1])
					minSampleToNLab[i][int(row[0])] = minSampleToNLab[i][int(row[0])] + int(row[2])
					minTimeToNLabCount[i][int(row[0])] = minTimeToNLabCount[i][int(row[0])] + 1
					timeHitNLab[i][int(row[0])] = timeHitNLab[i][int(row[0])] + 1

	# compute some averages
	avgLab[i] = avgLab[i] / len(dirs)
	avgMinLab[i] = avgMinLab / len(dirs)

	for key in minTimeToNLab:
		minTimeToNLab[i][key] = minTimeToNLab[i][key] / minTimeToNLabCount[i][key]
		minSampleToNLab[i][key] = minSampleToNLab[i][key] / minTimeToNLabCount[i][key]

print avgLab
print avgMinLab
print minTimeToNLab
print minSampleToNLab
print timeHitNLab