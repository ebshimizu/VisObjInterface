import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import plotLib

def main(arg1, arg2 = ''):
	prefix = arg1
	filename = prefix + "results.csv"

	plots = plotLib.getPlots(filename, ['19', '103', '226'], arg2)

	graphData = [plots['attributePoints'], plots['attributeTrend'], plots['minAttrValues'], plots['top25'], plots['events']]

	layout = go.Layout(
		title="Attribute Values over Time"
		#yaxis2 = dict(overlaying='y', side='right')
	)

	py.plot(dict(data=graphData, layout=layout), filename = prefix + "attr-vals.html", auto_open=False)

	graphData = [plots['labPoints'], plots['labTrend']]

	layout = go.Layout(
		title="Lab Distances over Time"
		# yaxis2 = dict(overlaying='y', side='right')
	)

	py.plot(dict(data=graphData, layout=layout), filename = prefix + "lab-vals.html", auto_open=False)

if __name__ == "__main__":
	if len(sys.argv) == 3:
		main(sys.argv[1], sys.argv[2])
	else:
		main(sys.argv[1])