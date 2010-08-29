
set(mainSrcs 	main.cpp 
		        Client.cpp				)


add_executable(ppi-client ${mainSrcs} )

target_link_libraries(ppi-client ppiutil ppipropertyutil ppicppclient ppiserverlibclient ppiserverlibserver)
install(TARGETS ppi-client DESTINATION bin)

