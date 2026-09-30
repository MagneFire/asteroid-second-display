set(SRC
    main.cpp
)
set(HEADERS
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_interface(SRC
    ../dbus/org.asteroid.SecondDisplay.Display.xml
    display_interface
)

qt_add_dbus_interface(SRC
    ../dbus/org.asteroid.SecondDisplay.Hands.xml
    hands_interface
)

include_directories(${INCLUDES})

add_executable(asteroid-second-display ${SRC} ${HEADERS})

target_link_libraries(asteroid-second-display
    Qt6::DBus)
