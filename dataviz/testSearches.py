import sys
from subprocess import call
import glob

scene = sys.argv[1]
targetsDir = sys.argv[2]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

for imgPath in targetImages:
	#call(exe + " --preload " + scene + " --auto 0 --img-attr " + imgPath + " --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
	#call(exe + " --preload " + scene + " --auto 4 --img-attr " + imgPath + " --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 5 --img-attr " + imgPath + " --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	call(exe + " --preload " + scene + " --auto 6 --img-attr " + imgPath + " --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")

for i in range(0, 194):
	#call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
	#call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	#call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5 --timeout 10")
	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 400 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 2 --timeout 10")