
include_directories(${CMAKE_SOURCE_DIR}/libs/interface)

set(portserverSrcs  owserver.cpp
                    OwServerQuestions.cpp
                    KernelModule.cpp
                    ShellWriter.cpp
            		libs/chipaccess/ports/ExternPorts.cpp
            	    libs/chipaccess/maxim/maximchipaccess.cpp
            		libs/chipaccess/vellemann/VellemannK8055.cpp
            		libs/chipaccess/lirc/irsend.cpp
            		libs/chipaccess/lirc/LircClient.cpp
                    owreadermain.cpp        )
        		
add_executable(ppi-owreader ${portserverSrcs})
target_link_libraries(ppi-owreader 	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
									ppidbinterface ppilogger ppiserverlibclient
                                    lirc_client ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} )
install(TARGETS ppi-owreader DESTINATION bin)