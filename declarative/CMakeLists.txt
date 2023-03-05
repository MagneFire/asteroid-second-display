set(SRC
	seconddisplay_plugin.cpp
	seconddisplay.cpp
	hands.cpp
)
set(HEADERS
	seconddisplay_plugin.h
	seconddisplay.h
	hands.h
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

add_library(asteroidseconddisplaysplugin ${SRC} ${HEADERS})

target_link_libraries(asteroidseconddisplaysplugin
	Qt5::DBus
	Qt5::Qml
	Qt5::Quick)

install(TARGETS asteroidseconddisplaysplugin
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/seconddisplay)
install(FILES qmldir
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/seconddisplay)
