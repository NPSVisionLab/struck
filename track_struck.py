'''
Easy!  Demo for struck tracker
'''
import Ice
import easy
import cvac
import sys

class MyDetectorCallbackReceiverI(easy.DetectorCallbackReceiverI):
    def __init__(self):
        import easy
        #super(MyDetectorCallbackReceiverI, self).__init__()
        easy.DetectorCallbackReceiverI.__init__(self)

    def foundNewResults(self, r2, current=None):
        import easy
        easy.DetectorCallbackReceiverI.foundNewResults(self, r2, current)
        #easy.printResults(r2.results)
        #sys.stdout.flush()
        #import pydevd
        #pydevd.connected = True
        #pydevd.settrace(suspend=False)
        easy.drawResults(r2.results)
        
        
print(easy.CVAC_DataDir)
callbackRecv = MyDetectorCallbackReceiverI();


# obtain a reference to Struck Tracker detector
detector = easy.getDetector( "StruckTracker" )


# a test video; the location is relative to the "CVAC.DataDir"
vfile = "tracks/toySoldier.mpg"
runset = cvac.RunSet()
vlab = easy.getLabelable( vfile, labelText="soldier" )
labloc = cvac.BBox(255, 195,70,120)
lab = cvac.LabeledLocation()
lab.loc = labloc
lab.confidence = 0
lab.sub = vlab.sub
easy.addToRunSet(runset, lab, easy.getPurpose('pos'))
# a model file is not required for this detector
modelfile = None
props = easy.getDetectorProperties(detector)
# turn on server display of tracking
props.nativeWindowSize.width = 640;
props.nativeWindowSize.height = 480;
# quietMode false is currently only supported for windows
#if sys.platform == "win32":
#   props.props["quietMode"] = "false"
# turn on server display of debugging info
#props.props["debugMode"] = "true"
props.props["callbackFrequency"] = "immediate"
# apply the detector type, using the model and testing the imgfile
easy.detect( detector, modelfile, runset, callbackRecv = callbackRecv, detectorProperties = props)
results = callbackRecv.allResults;
# you can print the results with Easy!'s pretty-printer;
# we will explain the meaning of the "unlabeled" and the number
# of the found label later.
# or you can always print variables directly (uncomment the next line):
# print("{0}".format( results ))
print("------- Struck Tracker results: -------")
# easy.drawResults( results )
