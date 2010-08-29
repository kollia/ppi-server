
set(utilSrcs	properties.cpp
        		interlacedproperties.cpp
                actionproperties.cpp
        		interlacedactionproperties.cpp
        		configpropertycasher.cpp			)

add_library(ppipropertyutil ${libraries} ${utilSrcs})
target_link_libraries(ppipropertyutil ppistreamutil ppipattern)
set_target_properties(ppipropertyutil PROPERTIES VERSION ${PPIPROPERTYUTIL_LIB_VERSION} SOVERSION ${PPIPROPERTYUTIL_LIB_SOVERSION})
install(TARGETS ppipropertyutil DESTINATION lib)
