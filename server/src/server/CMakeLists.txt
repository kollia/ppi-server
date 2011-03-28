


set(serverSrcs	internetservermain.cpp
        		ServerTransaction.cpp	        )
        		
add_executable(ppi-internet-server ${serverSrcs})
target_link_libraries(ppi-internet-server	ppiutil ppipropertyutil ppithreadutil ppiprocessutil 
											ppiserverlibclient ppiserverlibserver 
											ppilogger ppidbinterface )

install(TARGETS ppi-internet-server DESTINATION bin)
