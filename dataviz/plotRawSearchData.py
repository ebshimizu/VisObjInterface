import sys
import csv
import colorsys
from collections import OrderedDict
from subprocess import call
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import plotLib

def main(arg1):
	filename = arg1 + '.csv'

	plots = plotLib.getRawPlots(filename, ['19', '103', '226'])

	graphData = [plots['attributePoints'], plots['attributeTrend'], plots['minAttrValues'], plots['top25'], plots['events']]

	layout = go.Layout(
		title="Attribute Values over Time"
		#yaxis2 = dict(overlaying='y', side='right')
	)

	py.plot(dict(data=graphData, layout=layout), filename = filename + "-attr-vals.html", auto_open=False)

if __name__ == "__main__":
	main(sys.argv[1])