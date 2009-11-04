 
set(DbInterfaceLibSrcs	DbInterface.cpp
                        NeedDbChanges.cpp	)
   
     		
add_library(ppidb SHARED ${DbInterfaceLibSrcs})
set_target_properties(ppidb PROPERTIES VERSION ${PPIDB_LIB_VERSION} SOVERSION ${PPIDB_LIB_SOVERSION})
install(TARGETS ppidb DESTINATION lib)        		
  
        		
