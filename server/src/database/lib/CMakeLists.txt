 
set(DbInterfaceLibSrcs	DbInterface.cpp
                        NeedDbChanges.cpp	)
   
add_library(ppidb ${libraries} ${DbInterfaceLibSrcs})
target_link_libraries(ppidb ppithreadutil ppistreamutil ppiserverlibclient ppilogger)
set_target_properties(ppidb PROPERTIES VERSION ${PPIDB_LIB_VERSION} SOVERSION ${PPIDB_LIB_SOVERSION})
install(TARGETS ppidb DESTINATION lib)        		
  
        		
