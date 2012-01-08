
if(USE_K8055)
	set(vk8055Srcs	VellemannK8055.cpp	)
	
	add_library(ppivk8055access ${libraries} ${vk8055Srcs})
	target_link_libraries(ppivk8055access ppiutil ppipropertyutil ppistreamutil ppipattern ${K8055_LIBRARIES} ${USB_LIBRARIES})
	set_target_properties(ppivk8055access PROPERTIES VERSION ${PPI_LIBVK8055ACCESS_VERSION} SOVERSION ${PPI_LIBVK8055ACCESS_SOVERSION})
	install(TARGETS ppivk8055access DESTINATION lib)
endif(USE_K8055)