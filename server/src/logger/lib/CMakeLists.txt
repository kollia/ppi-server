

set(loggerLibSrcs	LogInterface.cpp	)
        		
add_library(ppilogger SHARED ${loggerLibSrcs})
install(TARGETS ppilogger DESTINATION lib)        		
  
        		
