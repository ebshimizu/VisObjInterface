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

	if (len(dirs) == 0):
		continue

	for dir in dirs:
		dpath = logFolder + str(i) + "/" + dir + "/"
		print "Processing: " + dpath
		plotSearchData.main(dpath)

		# extract data from stats.csv
		filename = dpath + "stats.csv"
		with open(filename, 'rb') as csvfile:
			freader = csv.reader(csvfile, delimiter=",")
			rowIndex = 0
			for row in freader:
				if (rowIndex == 1):
					avgLab[i] = avgLab[i] + float(row[1])
				if (rowIndex == 3):
					avgMinLab[i] = avgMinLab[i] + float(row[1])
				if (rowIndex >= 6): # this may change when additional data is added
					minTimeToNLab[i][int(row[0])] = minTimeToNLab[i].get(int(row[0]), 0) + float(row[1])
					minSampleToNLab[i][int(row[0])] = minSampleToNLab[i].get(int(row[0]), 0) + float(row[2])
					minTimeToNLabCount[i][int(row[0])] = minTimeToNLabCount[i].get(int(row[0]), 0) + 1
					timeHitNLab[i][int(row[0])] = timeHitNLab[i].get(int(row[0]), 0) + 1
				rowIndex = rowIndex + 1

	# compute some averages
	avgLab[i] = avgLab[i] / len(dirs)
	avgMinLab[i] = avgMinLab[i] / len(dirs)

	for key in minTimeToNLab[i]:
		minTimeToNLab[i][key] = minTimeToNLab[i][key] / minTimeToNLabCount[i][key]
		minSampleToNLab[i][key] = minSampleToNLab[i][key] / minTimeToNLabCount[i][key]

# graphs, eventually
#print avgLab
#print avgMinLab
#print minTimeToNLab
#print minSampleToNLab
#print timeHitNLab

avgTrace = go.Bar(
	x = searchModes,
	y = [avgLab[0], avgLab[4], avgLab[5], avgLab[6]],
	name = 'Average Lab Result Distance'
)

avgMinLabTrace = go.Bar(
	x = searchModes,
	y = [avgMinLab[0], avgMinLab[4], avgMinLab[5], avgMinLab[6]],
	name = 'Average Minimum Lab Result Distance'
)

data = [avgTrace, avgMinLabTrace]
layout = go.Layout(
	barmode = 'group'
)

fig = go.Figure(data=data, layout=layout)
py.plot(fig, filename= logFolder + "summary.html", auto_open=False)