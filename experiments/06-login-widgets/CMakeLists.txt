cmake_minimum_required(VERSION 3.16)
project(GoogleVoiceLogin LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to Qt: Set QTDIR=/path/to/qt-bin-version
set(Qt6Core_DIR "$ENV{QTDIR}")

# Enable Qt's automatic MOC processing
set(CMAKE_AUTOMOC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets StateMachine)

# Add the executable
add_executable(GoogleVoiceLogin main.cpp mainwindow.cpp)

# Link Qt6 libraries
target_link_libraries(GoogleVoiceLogin PRIVATE Qt6::Widgets Qt6::WebEngineWidgets Qt6::StateMachine)
