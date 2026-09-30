set(SRC
	main.cpp
    backend.cpp
)
set(HEADERS
    backend.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.SecondDisplay.Display.xml
    backend.h
    SecondDisplay::Backend
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.SecondDisplay.Hands.xml
    backend.h
    SecondDisplay::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-seconddisplayd ${SRC} ${HEADERS})

target_link_libraries(asteroid-seconddisplayd PRIVATE
	Qt6::DBus)

install(TARGETS asteroid-seconddisplayd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
