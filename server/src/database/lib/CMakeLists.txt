        		
set(databaseLibSrcs	DatabaseFactory.cpp
					Database.cpp	)
   
add_library(ppidb ${libraries} ${databaseLibSrcs})
target_link_libraries(ppidb ppistreamutil ppithreadutil ppipattern)
set_target_properties(ppidb PROPERTIES VERSION ${PPIDB_LIB_VERSION} SOVERSION ${PPIDB_LIB_SOVERSION})
install(TARGETS ppidb DESTINATION lib)


set(DbInterfaceLibSrcs	DbInterface.cpp
                        NeedDbChanges.cpp	)
   
add_library(ppidbinterface ${libraries} ${DbInterfaceLibSrcs})
target_link_libraries(ppidbinterface ppithreadutil ppistreamutil ppiserverlibclient ppilogger ppipattern)
set_target_properties(ppidbinterface PROPERTIES VERSION ${PPIDBINTERFACE_LIB_VERSION} SOVERSION ${PPIDBINTERFACE_LIB_SOVERSION})
install(TARGETS ppidbinterface DESTINATION lib)        		
