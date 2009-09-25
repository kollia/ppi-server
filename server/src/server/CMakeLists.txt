

set(serverSrcs	ClientTransaction.cpp
        		hearingthread.cpp
        		ServerTransaction.cpp	        )
        		
add_library(ppiserver SHARED ${serverSrcs})
install(TARGETS ppiserver DESTINATION lib)



#set(communicateserverSrcs   communicatemain.cpp        )
                            
#add_executable(ppi-communicate-server ${communicateserverSrcs})
#target_link_libraries(ppi-communicate-server ppiutil ppiserverlib ppiserverlibclient ppiserverlibserver ppilogger ${RT_LIBRARIES})
#install(TARGETS ppi-communicate-server DESTINATION bin)
