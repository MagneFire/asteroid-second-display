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

add_library(asteroidseconddisplaysplugin MODULE ${SRC} ${HEADERS})

target_link_libraries(asteroidseconddisplaysplugin
	Qt6::DBus
	Qt6::Qml
	Qt6::Quick)

install(TARGETS asteroidseconddisplaysplugin
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/seconddisplay)
install(FILES qmldir
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/seconddisplay)
