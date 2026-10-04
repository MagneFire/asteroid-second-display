if(GBINDER_FOUND)
	install(DIRECTORY hoki
		DESTINATION ${CMAKE_INSTALL_DATADIR}/asteroid-seconddisplay
		FILES_MATCHING PATTERN "*.png")
endif()
