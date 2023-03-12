qt5_generate_dbus_interface(display.h
    org.aos.display.xml
    OPTIONS -A
)

qt5_generate_dbus_interface(hands.h
    org.aos.hands.xml
    OPTIONS -A
)

install(FILES ../dbus/org.asteroid.SecondDisplay.conf
    DESTINATION /etc/dbus-1/system.d)
