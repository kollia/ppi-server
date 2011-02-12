
set(utilSrcs	Subroutine.cpp
				Switch.cpp	
				Value.cpp
				Set.cpp
				Timer.cpp
				Shell.cpp
				Debug.cpp
				Lirc.cpp
				Folder.cpp	)

add_library(ppiportssubroutine ${libraries} ${utilSrcs})
set_target_properties(ppiportssubroutine PROPERTIES VERSION ${PPISUBROUTINES_LIB_VERSION} SOVERSION ${PPISUBROUTINES_LIB_SOVERSION})
install(TARGETS ppiportssubroutine DESTINATION lib)
