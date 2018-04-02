cmake_minimum_required(VERSION 3.0)
project(KItinerary VERSION 0.0.1)

find_package(ECM 5.38 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)

set(CMAKE_AUTOMOC ON)
ecm_setup_version(PROJECT VARIABLE_PREFIX KITINERARY
    VERSION_HEADER kitinerary_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KItineraryConfigVersion.cmake"
)

find_package(Qt5 REQUIRED COMPONENTS Test Gui Qml)
find_package(KF5 REQUIRED COMPONENTS I18n)
find_package(KF5Mime CONFIG REQUIRED)
find_package(KF5CalendarCore CONFIG REQUIRED)
find_package(KPimPkPass CONFIG REQUIRED)
find_package(Poppler COMPONENTS Qt5)
set_package_properties("Poppler" PROPERTIES TYPE OPTIONAL PURPOSE "Support for extraction from PDF booking confirmations.")

add_subdirectory(src)
add_subdirectory(autotests)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KItinerary")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KItineraryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KItineraryConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KItineraryConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KItineraryConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KItineraryTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KItineraryTargets.cmake
)
install(FILES org_kde_kitinerary.categories DESTINATION ${KDE_INSTALL_CONFDIR})
