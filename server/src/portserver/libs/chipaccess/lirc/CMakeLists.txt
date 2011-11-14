
set(lircSrcs	irsend.cpp
				LircClient.cpp	)
				
add_library(ppilircaccess ${libraries} ${lircSrcs})
target_link_libraries(ppilircaccess ppiutil ppipropertyutil ppistreamutil ppipattern lirc_client)
set_target_properties(ppilircaccess PROPERTIES VERSION ${PPI_LIBLIRCACCESS_VERSION} SOVERSION ${PPI_LIBLIRCACCESS_SOVERSION})
install(TARGETS ppilircaccess DESTINATION lib)