import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy as Math
import plotly.offline as py
import plotly.graph_objs as go

prefix = sys.argv[1]

filename = prefix + "results.csv"

f = open(filename, 'w')

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

f.close()

# plot with plotly
attrPoints = go.Scatter(
	x = time,
	y = attrVal,
	mode = 'lines+markers',
	name = 'Attribute Value Over Time',
	text = desc
)

graphData = [attrPoints]

layout = go.Layout(
	title=prefix
)

py.plot(dict(data=graphData, layout=layout), filename=prefix + "-graph.html")