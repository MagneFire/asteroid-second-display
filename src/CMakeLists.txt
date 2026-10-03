set(SRC
	main.cpp
    backend.cpp
    display.cpp
    hands.cpp
    propertieschangedrelay.cpp
    settingsstore.cpp
    login1.cpp
    backend_casio.cpp
    backend_narwhal.cpp
)
set(HEADERS
    backend.h
    display.h
    hands.h
    propertieschangedrelay.h
    settingsstore.h
    settingskeys.h
    login1.h
    backend_casio.h
    backend_narwhal.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.SecondDisplay.Display.xml
    display.h
    SecondDisplay::Display
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.SecondDisplay.Hands.xml
    hands.h
    SecondDisplay::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-seconddisplayd ${SRC} ${HEADERS})

target_link_libraries(asteroid-seconddisplayd PRIVATE
	Qt6::DBus)

if(HYBRIS_FOUND)
	target_sources(asteroid-seconddisplayd PRIVATE backend_mobvoi.cpp)
	target_compile_definitions(asteroid-seconddisplayd PRIVATE HAVE_HYBRIS)
	target_link_libraries(asteroid-seconddisplayd PRIVATE PkgConfig::HYBRIS)
endif()

if(GBINDER_FOUND)
	add_library(seconddisplay-sidekick STATIC
		hidl/sidekickclient.cpp
		hidl/sidekickclient.h
		hidl/sidekicktypes.h)
	target_include_directories(seconddisplay-sidekick PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/hidl)
	target_link_libraries(seconddisplay-sidekick PUBLIC Qt6::Core PkgConfig::GBINDER)
endif()

if(MLITE_FOUND)
	target_sources(asteroid-seconddisplayd PRIVATE dconfsettingsstore.cpp)
	target_compile_definitions(asteroid-seconddisplayd PRIVATE HAVE_MLITE)
	target_link_libraries(asteroid-seconddisplayd PRIVATE PkgConfig::MLITE)
endif()

install(TARGETS asteroid-seconddisplayd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
