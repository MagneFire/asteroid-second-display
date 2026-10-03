cmake_minimum_required(VERSION 3.16)

project(asteroid-seconddisplay
	VERSION 1.0.0
	DESCRIPTION "Second Display support for AsteroidOS")

include(FeatureSummary)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core DBus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MLITE IMPORTED_TARGET mlite6)
pkg_check_modules(HYBRIS IMPORTED_TARGET hybris-egl-platform libandroid-properties)
pkg_check_modules(GBINDER IMPORTED_TARGET libgbinder glib-2.0)
add_feature_info(libhybris HYBRIS_FOUND "Backend for catfish and rubyfish")
add_feature_info(libgbinder GBINDER_FOUND "Backend for hoki")
add_feature_info(mlite6 MLITE_FOUND "Keeps settings in dconf; without it they are lost on restart")

add_subdirectory(assets)
add_subdirectory(dbus)
add_subdirectory(src)
add_subdirectory(systemd)
add_subdirectory(tools)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
