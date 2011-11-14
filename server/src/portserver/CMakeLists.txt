
include_directories(${CMAKE_SOURCE_DIR}/libs/interface)

set(portserverSrcs  owserver.cpp
                    OwServerQuestions.cpp
                    KernelModule.cpp
                    ShellWriter.cpp
                    owreadermain.cpp        )
        		
add_executable(ppi-owreader ${portserverSrcs})
target_link_libraries(ppi-owreader 	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
									ppidbinterface ppilogger ppiserverlibclient
									ppilircaccess ppiowfsaccess ppiportsaccess ppivk8055access				)
install(TARGETS ppi-owreader DESTINATION bin)