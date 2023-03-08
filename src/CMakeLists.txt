set(SRC
	main.cpp
    backend.cpp
    backend_catfish.cpp
)
set(HEADERS
    backend.h
    backend_catfish.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.aos.display.xml
    backend.h
    AsteroidOS::SecondDisplayDaemon::Backend
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.aos.hands.xml
    backend.h
    AsteroidOS::SecondDisplayDaemon::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-seconddisplayd ${SRC} ${HEADERS})

target_link_libraries(asteroid-seconddisplayd PRIVATE
	Qt5::DBus Mlite5::Mlite5 ${LIBHYBRIS_LIBRARIES})

install(TARGETS asteroid-seconddisplayd
	DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ../systemd/asteroid-seconddisplayd.service
    DESTINATION /lib/systemd/system)