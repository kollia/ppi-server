

set(loggerLibSrcs	LogInterface.cpp	)
        		
add_library(ppilogger SHARED ${loggerLibSrcs})
install(TARGETS ppilogger DESTINATION lib)        		
  
        		
        		
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(loggerSrcs	LogThread.cpp
        		LogProcess.cpp
        		loggermain.cpp	)
		
add_executable(ppi-log-client ${loggerSrcs})
target_link_libraries(ppi-log-client ppiutil ppilogger ppiports ppidatabase ppiportserver ppiserver 
                        ${RT_LIBRARIES} ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} ${OPENSSL_LIBRARIES} ${LircClient_LIBRARIES})
install(TARGETS ppi-log-client DESTINATION bin)

