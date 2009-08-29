


set(databaseSrcs    Database.cpp 
		            DefaultChipConfigReader.cpp	)
        		
add_library(ppidatabase SHARED ${databaseSrcs})

install(TARGETS ppidatabase DESTINATION lib)