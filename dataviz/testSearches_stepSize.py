import sys
from subprocess import call
import glob

# tests to compare effect of edit step size on search results
# Temperature = 0.5, initial edit depth = 3, JND = 0.1

scene = sys.argv[1]
targetsDir = sys.argv[2]

stepSizes = [0.1, 0.05, 0.01, 0.2, 0.5]
editModes = [0, 5, 6, 7]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

for imgPath in targetImages:
	for stepSize in stepSizes:
		logDir = "../analysis/stepSize/" + str(tvalue) + "/"

		# special cases
		# uniform edit weight
		cmd = exe + "--step-size " + str(stepSize) + " --preload " + scene + " --auto 0 --uniform-edits --img-attr " + imgPath + " --more --samples 700 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature 0.5 --session-name 0.1"
		print cmd
		call(cmd)


		for editMode in editModes:
			cmd = exe + "--step-size " + str(stepSize) + " --preload " + scene + " --auto " + str(editMode) + " --img-attr " + imgPath + " --more --samples 700 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature 0.5"
			print cmd
			call(cmd)

		call("python processData.py " + logDir)


	# lmgd just for fun
	cmd = exe + "--step-size " + str(stepSize) + " --preload " + scene + " --auto 1 --uniform-edits --img-attr " + imgPath + " --more --samples 700 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature 0.5"


#for i in range(0, 194):
#	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
#	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 6 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")