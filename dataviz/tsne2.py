import sys
from subprocess import call
import numpy as Math
import plotly.offline as py
import plotly.graph_objs as go

prefix = sys.argv[1]
perp = sys.argv[2]

tsneIn = prefix + "--vectors.txt"
vals = prefix + "--vals.txt"
allIds = prefix + "--ids.txt"
selectedIds = prefix + "-filtered-ids.txt"

tsneOut = prefix + "-tsne.dat"

# run t-sne
call("python C:/Users/falindrith/OneDrive/Documents/research/attributes_project/app/AttributesInterface/dataviz/bhtsne.py -i " + tsneIn + " -o " + tsneOut + " -d 2 -p " + perp)

# read data from files
Y = Math.loadtxt(tsneOut)
labels = Math.loadtxt(vals)
ids = Math.loadtxt(allIds)
selected = Math.loadtxt(selectedIds)

# plot with plotly
# all data
allPoints = go.Scatter(
	x = Y[:,0],
	y = Y[:,1],
	mode = 'markers',
	name = 'Samples',
	marker = dict(size=12, color=labels, showscale=True)
)

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
	name = 'Selected Samples',
	marker = dict(size=24, color='rgb(0,255,0)')
)

graphData = [allPoints, selPoints]

py.plot(graphData, filename=prefix + "-graph.html")
