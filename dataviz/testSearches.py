import sys
from subprocess import call

call("../Builds/VisualStudio2015NoArnold/x64/Release/AttributesInterface.exe --preload C:/Users/eshimizu/OneDrive/Documents/research/attributes_project/scenes/apartment/apartment_cache_al.rig.json --auto 0 --img-attr auto --more --samples 100 --out C:/Users/eshimizu/Documents/AttributesInterface/logs/ --jnd 1.5")
