

add_subdirectory( ports )
add_subdirectory( vellemann )
if(USE_OWFS)
	add_subdirectory( maxim )
endif(USE_OWFS)
if(USE_LircClient)
	add_subdirectory( lirc )
endif(USE_LircClient)