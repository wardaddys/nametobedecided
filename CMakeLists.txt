cmake_minimum_required(VERSION 3.20)
project(OSTuner VERSION 0.5.0 LANGUAGES CXX)

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
    src/core/LogPlayer.cpp
    src/core/ProjectManager.cpp
    src/core/MsqParser.cpp
    src/core/UpdateChecker.cpp
    src/core/WorkspaceRegistry.cpp
    src/widgets/CircularGauge.cpp
    src/widgets/StatusBox.cpp
    src/widgets/DashboardWidget.cpp
    src/widgets/ThrottleBodyWidget.cpp
    src/widgets/AllTablesWidget.cpp
    src/widgets/TableEditor.cpp
    src/widgets/IdleControlWidget.cpp
    src/widgets/AccelEnrichmentWidget.cpp
    src/widgets/TrimsWidget.cpp
    src/widgets/ColdStartWidget.cpp
    src/widgets/MapSensorWidget.cpp
    src/widgets/TempSensorWidget.cpp
    src/widgets/InjectorVisualWidget.cpp

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
    src/widgets/ProjectWizardOverlay.cpp
    src/widgets/ReqFuelCalculatorDialog.cpp
    src/widgets/onboarding/ProductTourOverlay.cpp
    src/core/ReferenceDatabase.cpp
    # Group D — Workspaces (real implementations, opt-in via Settings flag).
    src/widgets/workspaces/WorkspaceContainer.cpp
    src/widgets/workspaces/FuelingWorkspace.cpp
    src/widgets/workspaces/IgnitionWorkspace.cpp
    # Group E — firmware-parity widgets.
    src/widgets/DecoderConfigWidget.cpp
    src/widgets/EngineCommandsWidget.cpp
    src/widgets/CurveEditorWidget.cpp
    src/widgets/IdleAlgorithmWidget.cpp
    src/widgets/VVT2ControlWidget.cpp
    src/widgets/EngineProtectionWidget.cpp
    src/widgets/SDLoggerWidget.cpp
    src/widgets/AdvancedFuelingWidget.cpp
    src/widgets/CommsConfigWidget.cpp
    src/widgets/CompositeLoggerWidget.cpp
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
    src/core/LogPlayer.h
    src/core/ProjectManager.h
    src/core/MsqParser.h
    src/core/UpdateChecker.h
    src/core/WorkspaceRegistry.h
    src/widgets/CircularGauge.h
    src/widgets/StatusBox.h
    src/widgets/DashboardWidget.h
    src/widgets/ThrottleBodyWidget.h
    src/widgets/AllTablesWidget.h
    src/widgets/TableEditor.h
    src/widgets/IdleControlWidget.h
    src/widgets/AccelEnrichmentWidget.h
    src/widgets/TrimsWidget.h
    src/widgets/ColdStartWidget.h
    src/widgets/MapSensorWidget.h
    src/widgets/TempSensorWidget.h
    src/widgets/InjectorVisualWidget.h

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
    src/widgets/ProjectWizardOverlay.h
    src/widgets/ReqFuelCalculatorDialog.h
    src/widgets/onboarding/ProductTourOverlay.h
    src/widgets/onboarding/TourStep.h
    src/core/ReferenceDatabase.h
    # Group D — Workspaces.
    src/widgets/workspaces/WorkspaceBase.h
    src/widgets/workspaces/WorkspaceContainer.h
    src/widgets/workspaces/FuelingWorkspace.h
    src/widgets/workspaces/IgnitionWorkspace.h
    # Group E — firmware-parity widgets.
    src/widgets/DecoderConfigWidget.h
    src/widgets/EngineCommandsWidget.h
    src/widgets/CurveEditorWidget.h
    src/widgets/IdleAlgorithmWidget.h
    src/widgets/VVT2ControlWidget.h
    src/widgets/EngineProtectionWidget.h
    src/widgets/SDLoggerWidget.h
    src/widgets/AdvancedFuelingWidget.h
    src/widgets/CommsConfigWidget.h
    src/widgets/CompositeLoggerWidget.h
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
set(CPACK_PACKAGE_NAME "OSTuner")
set(CPACK_PACKAGE_VENDOR "OS Tuner Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-Source ECU Tuning Application for Speeduino")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OSTuner")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

# NSIS Specifics
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_DISPLAY_NAME "OS Tuner")
set(CPACK_NSIS_PACKAGE_NAME "OS Tuner")
set(CPACK_NSIS_HELP_LINK "https://github.com/AeroSaeed/OS-Tuner")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/AeroSaeed/OS-Tuner")
set(CPACK_NSIS_CHASSIS_DESCRIPTION "An open-source ECU tuning application for Speeduino.")

# Desktop shortcut and Start Menu
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\OS Tuner.lnk' '$INSTDIR\\OSTuner.exe'")
set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$DESKTOP\\OS Tuner.lnk'")

include(CPack)

# Tests
include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    # Core sources shared across every test executable (G1-G4 each get their
    # own binary so a crash in one suite doesn't block the others).
    set(OSTUNER_TEST_CORE_SOURCES
        src/core/ECUSettingsManager.cpp
        src/core/ECUDefinition.cpp
        src/core/ECUData.cpp
        src/core/SerialManager.cpp
        src/core/SpeeduinoProtocol.cpp
        src/core/WorkspaceRegistry.cpp
        src/core/MsqParser.cpp
        src/utils/Logger.cpp
    )

    set(OSTUNER_TEST_INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/widgets
        ${CMAKE_SOURCE_DIR}/src/dialogs
        ${CMAKE_SOURCE_DIR}/src/utils
    )

    set(OSTUNER_TEST_LIBS
        Qt6::Core
        Qt6::Test
        Qt6::SerialPort
        Qt6::Xml
        Qt6::Network
    )

    function(add_ostuner_test target_name source_file)
        add_executable(${target_name} ${source_file} ${OSTUNER_TEST_CORE_SOURCES})
        target_include_directories(${target_name} PRIVATE ${OSTUNER_TEST_INCLUDE_DIRS})
        target_link_libraries(${target_name} ${OSTUNER_TEST_LIBS})
        # SOURCE_DIR_FOR_TESTS lets the INI smoke test locate the bundled
        # MS1 Extra fixture relative to the source tree.
        target_compile_definitions(${target_name} PRIVATE
            "SOURCE_DIR_FOR_TESTS=\"${CMAKE_SOURCE_DIR}\""
        )
        add_test(NAME ${target_name} COMMAND ${target_name})
    endfunction()

    # Phase 0 guardrails (existing test suite).
    add_ostuner_test(OSTunerCoreTests       tests/ECUSettingsManagerGuardrailsTest.cpp)
    # G1: protocol round-trip — CRC32, frame wrap/unwrap, command builders.
    add_ostuner_test(SpeeduinoProtocolTest  tests/SpeeduinoProtocolTest.cpp)
    # G2: INI parser coverage on real and synthetic fixtures.
    add_ostuner_test(ECUDefinitionTest      tests/ECUDefinitionTest.cpp)
    # G3: SerialManager simulation + signature-gate contract.
    add_ostuner_test(SerialHandshakeTest    tests/SerialHandshakeTest.cpp)
    # G4: burnAllDirty regression lock (BUG-A).
    add_ostuner_test(BurnChainTest          tests/BurnChainTest.cpp)
    # F4: MSQ round-trip + metadata regression coverage.
    add_ostuner_test(MsqRoundTripTest       tests/MsqRoundTripTest.cpp)
endif()
