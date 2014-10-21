PROJECT( struck_standalone )

ADD_EXECUTABLE( struck_standalone
  main.cpp
)

SET(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${EIGEN_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

TARGET_LINK_LIBRARIES( struck_standalone
    struck
    ${OpenCV_LIBS}
)
