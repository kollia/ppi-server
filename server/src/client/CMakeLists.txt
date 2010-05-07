
set(mainSrcs 	main.cpp 
		        Client.cpp				)


add_executable(ppi-client ${mainSrcs} )

target_link_libraries(ppi-client ppicppclient)
#target_link_libraries(ppi-client ppiutil ppiserverlib ppiserverlibclient ppiclientlib ppilogger ${RT_LIBRARIES})
install(TARGETS ppi-client DESTINATION bin)

