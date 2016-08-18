import sys
from subprocess import call
import glob

scene = sys.argv[1]
targetsDir = sys.argv[2]

tvalues = [0.1, 10, 5, 1, 0.5, 0.001]
editModes = [0, 5, 6, 7]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

for imgPath in targetImages:
	for tvalue in tvalues:
		logDir = "../analysis/temperature/T" + str(tvalue) + "/"

		# special cases
		# uniform edit weight
		cmd = exe + " --preload " + scene + " --auto 0 --uniform-edits --img-attr " + imgPath + " --more --samples 1000 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature " + str(tvalue) + " --session-name 0.1"
		print cmd
		call(cmd)


		for editMode in editModes:
			cmd = exe + " --preload " + scene + " --auto " + str(editMode) + " --img-attr " + imgPath + " --more --samples 1000 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature " + str(tvalue)
			print cmd
			call(cmd)

		call("python processData.py " + logDir)

	# lmgd just for fun
	cmd = exe + " --preload " + scene + " --auto 1 --uniform-edits --img-attr " + imgPath + " --more --samples 1000 --out " + logDir + " --jnd 0.1 --timeout 10 --temperature " + str(tvalue)

call("python crossCompareT.py")

#for i in range(0, 194):
#	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
#	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 6 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")