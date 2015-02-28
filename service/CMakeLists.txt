PROJECT( StruckTracker )

WRAP_SLICE(SLICE_FILES
           ${CVAC_ROOT_DIR}/src/Data.ice
           ${CVAC_ROOT_DIR}/src/Services.ice
           )

SET(SOURCE_FILES
    StruckTracker.cpp
)

SET(HEADER_FILES
    StruckTracker.h
)

SET(INCLUDE_DIRS
    .
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${EIGEN_INCLUDE_DIR}
    ${CVAC_INCLUDE_DIR}
    ${ICE_INCLUDE}
    ${CMAKE_CURRENT_BINARY_DIR}#for the generated slice files
)

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})

ADD_LIBRARY(StruckTracker SHARED
            ${SOURCE_FILES}
            ${HEADER_FILES}
            ${SLICE_FILES}
            )

TARGET_LINK_LIBRARIES(StruckTracker
    struck
    utils
    ${ICE_LIBRARIES} ${ICE_BOX_LIBRARY}
    ${OpenCV_LIBS}
    )

ADD_DEPENDENCIES(StruckTracker ${SLICE_FILES})
SET_PROPERTY(SOURCE StruckTracker.cpp APPEND PROPERTY OBJECT_DEPENDS ${SLICE_FILES})
          
IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
    IF(WIN32)
        INSTALL( TARGETS StruckTracker
               DESTINATION bin
               COMPONENT Runtime
       )
    ELSE (WIN32)
        INSTALL( TARGETS StruckTracker
               DESTINATION lib
               COMPONENT Runtime
       )
    ENDIF (WIN32)
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
