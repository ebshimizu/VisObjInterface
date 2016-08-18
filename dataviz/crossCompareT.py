import sys
import csv
import colorsys
import os
import numpy
import plotly.offline as py
import plotly.graph_objs as go
import crossDirCompare

editTypes = [0, 0.1, 5, 6, 7]
targetNames = { 1 : "target1", 2 : "target2", 3 : "target3" }
folderIds = [ 1, 2, 3 ]

for i in editTypes:
	for folderId in folderIds:
		crossDirCompare.main(["", "../analysis/temperature/", str(folderId), str(i), "../analysis/temperature/" + targetNames[folderId] + "-" + str(i) + ".html"])