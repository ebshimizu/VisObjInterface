import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy
import plotly.offline as py
import plotly.graph_objs as go

def main(arg1):
	prefix = arg1

	filename = prefix + "results.csv"

	threadId = []
	sampleId = []
	time = []
	attrVal = []
	labVal = []
	desc = []
	startid = 0

	# gather data from csv into proper format for t-sne
	with open(filename, 'rb') as csvfile:
		freader = csv.reader(csvfile, delimiter=",")
		for row in freader:
			threadId.append(int(row[0]))
			sampleId.append(int(row[1]))
			time.append(float(row[2]))
			attrVal.append(float(row[3]))
			labVal.append(float(row[4]))
			desc.append('Score: ' + row[3] + '<br>Thread: ' + row[0] + ' Sample: ' + row[1] + '<br>Edit History: ' + row[5])

	# fit some curves
	attrLine = numpy.polyfit(time, attrVal, 1)
	labLine = numpy.polyfit(time, labVal, 1)

	# plot with plotly
	attrPoints = go.Scatter(
		x = time,
		y = attrVal,
		mode = 'markers',
		name = 'Attribute Value Over Time',
		text = desc
	)

	attrLinePlot = go.Scatter(
		x = [time[0], time[-1]],
		y = [attrLine[1] + attrLine[0] * time[0], attrLine[1] + attrLine[0] * time[-1]],
		mode = 'lines',
		name = 'trend',
		text = str(attrLine[1]) + " + " + str(attrLine[0]) + " * x"
	)

	graphData = [attrPoints, attrLinePlot]

	layout = go.Layout(
		title="Attribute Values over Time"
	)

	py.plot(dict(data=graphData, layout=layout), filename = prefix + "attr-vals.html", auto_open=False)

	labPoints = go.Scatter(
		x = time,
		y = labVal,
		mode = 'markers',
		name = 'Lab Distance Over Time',
		text = desc
	)

	labLinePlot = go.Scatter(
		x = [time[0], time[-1]],
		y = [labLine[1] + labLine[0] * time[0], labLine[1] + labLine[0] * time[-1]],
		mode = 'lines',
		name = 'trend',
		text = str(labLine[1]) + " + " + str(labLine[0]) + " * x"
	)

	graphData = [labPoints, labLinePlot]

	layout = go.Layout(
		title="Lab Distances over Time"
	)

	py.plot(dict(data=graphData, layout=layout), filename = prefix + "lab-vals.html", auto_open=False)

if __name__ == "__main__":
	main(sys.argv[1])