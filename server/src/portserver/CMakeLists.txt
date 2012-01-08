
include_directories(${CMAKE_SOURCE_DIR}/libs/interface)
 
if(USE_LircClient)
	set(extern_libraries ${extern_libraries} ppilircaccess)
endif(USE_LircClient)
if(USE_K8055)
	set(extern_libraries ${extern_libraries} ppivk8055access)
endif(USE_K8055)
if(USE_OWFS)
	set(extern_libraries ${extern_libraries} ppiowfsaccess)
endif(USE_OWFS)  
  
set(portserverSrcs  owserver.cpp
                    OwServerQuestions.cpp
                    KernelModule.cpp
                    ShellWriter.cpp
                    owreadermain.cpp        )
   		
add_executable(ppi-owreader ${portserverSrcs})
target_link_libraries(ppi-owreader 	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
									ppidbinterface ppilogger ppiserverlibclient
									ppiportsaccess ${extern_libraries}					 				)
install(TARGETS ppi-owreader DESTINATION bin)