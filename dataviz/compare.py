import sys
import csv
import colorsys
import os
import numpy
import plotly.offline as py
import plotly.graph_objs as go

# The command line args for this are totally weird but here's how it works.
# Everything hinges off of the 3rd argument and whether or not its an integer
# If args[3] is an int the format is: [output file name] [input directory] [folder number]
# If args[3] is not an int the format is: [output file name] [list of directories...]
def main(args):
	searchModeNames = {0: 'MCMC with Edits', 4 : 'Minimizing MCMC with Edits', 5 : 'MCMC with LMGD Refinement', 6 : 'Recentring MCMC', 7 : 'Recentering MCMC with LMGD'}
	searchModes = [0, 4, 5, 6, 7]

	arbitrary = False
	try:
		x = int(args[3])
	except ValueError:
		arbitrary = True

	dirList = []
	if arbitrary == True:
		dirList = args[2:-1]
	else:
		prefix = args[2]
		dirNum = int(args[3])
		for i in searchModes:
			dirs = os.listdir(prefix + str(i))
			dirList.append(prefix + str(i) + "/" + dirs[dirNum])

	attrPlots = []
	LabPlots = []
	attrTrendPlots = []
	LabTrendPlots = []
	diamPlots = []
	varPlots = []

	print "comparing directories:\n" + "\n".join(dirList)

	i = 0
	for path in dirList:
		dirName = path.rpartition('/')[2]
		filename = path + "/results.csv"

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

		if arbitrary == False:
			dirName = searchModeNames[searchModes[i]]

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

	reportFile = args[1]
	f = open(reportFile, 'w')

	f.write("<html>\n\t<head>\n\t\t<title>Comparison Report</title>\n\t</head>\n\t<body>\n")

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