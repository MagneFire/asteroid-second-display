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

add_subdirectory(src)
add_subdirectory(tools)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
