cmake_minimum_required(VERSION 3.20)
project(TunerPro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages - Required components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort Xml Network)

# Find optional Qt6 components (don't fail if not found)
find_package(Qt6 QUIET COMPONENTS Charts)
find_package(Qt6 QUIET COMPONENTS DataVisualization)

if(TARGET Qt6::Charts)
    add_compile_definitions(HAS_QT_CHARTS)
    message(STATUS "Qt6 Charts found - chart features enabled")
endif()

if(TARGET Qt6::DataVisualization)
    add_compile_definitions(HAS_QT_DATAVIS)
    message(STATUS "Qt6 DataVisualization found - 3D features enabled")
endif()

# UI files directory
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/core/SerialManager.cpp
    src/core/SpeeduinoProtocol.cpp
    src/core/ECUData.cpp
    src/core/ECUDefinition.cpp
    src/core/LoggingManager.cpp
    src/core/ECUSettingsManager.cpp
    src/core/ProjectManager.cpp
    src/core/MsqParser.cpp
    src/core/UpdateChecker.cpp
    src/widgets/CircularGauge.cpp
    src/widgets/StatusBox.cpp
    src/widgets/DashboardWidget.cpp
    src/widgets/AllTablesWidget.cpp
    src/widgets/TableEditor.cpp
    src/widgets/IdleControlWidget.cpp
    src/widgets/AccelEnrichmentWidget.cpp
    src/widgets/TrimsWidget.cpp
    src/widgets/ColdStartWidget.cpp
    src/widgets/SettingsDropdown.cpp
    src/widgets/VVTVTECWidget.cpp
    src/widgets/BoostWidget.cpp
    src/widgets/KnockWidget.cpp
    src/widgets/LimitersWidget.cpp
    src/dialogs/ConnectionDialog.cpp
    src/dialogs/AboutDialog.cpp
    src/utils/Logger.cpp
    src/utils/Settings.cpp
    src/utils/DemoModeManager.cpp
    src/utils/ExpressionParser.cpp
    src/widgets/IOConfigWidget.cpp
    src/widgets/ECUSettingsWidget.cpp
    src/widgets/TunerColorMap.cpp
    src/widgets/HeatmapDelegate.cpp
    src/widgets/ColorLegendBar.cpp
    src/widgets/TunerMap3DWidget.cpp
    src/widgets/TunerGauge.cpp
    src/widgets/TunerProSplashScreen.cpp
    src/widgets/LoggingWidget.cpp
    src/widgets/StartupDialog.cpp
    src/widgets/CommandButton.cpp
    src/widgets/DashboardTheme.cpp
    src/widgets/DataGraph.cpp
    src/widgets/TuningWidgetBase.cpp
    src/widgets/ToothLoggerWidget.cpp
    src/core/ReferenceDatabase.cpp
)

# Header files
set(HEADERS
    src/MainWindow.h
    src/core/SerialManager.h
    src/core/SpeeduinoProtocol.h
    src/core/SpeeduinoConstants.h
    src/core/ECUData.h
    src/core/ECUDefinition.h
    src/core/LoggingManager.h
    src/core/ECUSettingsManager.h
    src/core/ProjectManager.h
    src/core/MsqParser.h
    src/core/UpdateChecker.h
    src/widgets/CircularGauge.h
    src/widgets/StatusBox.h
    src/widgets/DashboardWidget.h
    src/widgets/AllTablesWidget.h
    src/widgets/TableEditor.h
    src/widgets/IdleControlWidget.h
    src/widgets/AccelEnrichmentWidget.h
    src/widgets/TrimsWidget.h
    src/widgets/ColdStartWidget.h
    src/widgets/SettingsDropdown.h
    src/widgets/VVTVTECWidget.h
    src/widgets/BoostWidget.h
    src/widgets/KnockWidget.h
    src/widgets/LimitersWidget.h
    src/dialogs/ConnectionDialog.h
    src/widgets/IOConfigWidget.h
    src/widgets/ECUSettingsWidget.h
    src/widgets/TunerColorMap.h
    src/widgets/HeatmapDelegate.h
    src/widgets/ColorLegendBar.h
    src/widgets/TunerMap3DWidget.h
    src/widgets/TunerGauge.h
    src/widgets/TunerProSplashScreen.h
    src/widgets/LoggingWidget.h
    src/widgets/StartupDialog.h

    src/dialogs/AboutDialog.h
    src/utils/Logger.h
    src/utils/Settings.h
    src/utils/DemoModeManager.h
    src/utils/ExpressionParser.h
    src/widgets/CommandButton.h
    src/widgets/DashboardTheme.h
    src/widgets/DataGraph.h
    src/widgets/TuningWidgetBase.h
    src/widgets/ToothLoggerWidget.h
    src/core/ReferenceDatabase.h
)

# UI files
set(UI_FILES
    ui/MainWindow.ui
    ui/DashboardWidget.ui
    ui/ConnectionDialog.ui
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Xml
    Qt6::Network
)

if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt6::Charts)
endif()

if(Qt6DataVisualization_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt6::DataVisualization)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/widgets
    ${CMAKE_SOURCE_DIR}/src/dialogs
    ${CMAKE_SOURCE_DIR}/src/utils
)

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)

# Install required resources and runtime dependencies (conceptual for CPack)
# Note: For Qt apps on Windows, windeployqt is usually needed. 
# We'll rely on CPack to bundle the install tree.

# CPack Configuration
set(CPACK_PACKAGE_NAME "TunerPro-V2")
set(CPACK_PACKAGE_VENDOR "TunerPro Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Professional ECU Tuning Application for Speeduino")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TunerPro-V2")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# NSIS Specifics
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_DISPLAY_NAME "TunerPro V2")
set(CPACK_NSIS_PACKAGE_NAME "TunerPro V2")
set(CPACK_NSIS_HELP_LINK "https://github.com/tunerpro/tunerpro-v2")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/tunerpro/tunerpro-v2")
set(CPACK_NSIS_CHASSIS_DESCRIPTION "A professional ECU tuning application for Speeduino.")

# Desktop shortcut and Start Menu
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\TunerPro V2.lnk' '$INSTDIR\\TunerPro.exe'")
set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\TunerPro V2.lnk'")

include(CPack)
