 

set(OWInterfaceLibSrcs	OWInterface.cpp	)
        		
add_library(ppiow SHARED ${OWInterfaceLibSrcs})
install(TARGETS ppiow DESTINATION lib)        		
  
        		
