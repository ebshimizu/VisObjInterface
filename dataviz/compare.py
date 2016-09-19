import sys
import csv
import colorsys
import os
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import plotLib

# The command line args for this are a bit weird but should make sense
# The args array is treated as containing tuples, so [(path + name)] and the
# lenth of args-1 should be divisible by 2
def main(args):
	args = args[1:len(args)]
	if len(args) % 2 != 1:
		print "Error: args mush contain an odd number of elements"
		return

	dirList = []
	nameList = []
	eventData = []
	for i in range(0, ((len(args)-1) / 2)):
		path = args[i * 2].split('/')
		dirname = path[-1]
		containingFolder = '/'.join(path[0:-1])
		dirList.append(args[i * 2] + "/results.csv")
		eventData.append(containingFolder + "/traces/" + dirname + ".csv")
		nameList.append(args[i * 2 + 1])
	
	reportFile = args[-1]
	plots = []

	print "comparing files:\n" + "\n".join(dirList)
	if (len(dirList) == 0):
		return

	hue = 0
	i = 0
	for path in dirList:
		if os.path.isfile(path):
			rgb = colorsys.hsv_to_rgb(hue, 1.0, 0.7)
			newPlots = plotLib.getPlots(path, [str(rgb[0] *255), str(rgb[1]*255), str(rgb[2]*255)], eventData[i])
			newPlots = plotLib.renamePlots(newPlots, nameList[i])
			plots.append(newPlots)
			hue = hue + 0.15
			i = i + 1

	# put all of these into a nice report, or at least try to
	attrPlots = []
	attrTrendPlots = []
	topPlots = []
	labPlots = []
	eventPlots = []
	for plot in plots:
		attrPlots.append(plot['attributePoints'])
		attrTrendPlots.append(plot['attributeTrend'])
		topPlots.append(plot['top25'])
		topPlots.append(plot['minAttrValues'])
		labPlots.append(plot['labPoints'])
		labPlots.append(plot['labTrend'])
		eventPlots.append(plot['events'])


	attrLayout = go.Layout(
		title = "Attribute Value over Time",
	)

	attrTrendsLayout = go.Layout(
		title = "Attribute Value Trends"
	)

	attrStatsLayout = go.Layout(
		title = "Attribute Value Statistics"
	)

	labLayout = go.Layout(
		title = "Lab Value over Time"
	)

	f = open(reportFile, 'w')

	f.write("<html>\n\t<head>\n\t\t<title>Comparison Report</title>")
	f.write('\n\t\t<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.1.0/jquery.min.js"></script>')
	f.write('\n\t</head>\n\t<body>\n')

	f.write('<div id="main" style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=attrPlots+topPlots + eventPlots, layout=attrLayout), output_type='div'))
	f.write('</div>')

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=labPlots, layout=labLayout), output_type='div'))
	f.write('</div>')

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=attrTrendPlots+attrPlots, layout=attrTrendsLayout), output_type='div'))
	f.write('</div>')

	f.write('<div id="stats" style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=topPlots, layout=attrStatsLayout), output_type='div'))
	f.write('</div>')

	#f.write('<div style="display: block; height: 700px;">')
	#f.write(py.plot(dict(data=diamPlots, layout=diamLayout), output_type='div'))
	#f.write('</div>')

	#f.write('<div style="display: block; height: 700px;">')
	#f.write(py.plot(dict(data=varPlots, layout=varLayout), output_type='div'))
	#f.write('</div>')

	f.write("\n\t</body>\n</html>")

if __name__ == "__main__":
	main(sys.argv)