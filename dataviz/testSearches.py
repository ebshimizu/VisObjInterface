import sys
from subprocess import call
import glob

scene = sys.argv[1]
targetsDir = sys.argv[2]

targetImages = glob.glob(targetsDir + "/*.png")

exe = "../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe"

for imgPath in targetImages:
	call(exe + " --preload " + scene + " --auto 0 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")
	# call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
	call(exe + " --preload " + scene + " --auto 4 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")
	call(exe + " --preload " + scene + " --auto 5 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")

for i in range(0, 10):
	call(exe + " --preload " + scene + " --auto 0 --img-attr auto --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")
	# call(exe + " --preload " + scene + " --auto 1 --img-attr " + imgPath + " --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1")
	call(exe + " --preload " + scene + " --auto 4 --img-attr auto --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")
	call(exe + " --preload " + scene + " --auto 5 --img-attr auto --more --samples 200 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1 --timeout 15")
