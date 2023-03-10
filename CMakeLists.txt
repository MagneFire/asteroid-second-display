cmake_minimum_required(VERSION 3.6.0)

project(asteroid-seconddisplay
	VERSION 1.0.0
	DESCRIPTION "Second Display support for AsteroidOS")

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(GNUInstallDirs)
include(ECMFindQmlModule)
include(ECMGeneratePkgConfigFile)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(BUILD_SHARED_LIBS TRUE)

set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)

set(INSTALL_QML_IMPORT_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/qml")

find_package(Qt5 5 COMPONENTS DBus Qml Quick REQUIRED)

add_subdirectory(declarative)
add_subdirectory(src)
add_subdirectory(tools)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
