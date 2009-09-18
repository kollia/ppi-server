 

set(DbInterfaceLibSrcs	DbInterface.cpp
                        NeedDbChanges.cpp	)
        		
add_library(ppidb SHARED ${DbInterfaceLibSrcs})
install(TARGETS ppidb DESTINATION lib)        		
  
        		
