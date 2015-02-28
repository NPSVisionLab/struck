'''
Easy!  Demo for struck tracker.  We run the demo twice.  The first time is 
with just displaying the results at the end of the run.  This is the
simplest way to run the demo.  In the next run we create a callback handler
that takes the output from the tracker and displays the results on the images.
'''
import Ice
import easy
import cvac
import sys
import thread

# obtain a reference to Struck Tracker detector
detector = easy.getDetector( "StruckTracker" )

# a test video; the location is relative to the "CVAC.DataDir"
vfile = "tracks/toySoldier.mpg"
runset = cvac.RunSet()

# The tracker needs a labeled location that defines the location
# within the first frame of the video  of the item we want to track.
# In this we define a bounding box around the toy soldiers initial location.
vlab = easy.getLabelable( vfile, labelText="soldier" )
labloc = cvac.BBox(255, 195,70,120)
lab = cvac.LabeledLocation()
lab.loc = labloc
lab.confidence = 0
lab.sub = vlab.sub
easy.addToRunSet(runset, lab, easy.getPurpose('pos'))
# Setting the verbosity level gives us some feedback while the tracking is done.
props = easy.getDetectorProperties(detector)
props.verbosity = 7
# A model file is not required for this detector
modelfile = None
results = easy.detect( detector, modelfile, runset, detectorProperties = props)

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
'''
Now we run it again this time setting up a callback to display the
tracking frames on the images.
'''

class MyDetectorCallbackReceiverI(easy.DetectorCallbackReceiverI):
    def __init__(self):
        import easy
        easy.DetectorCallbackReceiverI.__init__(self)

    def foundNewResults(self, r2, current=None):
        easy.DetectorCallbackReceiverI.foundNewResults(self, r2, current)
        # Draw the result video frame and the bounding box
        easy.drawResults(r2.results)
        
        
# For OSX the detect must be done in a different thead and the thread
# start function wants a function to call so this is it
def doDetect():
    easy.detect( detector, modelfile, runset, callbackRecv = callbackRecv, detectorProperties = props)

callbackRecv = MyDetectorCallbackReceiverI();
props = easy.getDetectorProperties(detector)
# turn on server display of tracking
props.nativeWindowSize.width = 640;
props.nativeWindowSize.height = 480;
# Tell the tracker to send us results as soon as they are available
props.props["callbackFrequency"] = "immediate"
# apply the detector type, using the model and testing the imgfile
if sys.platform == 'darwin':
    thread.start_new_thread(doDetect, ())
    easy.guiqueue.startThread()
else:
    easy.detect( detector, modelfile, runset, callbackRecv = callbackRecv, detectorProperties = props)
    results = callbackRecv.allResults;
