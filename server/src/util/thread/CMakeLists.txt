
set(utilSrcs	StatusLogRoutine.cpp
				Thread.cpp
				Terminal.cpp			)

add_library(ppithreadutil ${libraries} ${utilSrcs})
target_link_libraries(ppithreadutil ${RT_LIBRARIES} ppiutil ppipattern)
set_target_properties(ppithreadutil PROPERTIES VERSION ${PPITHREADUTIL_LIB_VERSION} SOVERSION ${PPITHREADUTIL_LIB_SOVERSION})
install(TARGETS ppithreadutil DESTINATION lib)
