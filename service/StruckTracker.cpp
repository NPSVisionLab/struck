/******************************************************************************
 * CVAC Software Disclaimer
 * 
 * This software was developed at the Naval Postgraduate School, Monterey, CA,
 * by employees of the Federal Government in the course of their official duties.
 * Pursuant to title 17 Section 105 of the United States Code this software
 * is not subject to copyright protection and is in the public domain. It is 
 * an experimental system.  The Naval Postgraduate School assumes no
 * responsibility whatsoever for its use by other parties, and makes
 * no guarantees, expressed or implied, about its quality, reliability, 
 * or any other characteristic.
 * We would appreciate acknowledgement and a brief notification if the software
 * is used.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above notice,
 *       this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Naval Postgraduate School, nor the name of
 *       the U.S. Government, nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without
 *       specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE NAVAL POSTGRADUATE SCHOOL (NPS) AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NPS OR THE U.S. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include <iostream>
#include <vector>
#include <string>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <util/FileUtils.h>

#include <util/ServiceManI.h>
#include <util/OutputResults.h>

#include <Tracker.h>
#include <Config.h>

#include <stdlib.h>

#include "StruckTracker.h"

using namespace cvac;
using namespace Ice;
using namespace cv;
using namespace std;


///////////////////////////////////////////////////////////////////////////////
// This is called by IceBox to get the service to communicate with.
extern "C"
{
  /**
   * Create the detector service via a ServiceManager.  The 
   * ServiceManager handles all the icebox interactions.  Pass the constructed
   * detector instance to the ServiceManager.  The ServiceManager obtains the
   * service name from the config.icebox file as follows. Given this
   * entry:
   * IceBox.Service.BOW_Detector=bowICEServer:create --Ice.Config=config.service
   * ... the name of the service is BOW_Detector.
   */
  ICE_DECLSPEC_EXPORT IceBox::Service* create(CommunicatorPtr communicator)
  {
    StruckTracker *detector = new StruckTracker();
    ServiceManagerI *sMan = new ServiceManagerI( detector, detector );
    detector->setServiceManager( sMan );
    return sMan;
  }
}

///////////////////////////////////////////////////////////////////////////////

StruckTracker::StruckTracker()
  : callback(NULL)
  , mServiceMan(NULL)
{
}

StruckTracker::~StruckTracker()
{

}

void StruckTracker::setServiceManager(ServiceManagerI *sman)
{
  mServiceMan = sman;
}

void StruckTracker::starting()
{
  m_CVAC_DataDir = mServiceMan->getDataDir();
}  



// Client verbosity
bool StruckTracker::initialize( const DetectorProperties& detprops,
                                 const Current& current)
{
  // Create DetectorPropertiesI class to allow the user to modify detection
  // parameters
  mDetectorProps = new DetectorPropertiesI();

  // Get the default CVAC data directory as defined in the config file
  localAndClientMsg(VLogger::DEBUG, NULL, "Initializing StruckTracker...\n");
  Ice::PropertiesPtr iceprops = (current.adapter->getCommunicator()->getProperties());
  string verbStr = iceprops->getProperty("CVAC.ServicesVerbosity");
  if (!verbStr.empty())
  {
    getVLogger().setLocalVerbosityLevel( verbStr );
  }


  localAndClientMsg(VLogger::INFO, NULL, "StruckTracker initialized.\n");
  
  return true;
}

std::string StruckTracker::getName(const ::Ice::Current& current)
{
  return mServiceMan->getServiceName();
}

std::string StruckTracker::getDescription(const ::Ice::Current& current)
{
  return "Struck Tracker";
}

void StruckTracker::setVerbosity(::Ice::Int verbosity, const ::Ice::Current& current)
{
}

DetectorProperties StruckTracker::getDetectorProperties(const ::Ice::Current& current)
{
  return DetectorPropertiesI();
}

// static function to render the track rectangle onto the mat
static void rectangle(Mat &rMat, const FloatRect& rRect, const Scalar& rColour)
{
    IntRect r(rRect);
    rectangle(rMat, Point(r.XMin(), r.YMin()), Point(r.XMax(), r.YMax()), rColour);
}

/** Scans the detection cascade across each image in the RunSet
 * and returns the results to the client
 */
void StruckTracker::process( const Identity &client,
                              const RunSet& runset,
                              const FilePath& model,
                              const DetectorProperties& detprops,
                              const Current& current)
{
  callback = DetectorCallbackHandlerPrx::uncheckedCast(
            current.con->createProxy(client)->ice_oneway());

  bool initRes = initialize( detprops, current );
  if (initRes == false)
      return;
  mDetectorProps->load(detprops);
  //////////////////////////////////////////////////////////////////////////
  // Setup - RunsetConstraints
  cvac::RunSetConstraint mRunsetConstraint;  
  mRunsetConstraint.addType("jpg");
  mRunsetConstraint.addType("png");
  mRunsetConstraint.addType("tif");

  //////////////////////////////////////////////////////////////////////////
  // Start - RunsetWrapper
  mServiceMan->setStoppable();  
  cvac::RunSetWrapper mRunsetWrapper(&runset,m_CVAC_DataDir,mServiceMan);
  mServiceMan->clearStop();
  if(!mRunsetWrapper.isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, callback,
      "RunsetWrapper is not initialized, aborting.\n");    
    return;
  }
  // End - RunsetWrapper

  OutputResults outputres(callback, mDetectorProps->callbackFreq);

  localAndClientMsg(VLogger::INFO, NULL, "Converting video to images\n"); 
  //////////////////////////////////////////////////////////////////////////
  // Start - RunsetIterator
  int nSkipFrames = 1;  //the number of skip frames
  mServiceMan->setStoppable();
  cvac::RunSetIterator mRunsetIterator(&mRunsetWrapper,mRunsetConstraint,
                                       mServiceMan,callback,nSkipFrames);
  mServiceMan->clearStop();
  if(!mRunsetIterator.isInitialized())
  {
    localAndClientMsg(VLogger::ERROR, callback,
      "RunSetIterator is not initialized, aborting.\n");
    return;
  } 
  // End - RunsetIterator

  FloatRect initBB;
  float scaleW = 1.f;
  float scaleH = 1.f;
  string currentVideo = "";  // The current video we are processing
  Tracker tracker(mDetectorProps->config);
#ifdef WIN32
  // This shows a blank window on OSX so only do Windows
  if (!mDetectorProps->config.quietMode)
  {
      namedWindow("result", CV_WINDOW_KEEPRATIO);
      waitKey(1);
  }
#endif
  srand(mDetectorProps->config.seed);
  Mat frame;
  Mat result(mDetectorProps->config.frameHeight, mDetectorProps->config.frameWidth, CV_8UC3);
  mServiceMan->setStoppable();
  int frameCnt = 0;
  localAndClientMsg(VLogger::INFO, NULL, "Processing images\n"); 
  while(mRunsetIterator.hasNext())
  {
    if((mServiceMan != NULL) && (mServiceMan->stopRequested()))
    {        
      mServiceMan->stopCompleted();
      break;
    }
    
    cvac::Labelable& labelable = *(mRunsetIterator.getNext());
    Result &curres = mRunsetIterator.getCurrentResult();
    string vfullname = getFSPath( RunSetWrapper::getFilePath(curres.original), m_CVAC_DataDir );
    string iname = getFSPath( RunSetWrapper::getFilePath(labelable), m_CVAC_DataDir );
    Mat frameOrig = cv::imread(iname, 0);
    if (frameOrig.empty())
    {
        localAndClientMsg(VLogger::ERROR, callback,
                         "Failed to read in Frame file for video %s.\n", vfullname.c_str());    
        return;
    }
    resize(frameOrig, frame, cv::Size(mDetectorProps->config.frameWidth,
                                  mDetectorProps->config.frameHeight));
    
    if (vfullname != currentVideo)
    {
        frameCnt = 0;
        currentVideo = vfullname;
        scaleW = (float)mDetectorProps->config.frameWidth / frameOrig.cols;
        scaleH = (float)mDetectorProps->config.frameHeight / frameOrig.rows;
        LabeledLocationPtr loc = cvac::LabeledLocationPtr::dynamicCast(curres.original);
        if (loc)
        {
            BBoxPtr box = BBoxPtr::dynamicCast(loc->loc);
            if (box)
            {
                initBB = FloatRect(box->x * scaleW, box->y * scaleH, box->width * scaleW,
                                   box->height * scaleH);
                tracker.Initialise(frame, initBB);
            }else
            {
                 localAndClientMsg(VLogger::ERROR, callback,
                         "Video %s does not have inital frame bounding box.\n", vfullname.c_str());    
                  return;
            }
        }else
        { 
             localAndClientMsg(VLogger::ERROR, callback,
                         "Video %s does not have a labeled location for what to track.\n", vfullname.c_str());    
             return;
        }
    }
    if (tracker.IsInitialised())
    {
        localAndClientMsg(VLogger::DEBUG, NULL, "Process frame %d\n", 
                          frameCnt++);
        tracker.Track(frame);
#ifdef WIN32
        if (!mDetectorProps->config.quietMode && mDetectorProps->config.debugMode)
            tracker.Debug();
       
        if (!mDetectorProps->config.quietMode)
        { // Show tracking results in window
            cvtColor(frame, result, CV_GRAY2RGB);
            rectangle(result, tracker.GetBB(), CV_RGB(0,255,0));
            imshow("result", result);
            // need to process window events
            waitKey(1);
        }
#endif
        std::vector<cv::Rect> rects;
        const FloatRect& bb = tracker.GetBB();
        cv::Rect rec = cv::Rect((int)bb.XMin()/scaleW, (int) bb.YMin()/scaleH,
                                 (int)bb.Width()/scaleW, (int) bb.Height()/scaleH);
        localAndClientMsg(VLogger::DEBUG, NULL, "res rec %d, %d, %d, %d\n",
                          rec.x, rec.y, rec.width, rec.height
                          );
        rects.push_back(rec);
        outputres.addResult(curres,labelable, rects, "positive", 1.0f);
    }else
    {
        localAndClientMsg(VLogger::ERROR, callback,
                         "Tracker failed to initialize.\n");    
        return;
    }
    
  }  
#ifdef WIN32
  if (!mDetectorProps->config.quietMode)
  {
      destroyAllWindows();
      waitKey(1);
  }
#endif
  // We are done so send any final results
  outputres.finishedResults(mRunsetIterator);
  mServiceMan->clearStop();

  //////////////////////////////////////////////////////////////////////////
}

bool StruckTracker::cancel(const Identity &client, const Current& current)
{
  localAndClientMsg(VLogger::WARN, NULL, "cancel not implemented.");
  return false;
}

//----------------------------------------------------------------------------
DetectorPropertiesI::DetectorPropertiesI()
{
    verbosity = 0;
    config.quietMode = true;
    config.debugMode = false;
    config.frameWidth = 320;
    config.frameHeight = 240;
    nativeWindowSize.width = 320;
    nativeWindowSize.height = 240;
    config.seed = 0;
    config.searchRadius = 30;
    config.svmC = 100.0;
    config.svmBudgetSize = 100;
    config.features.clear();
    Config::FeatureKernelPair kp;
    kp.feature = Config::kFeatureTypeHaar;
    kp.kernel = Config::kKernelTypeGaussian;
    std::vector<double> params;
    params.push_back(0.2);
    kp.params = params;
    config.features.push_back(kp);
    
  
}

void DetectorPropertiesI::load(const DetectorProperties &p) 
{
    verbosity = p.verbosity;
    props = p.props;
    //Only load values that are not zero
    if (p.nativeWindowSize.width > 0 && p.nativeWindowSize.height > 0)
        nativeWindowSize = p.nativeWindowSize;
        config.frameWidth = nativeWindowSize.width;
        config.frameHeight = nativeWindowSize.height;

    readProps();
}

bool DetectorPropertiesI::readProps()
{
    bool res = true;
    cvac::Properties::iterator it;
    for (it = props.begin(); it != props.end(); it++)
    {
        if (it->first.compare("callbackFrequency") == 0)
        {
            callbackFreq = it->second;
            if ((it->second.compare("labelable") != 0) &&
                (it->second.compare("immediate") != 0) &&
                (it->second.compare("final") == 0))
            {
                localAndClientMsg(VLogger::ERROR, NULL, 
                         "callbackFrequency type not supported.\n");
                res = false;
            }
        }
        if (it->first.compare("quietMode") == 0)
        {
            if ((it->second.compare("true") == 0) ||
                (it->second.compare("True") == 0))
            {
                config.quietMode = true;
            } else
            {
                config.quietMode = false;
            }
        }
        if (it->first.compare("debugMode") == 0)
        {
            if ((it->second.compare("true") == 0) ||
                (it->second.compare("True") == 0))
            {
                config.debugMode = true;
            } else
            {
                config.debugMode = false;
            }
        }
    }   
    return res;
}
 
bool DetectorPropertiesI::writeProps()
{

    bool res = true;
    props.insert(std::pair<string, string>("callbackFrequency", callbackFreq));
    if (config.quietMode)
        props.insert(std::pair<string, string>("quietMode", "true"));
    else
        props.insert(std::pair<string, string>("quietMode", "false"));
    if (config.debugMode)
        props.insert(std::pair<string, string>("debugMode", "true"));
    else
        props.insert(std::pair<string, string>("debugMode", "false"));
    return res;
}

