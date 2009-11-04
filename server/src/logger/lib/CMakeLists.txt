

set(loggerLibSrcs	LogInterface.cpp	)
        		
add_library(ppilogger SHARED ${loggerLibSrcs})
set_target_properties(ppilogger PROPERTIES VERSION ${PPILOGGER_LIB_VERSION} SOVERSION ${PPILOGGER_LIB_SOVERSION})
install(TARGETS ppilogger DESTINATION lib)        		
  
        		
