import sys
import csv
import colorsys
import os
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import plotLib

# The command line args for this are totally weird but here's how it works.
# Everything hinges off of the 3rd argument and whether or not its an integer
# If args[3] is an int the format is: [output file name] [input directory] [folder number]
# If args[3] is not an int the format is: [output file name] [list of directories...]
def main(args):
	searchModeNames = {0: 'MCMC with Edits', 0.1 : 'MCMC with Uniform Edit Weights', 4 : 'Minimizing MCMC with Edits', 5 : 'MCMC with LMGD Refinement', 6 : 'Recentring MCMC', 7 : 'Recentering MCMC with LMGD'}
	searchModes = [0, 0.1, 5, 6, 7]

	arbitrary = False
	try:
		x = int(args[3])
	except ValueError:
		arbitrary = True

	dirList = []
	eventData = []
	if arbitrary == True:
		dirList = args[2:-1]
	else:
		prefix = args[2]
		dirNum = int(args[3])
		for i in searchModes:
			if os.path.exists(prefix + str(i)):
				dirs = os.listdir(prefix + str(i))
				if (os.path.isfile(prefix + str(i) + "/" + dirs[dirNum] + "/results.csv")):
					dirList.append(prefix + str(i) + "/" + dirs[dirNum])
					eventData.append(prefix + str(i) + "/traces/" + dirs[dirNum] + ".csv")

	plots = []

	print "comparing directories:\n" + "\n".join(dirList)

	if (len(dirList) == 0):
		return

	hue = 0
	i = 0
	for path in dirList:
		dirName = path.rpartition('/')[2]
		filename = path + "/results.csv"

		if os.path.isfile(filename):
			rgb = colorsys.hsv_to_rgb(hue, 1.0, 0.7)
			newPlots = plotLib.getPlots(filename, [str(rgb[0] *255), str(rgb[1]*255), str(rgb[2]*255)], eventData[i])
			newPlots = plotLib.renamePlots(newPlots, searchModeNames[searchModes[i]])
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

	reportFile = args[1]
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