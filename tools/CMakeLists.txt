set(SRC
    main.cpp
)
set(HEADERS
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_interface(SRC
    ../dbus/org.aos.display.xml
    display_interface
)

qt_add_dbus_interface(SRC
    ../dbus/org.aos.hands.xml
    hands_interface
)

include_directories(${INCLUDES})

add_executable(asteroid-second-display ${SRC} ${HEADERS})

target_link_libraries(asteroid-second-display
    Qt5::DBus
    Qt5::Qml
    Qt5::Quick)

install(TARGETS asteroid-second-display
    DESTINATION ${CMAKE_INSTALL_BINDIR})
