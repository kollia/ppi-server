

set(patternSrcs 	IListObjectPattern.cpp
					LogHolderPattern.cpp			)
        		
add_library(ppipattern STATIC ${patternSrcs})
set_target_properties(ppipattern PROPERTIES VERSION ${PPIPATTERN_LIB_VERSION} SOVERSION ${PPIPATTERN_LIB_SOVERSION})
install(TARGETS ppipattern DESTINATION lib)