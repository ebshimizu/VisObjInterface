import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy as Math
import plotly.offline as py
import plotly.graph_objs as go

prefix = sys.argv[1]
perp = sys.argv[2]

filename = prefix + ".csv"
tsneOut = prefix + "-tsne.dat"
tsneIn = prefix + "-tsne-in.txt"

f = open(tsneIn, 'w')

labels = []
ids = []
selected = []
traces = []
desc = []
startid = 0

# gather data from csv into proper format for t-sne
with open(filename, 'rb') as csvfile:
	freader = csv.reader(csvfile, delimiter=",")
	i = 0
	previousRow = ""
	currentTrace = []
	for row in freader:
		labels.append(float(row[2]))
		ids.append(i)
		desc.append('Score: ' + row[2] + '<br>Edit: ' + row[4] + '<br>Trace: ' + str(len(traces) + 1) + ' Thread: ' + row[0] + ' Sample: ' + row[1])

		if (row[4] == "START"):
			startId = i
		elif (row[4] == "TERMINAL" or row[4] == "L-M TERMINAL"):
			if (row[5] == "1"):
				selected.append(i)
			traces.append(currentTrace)
			currentTrace = []
		else:
			currentTrace.append(i)

		# elements 6+ are feature vector entries
		f.write(",".join(row[6:]) + "\n")

		i = i + 1

f.close()

# run t-sne
call("python C:/Users/eshimizu/Documents/AttributesInterface/dataviz/bhtsne.py -i " + tsneIn + " -o " + tsneOut + " -d 2 -p " + perp)

# read data from files
Y = Math.loadtxt(tsneOut)

# plot with plotly
allPoints = go.Scatter(
	x = Y[:,0],
	y = Y[:,1],
	mode = 'markers',
	name = 'Samples',
	marker = dict(size=9, color=labels, showscale=True, colorscale=[[0, 'rgb(255,0,0)'], [1, 'rgb(0,0,255)']], colorbar = dict(xanchor="right")),
	text = desc
)

graphData = [allPoints]

# plot each trace as a separate path through the space
p = 1
for trace in traces:
	xs = [Y[startid, 0]]
	ys = [Y[startid, 1]]
	traceText = [desc[startid]]
	for i in trace:
		xs.append(Y[i, 0])
		ys.append(Y[i, 1])
		traceText.append(desc[i])

	newTrace = go.Scatter(
		x = xs,
		y = ys,
		mode = 'lines+markers',
		name = 'trace ' + str(p),
		marker = dict(size=6),
		visible = 'legendonly',
		text = traceText
	)

	graphData.append(newTrace)
	p = p + 1

# get the selected points
selX = []
selY = []
for i in range(0, len(ids)):
	if ids[i] in selected:
		selX.append(Y[i, 0])
		selY.append(Y[i, 1])

selPoints = go.Scatter(
	x = selX,
	y = selY,
	mode = 'markers',
	name = 'Returned Results',
	marker = dict(size=12, color='rgb(0,255,0)')
)

#plot the start point
startPoint = go.Scatter(
	x = [Y[startid, 0]],
	y = [Y[startid, 1]],
	mode = 'markers',
	name = 'Start Location',
	marker = dict(size=12, color='rgb(255,0,255)')
)

graphData.append(selPoints)
graphData.append(startPoint)

layout = go.Layout(
	title=prefix
)

py.plot(dict(data=graphData, layout=layout), filename=prefix + "-graph.html")