

set(clientLibSrcs   ClientTransaction.cpp 
                    hearingthread.cpp       )
add_library(ppiclientlib STATIC ${clientLibSrcs})



set(serverSrcs	internetservermain.cpp
        		ServerTransaction.cpp	        )
        		
add_executable(ppi-internet-server ${serverSrcs})
target_link_libraries(ppi-internet-server ppiutil ppiserverlib ppiserverlibserver ppiserverlibclient ppiclientlib ppidb ppiow ppilogger ${RT_LIBRARIES} 
                              ppiports ppiportserver ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} ) # for debug-info from owserver
install(TARGETS ppi-internet-server DESTINATION bin)



#set(communicateserverSrcs   communicatemain.cpp        )
                            
#add_executable(ppi-communicate-server ${communicateserverSrcs})
#target_link_libraries(ppi-communicate-server ppiutil ppiserverlib ppiserverlibclient ppiserverlibserver ppilogger ${RT_LIBRARIES})
#install(TARGETS ppi-communicate-server DESTINATION bin)
