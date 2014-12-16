PROJECT( struck )

SET(SOURCE_FILES
  Config.cpp
  Features.cpp
  HaarFeature.cpp
  HaarFeatures.cpp
  HistogramFeatures.cpp
  ImageRep.cpp
  LaRank.cpp
  MultiFeatures.cpp
  RawFeatures.cpp
  Sampler.cpp
  Tracker.cpp
  GraphUtils/GraphUtils.cpp
)

SET(HEADER_FILES
  Config.h
  Features.h
  HaarFeature.h
  HaarFeatures.h
  HistogramFeatures.h
  ImageRep.h
  LaRank.h
  MultiFeatures.h
  RawFeatures.h
  Sampler.h
  Tracker.h
  GraphUtils/GraphUtils.h
)

FIND_PATH( EIGEN_INCLUDE_DIR Eigen/Core
  PATHS
  /usr/local/include/eigen2  # struck is not compatible with eigen3
  /opt/local/include/eigen2  # struck is not compatible with eigen3
  REQUIRED
)

IF( NOT EIGEN_INCLUDE_DIR )
  MESSAGE( SEND_ERROR "could not find eigen library" )
ENDIF()

SET(INCLUDE_DIRS
    ${CVAC_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY( struck STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

