import sys
from subprocess import call
import glob
import compare
import plotLib
import os

scene = sys.argv[1]
targetsDir = sys.argv[2]

searchTypes = [0,1]
editTypes = [0,1]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

compareDirs = dict()

for editType in editTypes:
	for searchType in searchTypes:
		compareDirs[str(searchType) +"_" + str(editType)] = []

logDir = "../analysis/editModes/"
for imgPath in targetImages:
	for editType in editTypes:
		for searchType in searchTypes:
			sessionName = str(searchType) +"_" + str(editType)
			cmd = ""
			if searchType == 0:
				cmd = exe + " --preload " + scene + " --auto " + str(searchType) + " --edit-mode " + str(editType) + " --img-attr " + imgPath + " --more --samples 1000 --out " + logDir + " --jnd 3 --timeout 3 --session-name " + sessionName
			elif searchType == 1:
				cmd = exe + " --preload " + scene + " --auto " + str(searchType) + " --edit-mode " + str(editType) + " --img-attr " + imgPath + " --more --samples 1000 --step-size 0.25 --chain-length 30 --out " + logDir + " --jnd 3 --timeout 3 --session-name " + sessionName
			print cmd
			call(cmd)

			compareDirs[sessionName].append(logDir + sessionName + "/" + os.listdir(logDir + sessionName)[-2])

		call("python processData.py " + logDir)

# compare things we just tested
for i in range(0, len(targetImages)):
	compareArgs = ['compare.py']
	for editType in editTypes:
		for searchType in searchTypes:
			sessionName = str(searchType) +"_" + str(editType)
			compareArgs.append(compareDirs[sessionName][i])
			compareArgs.append(plotLib.searchModeNames[searchType] + ": " + plotLib.editModeNames[editType])
	compareArgs.append(logDir + targetImages[i].split('\\')[-1] + ".html")
	compare.main(compareArgs)

# call("python crossCompareT.py")

#for i in range(0, 194):
#	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
#	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
#	call(exe + " --preload " + scene + " --auto 6 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")