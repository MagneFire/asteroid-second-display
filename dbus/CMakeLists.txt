configure_file(org.asteroid.SecondDisplay.service.in org.asteroid.SecondDisplay.service @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.asteroid.SecondDisplay.service
	DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
