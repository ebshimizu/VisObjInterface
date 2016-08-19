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
	variance = []
	diameter = []
	minScene = 5000
	minSceneVals = []
	attrMean = 0
	attrMeanVals = []
	startid = 0

	# gather data
	with open(filename, 'rb') as csvfile:
		freader = csv.reader(csvfile, delimiter=",")
		for row in freader:
			threadId.append(int(row[0]))
			sampleId.append(int(row[1]))
			time.append(float(row[2]))
			attrVal.append(float(row[3]))
			labVal.append(float(row[4]))
			diameter.append(float(row[6]))
			variance.append(float(row[7]))
			desc.append('Score: ' + row[3] + '<br>Display ID:' + str(startid) + ' Thread: ' + row[0] + '<br>Edit History: ' + row[5])

			# minimum attribute scores
			if attrVal[-1] < minScene:
				minScene = attrVal[-1]

			minSceneVals.append(minScene)

			# mean attribute score
			attrMean = attrMean + attrVal[-1]
			attrMeanVals.append(attrMean / len(attrVal))

			startid = startid + 1

	# fit some curves
	attrLine = numpy.polyfit(time, attrVal, 1)
	labLine = numpy.polyfit(time, labVal, 1)

	# plot with plotly
	attrPoints = go.Scatter(
		x = time,
		y = attrVal,
		mode = 'markers',
		name = 'Attribute Score',
		text = desc
	)

	varPoints = go.Scatter(
		x = time,
		y = variance,
		yaxis = 'y2',
		mode = 'lines',
		name = "Variance"
	)

	diamPoints = go.Scatter(
		x = time,
		y = diameter,
		yaxis = 'y2',
		mode = 'lines',
		name = 'Diameter'
	)

	attrLinePlot = go.Scatter(
		x = [time[0], time[-1]],
		y = [attrLine[1] + attrLine[0] * time[0], attrLine[1] + attrLine[0] * time[-1]],
		mode = 'lines',
		name = 'trend',
		text = str(attrLine[1]) + " + " + str(attrLine[0]) + " * x"
	)
	
	# minimum attribute value
	minAttrVal = go.Scatter(
		x = time,
		y = minSceneVals,
		mode = 'lines',
		name = 'Minimum Attribute Value'
	)

	# mean attribute value
	meanAttrVal = go.Scatter(
		x = time,
		y = attrMeanVals,
		mode = 'lines',
		name = 'Mean Attribute Value'
	)

	graphData = [attrPoints, attrLinePlot, varPoints, diamPoints, minAttrVal, meanAttrVal]

	layout = go.Layout(
		title="Attribute Values over Time",
		yaxis2 = dict(overlaying='y', side='right')
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

	graphData = [labPoints, labLinePlot, varPoints, diamPoints]

	layout = go.Layout(
		title="Lab Distances over Time",
		yaxis2 = dict(overlaying='y', side='right')
	)

	py.plot(dict(data=graphData, layout=layout), filename = prefix + "lab-vals.html", auto_open=False)

if __name__ == "__main__":
	main(sys.argv[1])