
set(utilSrcs	ppivalues.cpp
                BaseErrorHandling.cpp
                ErrorHandling.cpp
                IMethodStringStream.cpp
        		IParameterStringStream.cpp
        		OMethodStringStream.cpp
        		OParameterStringStream.cpp		)

add_library(ppistreamutil ${libraries} ${utilSrcs})
set_target_properties(ppistreamutil PROPERTIES VERSION ${PPISTREAMUTIL_LIB_VERSION} SOVERSION ${PPISTREAMUTIL_LIB_SOVERSION})
install(TARGETS ppistreamutil DESTINATION lib)
