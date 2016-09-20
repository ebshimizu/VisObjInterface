import sys
from subprocess import call
import glob
import compare
import plotLib
import os

scene = sys.argv[1]
targetsDir = sys.argv[2]

editModes = [6, 8]
resampleTime = [5, 15, 30, 60, 100]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

for editMode in editModes:
	compareDirs = dict()
	compareNames = dict()

	for rt in resampleTime:
		compareDirs[rt] = []
		compareNames[rt] = []

	logDir = "../analysis/recenter/" + str(editMode) + "/"
	for imgPath in targetImages:
		for rt in resampleTime:
			cmd = exe + " --preload " + scene + " --auto " + str(editMode) + " --img-attr " + imgPath + " --more --samples 1000 --out " + logDir + " --jnd 0.1 --timeout 10 --session-name RT" + str(rt) + " --resample-time " + str(rt)
			print cmd
			call(cmd)

			compareDirs[rt].append(logDir + "RT" + str(rt) + "/" + os.listdir(logDir + "RT" + str(rt))[-2])
			compareNames[rt].append("RT" + str(rt))

	call("python processData.py " + logDir)

	# compare things we just tested
	for i in range(0, len(targetImages)):
		compareArgs = ['compare.py']
		for rt in resampleTime:
			compareArgs.append(compareDirs[rt][i])
			compareArgs.append(compareNames[rt][i])
		compareArgs.append(logDir + targetImages[i].split('\\')[-1] + ".html")
		compare.main(compareArgs)

# call("python crossCompareT.py")

#for i in range(0, 194):
#	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
#	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 6 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 1