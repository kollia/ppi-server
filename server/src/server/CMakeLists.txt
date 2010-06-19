


set(serverSrcs	internetservermain.cpp
        		ServerTransaction.cpp	        )
        		
add_executable(ppi-internet-server ${serverSrcs})
target_link_libraries(ppi-internet-server ppiutil ppiserverlib ppiserverlibserver ppiserverlibclient ppidb ppiow ppilogger ${RT_LIBRARIES} 
                              ppiports ${OWFS_LIBRARIES} ${USB_LIBRARIES} ${K8055_LIBRARIES} ) # for debug-info from owserver
install(TARGETS ppi-internet-server DESTINATION bin)
