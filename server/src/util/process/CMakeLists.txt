
set(utilSrcs	process.cpp
        		ProcessStarter.cpp	)

add_library(ppiprocessutil ${libraries} ${utilSrcs})
target_link_libraries(ppiprocessutil ppistreamutil ppithreadutil ppiserverlibclient)
set_target_properties(ppiprocessutil PROPERTIES VERSION ${PPIPROCESSUTIL_LIB_VERSION} SOVERSION ${PPIPROCESSUTIL_LIB_SOVERSION})
install(TARGETS ppiprocessutil DESTINATION lib)
