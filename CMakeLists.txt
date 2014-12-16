ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( service )
# Uncomment the standalone to build the standalone tracker executable
#ADD_SUBDIRECTORY( standalone )

#Since the demo is in a different git project copy it to the demo
#directory
FILE( COPY ${CMAKE_CURRENT_SOURCE_DIR}/track_struck.py
      DESTINATION ${CMAKE_SOURCE_DIR}/demo
    )

IF( NOT ${BUILD_3RDPARTY_PACKAGE} )
  INSTALL( FILES track_struck.py
           DESTINATION demo
           COMPONENT Runtime
         )
ENDIF( NOT ${BUILD_3RDPARTY_PACKAGE} )
