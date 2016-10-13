import sys
from subprocess import call
import glob
import compare
import plotLib
import os

scene = sys.argv[1]
targetsDir = sys.argv[2]

editModes = [8]
stepSizes = [0.1, 0.25, 0.5, 1]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

compareDirs = dict()

for editMode in editModes:
	compareDirs[editMode] = []

logDir = "../analysis/stepsize/"
for imgPath in targetImages:
	for stepSize in stepSizes:
		cmd = exe + " --preload " + scene + " --auto 8 --img-attr " + imgPath + " --more --step-size " + str(stepSize) + " --samples 1000 --out " + logDir + " --jnd 2 --timeout 3 --session-name " + str(stepSize)
		print cmd
		call(cmd)

		compareDirs[editMode].append(logDir + str(stepSize) + "/" + os.listdir(logDir + str(stepSize))[-2])

	call("python processData.py " + logDir)

# compare things we just tested
for i in range(0, len(targetImages)):
	compareArgs = ['compare.py']
	for editMode in editModes:
		compareArgs.append(compareDirs[editMode][i])
		compareArgs.append(plotLib.searchModeNames[editMode])
	compareArgs.append(logDir + targetImages[i].split('\\')[-1] + ".html")
	compare.main(compareArgs)

# call("python crossCompareT.py")

#for i in range(0, 194):
#	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
#	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 6 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")