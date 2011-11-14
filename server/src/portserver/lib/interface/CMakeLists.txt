 

set(OWInterfaceLibSrcs	OWInterface.cpp	)
   
add_library(ppiow ${libraries} ${OWInterfaceLibSrcs})
target_link_libraries(ppiow ppistreamutil ppiserverlibclient ppipattern)
set_target_properties(ppiow PROPERTIES VERSION ${PPIOW_LIB_VERSION} SOVERSION ${PPIOW_LIB_SOVERSION})
install(TARGETS ppiow DESTINATION lib)        		
  
        		
