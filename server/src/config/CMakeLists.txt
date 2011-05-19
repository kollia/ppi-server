
set(configSrcs	configmain.cpp
				LircSupport.cpp		)
        	
add_executable(ppi-mconfig ${configSrcs})
target_link_libraries(ppi-mconfig ppiportssubroutine ppiutil ppipropertyutil ppidb)
install(TARGETS ppi-mconfig DESTINATION bin)


#              The  library  dependency  graph is normally acyclic (a DAG), but in the case of mutually-dependent STATIC libraries CMake allows the graph to contain
#              cycles (strongly connected components).  When another target links to one of the libraries CMake repeats the entire connected component.   For  exam‐
#              ple, the code
#
#                add_library(A STATIC a.c)
#                add_library(B STATIC b.c)
#                target_link_libraries(A B)
#                target_link_libraries(B A)
#                add_executable(main main.c)
#                target_link_libraries(main A)
#
#              links  'main' to 'A B A B'.  (While one repetition is usually sufficient, pathological object file and symbol arrangements can require more.  One may
#              handle such cases by manually repeating the component in the last target_link_libraries call.  However, if two archives are really so  interdependent
#              they should probably be combined into a single archive.)
