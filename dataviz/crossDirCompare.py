import sys
import csv
import colorsys
import os
import numpy
import plotly.offline as py
import plotly.graph_objs as go

# Compares a very specific set of results
# args: [base path] [folder index] [search type] [output file]
def main(args):
	searchModeNames = {0: 'MCMC with Edits', 0.1 : 'MCMC with Uniform Edit Weights', 4 : 'Minimizing MCMC with Edits', 5 : 'MCMC with LMGD Refinement', 6 : 'Recentring MCMC', 7 : 'Recentering MCMC with LMGD'}
	searchModes = [0, 0.1, 5, 6, 7]
	folders = ["T10", "T5", "T1", "T0.5", "T0.1", "T0.001"]

	dirList = []
	prefix = args[1]
	dirNum = int(args[2])
	for i in folders:
		if os.path.exists(prefix + i):
			dirs = os.listdir(prefix + i + "/" + args[3])
			if (os.path.isfile(prefix + str(i) + "/" + args[3] + "/" + dirs[dirNum] + "/results.csv")):
				dirList.append(prefix + str(i) + "/" + args[3] + "/" + dirs[dirNum])

	attrPlots = []
	LabPlots = []
	attrTrendPlots = []
	LabTrendPlots = []
	diamPlots = []
	varPlots = []

	print "comparing directories:\n" + "\n".join(dirList)

	if (len(dirList) == 0):
		return

	i = 0
	for path in dirList:
		dirName = path.rpartition('/')[2]
		filename = path + "/results.csv"

		if os.path.isfile(filename):
			threadId = []
			sampleId = []
			time = []
			attrVal = []
			labVal = []
			desc = []
			variance = []
			diameter = []
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
					startid = startid + 1

			# fit some curves
			attrLine = numpy.polyfit(time, attrVal, 1)
			labLine = numpy.polyfit(time, labVal, 1)

			dirName = folders[i]

			# plot with plotly
			attrPoints = go.Scatter(
				x = time,
				y = attrVal,
				mode = 'markers',
				name = dirName,
				text = desc
			)

			varPoints = go.Scatter(
				x = time,
				y = variance,
				yaxis = 'y2',
				mode = 'lines',
				name = dirName
			)

			diamPoints = go.Scatter(
				x = time,
				y = diameter,
				yaxis = 'y2',
				mode = 'lines',
				name = dirName
			)

			attrLinePlot = go.Scatter(
				x = [time[0], time[-1]],
				y = [attrLine[1] + attrLine[0] * time[0], attrLine[1] + attrLine[0] * time[-1]],
				mode = 'lines',
				name = 'trend - ' + dirName,
				text = str(attrLine[1]) + " + " + str(attrLine[0]) + " * x"
			)

			labPoints = go.Scatter(
				x = time,
				y = labVal,
				mode = 'markers',
				name = dirName,
				text = desc
			)

			labLinePlot = go.Scatter(
				x = [time[0], time[-1]],
				y = [labLine[1] + labLine[0] * time[0], labLine[1] + labLine[0] * time[-1]],
				mode = 'lines',
				name = 'trend - ' + dirName,
				text = str(labLine[1]) + " + " + str(labLine[0]) + " * x"
			)

			attrPlots.append(attrPoints)
			attrTrendPlots.append(attrLinePlot)
			varPlots.append(varPoints)
			diamPlots.append(diamPoints)
			LabPlots.append(labPoints)
			LabTrendPlots.append(labLinePlot)
			i = i + 1

	# put all of these into a nice report, or at least try to
	attrLayout = go.Layout(
		title = "Attribute Value over Time"
	)

	labLayout = go.Layout(
		title = "Lab Value over Time"
	)

	diamLayout = go.Layout(
		title = "Result Space Diameter"
	)

	varLayout = go.Layout(
		title = "Result Variance"
	)

	reportFile = args[4]
	f = open(reportFile, 'w')

	f.write("<html>\n\t<head>\n\t\t<title>Comparison Report - Search Mode: " + args[3] + "</title>\n\t</head>\n\t<body>\n")

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=attrPlots+attrTrendPlots, layout=attrLayout), output_type='div'))
	f.write('</div>')

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=LabPlots+LabTrendPlots, layout=labLayout), output_type='div'))
	f.write('</div>')

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=diamPlots, layout=diamLayout), output_type='div'))
	f.write('</div>')

	f.write('<div style="display: block; height: 700px;">')
	f.write(py.plot(dict(data=varPlots, layout=varLayout), output_type='div'))
	f.write('</div>')

	f.write("\n\t</body>\n</html>")

if __name__ == "__main__":
	main(sys.argv)