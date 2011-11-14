
set(owfsSrcs	maximchipaccess.cpp	)

add_library(ppiowfsaccess ${libraries} ${owfsSrcs})
target_link_libraries(ppiowfsaccess ppiutil ppipropertyutil ppistreamutil ppipattern ${OWFS_LIBRARIES} ${USB_LIBRARIES})
set_target_properties(ppiowfsaccess PROPERTIES VERSION ${PPI_LIBOWFSACCESS_VERSION} SOVERSION ${PPI_LIBOWFSACCESS_SOVERSION})
install(TARGETS ppiowfsaccess DESTINATION lib)