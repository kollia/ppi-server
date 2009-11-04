 

set(OWInterfaceLibSrcs	OWInterface.cpp	)
        		
add_library(ppiow SHARED ${OWInterfaceLibSrcs})
set_target_properties(ppiow PROPERTIES VERSION ${PPIOW_LIB_VERSION} SOVERSION ${PPIOW_LIB_SOVERSION})
install(TARGETS ppiow DESTINATION lib)        		
  
        		
