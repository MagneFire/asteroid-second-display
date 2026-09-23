set(SYSTEMD_USER_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/user" CACHE PATH "Directory for systemd user units")

configure_file(asteroid-seconddisplayd.service.in asteroid-seconddisplayd.service @ONLY)
configure_file(asteroid-seconddisplay-sync-time.service.in asteroid-seconddisplay-sync-time.service @ONLY)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/asteroid-seconddisplayd.service
	${CMAKE_CURRENT_BINARY_DIR}/asteroid-seconddisplay-sync-time.service
	asteroid-seconddisplay-sync-time.timer
	DESTINATION ${SYSTEMD_USER_UNIT_DIR})
