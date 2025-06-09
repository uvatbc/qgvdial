cmake_minimum_required(VERSION 3.16)
project(GoogleVoiceLogin LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic MOC processing
set(CMAKE_AUTOMOC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Quick WebEngineQuick StateMachine)

# Add the executable
qt_add_executable(GoogleVoiceLogin
    main.cpp
    mainengine.cpp
)

qt_add_qml_module(GoogleVoiceLogin
    URI gv
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES main.qml
)

# Link Qt6 libraries
target_link_libraries(GoogleVoiceLogin PRIVATE Qt6::Quick Qt6::WebEngineQuick Qt6::StateMachine)
