

set(loggerLibSrcs	LogInterface.cpp
					ProcessLogInterface.cpp
					LogConnectionChecker.cpp	)
        
add_library(ppilogger ${libraries} ${loggerLibSrcs})
target_link_libraries(ppilogger ppithreadutil ppistreamutil ppiserverlibclient)
set_target_properties(ppilogger PROPERTIES VERSION ${PPILOGGER_LIB_VERSION} SOVERSION ${PPILOGGER_LIB_SOVERSION})
install(TARGETS ppilogger DESTINATION lib)        		
  
        		
