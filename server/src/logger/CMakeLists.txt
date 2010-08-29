       		
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(loggerSrcs	LogThread.cpp
        		LogProcess.cpp
        		loggermain.cpp	)
		
add_executable(ppi-log-client ${loggerSrcs})
target_link_libraries(ppi-log-client ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil ppiserverlibclient ppilogger)
install(TARGETS ppi-log-client DESTINATION bin)

