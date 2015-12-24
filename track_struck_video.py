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
import cv2

# obtain a reference to Struck Tracker detector
detector = easy.getDetector( "StruckTracker" )

# a test video; the location is relative to the "CVAC.DataDir"
vfile = "tracks/VTS_01_2.mpg"
runset = cvac.RunSet()

# The tracker needs a labeled location that defines the location
# within the first frame of the video  of the item we want to track.
# In this we define a bounding box around the toy soldiers initial location.
vlab = easy.getLabelable( vfile, labelText="car" )
labloc = cvac.BBox(290, 280,60,30)
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

class MyDetectorCallbackReceiverI(easy.DetectorCallbackReceiverI):
    def __init__(self):
        import easy
        easy.DetectorCallbackReceiverI.__init__(self)
        self.video = cv2.VideoWriter("results.mpg", cv2.cv.CV_FOURCC('P','I','M','1'),25, (720, 480))
        if self.video.isOpened() == False:
            print("Could not create video file!")

    def foundNewResults(self, r2, current=None):
        easy.DetectorCallbackReceiverI.foundNewResults(self, r2, current)
        # Draw the result video frame and the bounding box
        #easy.drawResults(r2.results)
        #write a video file with the results
        substrates = easy.collectSubstrates(r2.results)
        for subpath in substrates.iterkeys():
            img = cv2.imread(subpath)
            if img == None:
                print("Could not read frame file " + subpath)
                continue
            for lbl in substrates[subpath]:
                for frame in lbl.keyframesLocations:
                    if isinstance(frame.loc, cvac.BBox):
                        a = frame.loc.x
                        b = frame.loc.y
                        c = a+frame.loc.width
                        d = b+frame.loc.height
                        cv2.rectangle(img,(a,b),(c,d),(0,255,0),2)
            self.video.write(img)


    def writeResults():
        cv2.destroyAllWindows()
        self.video.release()
        
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
    callbackRecv.writeResults;
