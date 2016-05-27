import plotly.offline as py
import plotly.graph_objs as go
import csv
import sys
import colorsys
from collections import OrderedDict

def generateColors(n):
	colors = []
	for x in range(0, n):
		h = 0.05 * (x % 20)
		s = 1 - (0.2 * int(x / 20))
		v = 1 - (0.2 * int(x / 20))
		rgb = colorsys.hsv_to_rgb(h, s, v)
		colors.append("rgb(" + str(rgb[0]*255) + "," + str(rgb[1]*255) + "," + str(rgb[2]*255) + ")")
	return colors

prefix = sys.argv[1]

filename = prefix + ".csv"

data = OrderedDict()
ysamples = []
xsamples = []	# maps x-axis value to sample number (removes the START elements)
adata = { 'x' : [], 'y' : [] } 

# gather data from csv into dictionaries
with open(filename, 'rb') as csvfile:
	freader = csv.reader(csvfile, delimiter=",")
	i = 0
	previousRow = ""
	for row in freader:
		if row[2] in data:
			data[row[2]]['x'].append(i)
			data[row[2]]['y'].append(float(row[0]))
		else:
			data[row[2]] = dict()
			data[row[2]]['x'] = [i]
			data[row[2]]['y'] = [float(row[0])]

		ysamples.append(float(row[0]))

		if row[2] != "START":
			xsamples.append(i)

		if previousRow != row[2] and i != 0:
			data[previousRow]['x'].append(i)
			data[previousRow]['y'].append(None)
		previousRow = row[2]

		adata['x'].append(i)
		adata['y'].append(float(row[1]))
		i = i + 1

selected = []
yselected = []

# find selected element IDs
with open(idfilename, 'rb') as selcsv:
	freader = csv.reader(selcsv, delimiter=",")
	for row in freader:
		selected.append(xsamples[int(row[0])])
		yselected.append(ysamples[xsamples[int(row[0])]])

print selected

graphData = []

# append accept data
atrace = go.Scatter(
	x = adata['x'],
	y = adata['y'],
	name = 'Accept Rate',
	yaxis = 'y2',
	line = dict(color = 'rgba(150, 150, 150, 0.5)'),
	fill = 'tozeroy',
	fillcolor = 'rgba(150, 150, 150, 0.5)'
)

graphData.append(atrace)

# get color and convert to plotly data
colors = generateColors(len(data))
i = 0
for k in data:
	v = data[k]
	trace = go.Scatter(
		x = v['x'],
		y = v['y'],
		name = k,
		connectgaps = False,
		line=dict(color=colors[i])
	)

	if (k == "START"):
		trace['mode'] = 'markers'
		trace['marker'] = { 'size': 15, 'color' : 'rgb(0,0,0)' }

	if (k == "FINAL"):
		trace['mode'] = 'markers'
		trace['marker'] = { 'size' : 10, 'color' : 'rgb(0,0,0)'}

	graphData.append(trace)
	i = i + 1

# create plot for selected elements
selTrace = go.Scatter(
	x = selected,
	y = yselected,
	name = "Selected Samples",
	mode = 'markers',
	type = 'scatter',
	marker = dict(size=10, color='rgb(255, 0, 0)', symbol='star')
)

graphData.append(selTrace)

layout = go.Layout(
	title=filename,
	xaxis = dict(rangeselector=dict(), rangeslider=dict(bordercolor="#000", borderwidth=1), type="-"),
	yaxis = dict(),
	yaxis2 = dict(overlaying='y', side='right', range=[0,1])
)

py.plot(dict(data=graphData, layout=layout), filename=filename.replace(".csv", ".html"))