'''
Easy!  Demo for struck tracker
'''

import easy
import cvac

# obtain a reference to Struck Tracker detector
detector = easy.getDetector( "StruckTracker" )


# a test video; the location is relative to the "CVAC.DataDir"
vfile = "toysoldier.mpg"
runset = cvac.RunSet()
vlab = easy.getLabelable( vfile, labelText="soldier" )
labloc = cvac.BBox(100, 120,40,100)
lab = cvac.LabeledLocation()
lab.loc = labloc
lab.confidence = 0
lab.sub = vlab.sub
easy.addToRunSet(runset, lab, easy.getPurpose('pos'))
# a model file is not required for this detector
modelfile = None
props = easy.getDetectorProperties(detector)
# turn on server display of tracking
props.props["quietMode"] = "false"
# turn on server display of debugging info
#props.props["debugMode"] = "true"
# apply the detector type, using the model and testing the imgfile
results = easy.detect( detector, modelfile, runset, detectorProperties = props)

# you can print the results with Easy!'s pretty-printer;
# we will explain the meaning of the "unlabeled" and the number
# of the found label later.
# or you can always print variables directly (uncomment the next line):
# print("{0}".format( results ))
print("------- Struck Tracker results: -------")
easy.printResults( results )
