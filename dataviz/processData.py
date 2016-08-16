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
import compare

logFolder = sys.argv[1]

# we specifically look for folders 0, 4, 5 (more added as needed)
searchModes = [0, 5, 6, 7]
searchModeNames = {0: 'MCMC with Edits', 4 : 'Minimizing MCMC with Edits', 5 : 'MCMC with LMGD Refinement', 6 : 'Recentring MCMC', 7 : 'Recentering MCMC with LMGD'}

# collect averages of other data points of interest
avgLab = dict()
avgMinLab = dict()
diameters = dict()
variances = dict()
diamCounts = dict()

for i in searchModes:
	dirs = os.listdir(logFolder + str(i))

	avgLab[i] = 0
	avgMinLab[i] = 0
	diameters[i] = OrderedDict()
	variances[i] = OrderedDict()
	diamCounts[i] = OrderedDict()

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
					diameters[i][float(row[0])] = diameters[i].get(float(row[0]), 0) + float(row[1])
					variances[i][float(row[0])] = variances[i].get(float(row[0]), 0) + float(row[2])
					diamCounts[i][int(row[0])] = diamCounts[i].get(int(row[0]), 0) + 1
				rowIndex = rowIndex + 1

	# compute some averages
	avgLab[i] = avgLab[i] / len(dirs)
	avgMinLab[i] = avgMinLab[i] / len(dirs)

	for key in diameters[i]:
		diameters[i][key] = diameters[i][key] / diamCounts[i][key]
		variances[i][key] = variances[i][key] / diamCounts[i][key]

#comparison graphs
for i in range(0, len(os.listdir(logFolder + str(searchModes[-1])))):
	#this function kinda sucks due to it having a strange command line structure
	compare.main(['', logFolder + "compare-" + str(i) + ".html ", logFolder, str(i)])

# graphs
avgTrace = go.Bar(
	x = searchModeNames.values(),
	y = [avgLab[0], avgLab[4], avgLab[5], avgLab[6], avgLab[7]],
	name = 'Average Lab Result Distance'
)

avgMinLabTrace = go.Bar(
	x = searchModeNames.values(),
	y = [avgMinLab[0], avgMinLab[4], avgMinLab[5], avgMinLab[6], avgMinLab[7]],
	name = 'Average Minimum Lab Result Distance'
)

data = [avgTrace, avgMinLabTrace]
layout = go.Layout(
	barmode = 'group',
	title = 'Average Result Quality'
)

fig = go.Figure(data=data, layout=layout)
#py.plot(fig, filename= logFolder + "summary.html", auto_open=False)

diamData = []
varData = []
for i in searchModes:
	diamPlot = go.Scatter(
		mode = 'markers+lines',
		x = diameters[i].keys(),
		y = diameters[i].values(),
		name = searchModeNames[i]
	)
	varPlot = go.Scatter(
		mode = 'markers+lines',
		name = searchModeNames[i],
		x = variances[i].keys(),
		y = variances[i].values()
	)
	diamData.append(diamPlot)
	varData.append(varPlot)

diamLayout = go.Layout(
	title = 'Average Diameter'
)

varLayout = go.Layout(
	title = 'Average Variance'
)

reportFile = logFolder+"summary.html"
f = open(reportFile, 'w')

f.write("<html>\n\t<head>\n\t\t<title>Search Statistics</title>\n\t</head>\n\t<body>\n")

f.write('<div style="display: block; height: 500px;">')
f.write(py.plot(fig, output_type='div'))
f.write('</div>')

f.write('<div style="display: block; height: 600px;">')
f.write(py.plot(dict(data=diamData, layout=diamLayout), output_type='div'))
f.write('</div>')

f.write('<div style="display: block; height: 600px;">')
f.write(py.plot(dict(data=varData, layout=varLayout), output_type='div'))
f.write('</div>')

f.write("\n\t</body>\n</html>")