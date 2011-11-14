
set(portsaccessSrcs	ExternPorts.cpp	)

add_library(ppiportsaccess ${libraries} ${portsaccessSrcs})
target_link_libraries(ppiportsaccess ppiutil ppipropertyutil ppistreamutil ppipattern)
set_target_properties(ppiportsaccess PROPERTIES VERSION ${PPI_LIBPORTSACCESS_VERSION} SOVERSION ${PPI_LIBPORTSACCESS_SOVERSION})
install(TARGETS ppiportsaccess DESTINATION lib)