cmake_minimum_required(VERSION 3.6.0)

project(asteroid-seconddisplay
	VERSION 1.0.0
	DESCRIPTION "Second Display support for AsteroidOS")

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)

include(FeatureSummary)
include(GNUInstallDirs)
include(ECMFindQmlModule)
include(ECMGeneratePkgConfigFile)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BUILD_SHARED_LIBS TRUE)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

set(INSTALL_QML_IMPORT_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/qml")

find_package(Mlite5 MODULE REQUIRED)
find_package(Qt5 5 COMPONENTS DBus Qml Quick REQUIRED)
pkg_check_modules(LIBHYBRIS REQUIRED hybris-egl-platform libandroid-properties)

add_subdirectory(dbus)
add_subdirectory(declarative)
add_subdirectory(src)
add_subdirectory(systemd)
add_subdirectory(tools)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
