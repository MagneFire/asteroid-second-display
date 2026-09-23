add_executable(asteroid-seconddisplay main.cpp)
target_include_directories(asteroid-seconddisplay PRIVATE ${PROJECT_SOURCE_DIR}/dbus)
target_link_libraries(asteroid-seconddisplay PRIVATE Qt6::DBus)

install(TARGETS asteroid-seconddisplay
	DESTINATION ${CMAKE_INSTALL_BINDIR})
