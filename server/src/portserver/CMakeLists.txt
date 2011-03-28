
include_directories(${CMAKE_SOURCE_DIR}/lib)

set(portserverSrcs  owserver.cpp
                    OwServerQuestions.cpp
                    KernelModule.cpp
            	    maximchipaccess.cpp
            		VellemannK8055.cpp
            		ExternPorts.cpp
            		irsend.cpp
            		LircClient.cpp
                    owreadermain.cpp        )
        		
add_executable(ppi-owreader ${portserverSrcs})
target_link_libraries(ppi-owreader 	ppiutil ppistreamutil ppipropertyutil ppithreadutil ppiprocessutil 
									ppidbinterface ppilogger ppiserverlibclient
                                    lirc_client ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} )
install(TARGETS ppi-owreader DESTINATION bin)