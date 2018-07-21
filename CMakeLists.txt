cmake_minimum_required(VERSION 3.0)
set(PIM_VERSION "5.8.90")
project(KItinerary VERSION ${PIM_VERSION})

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

ecm_setup_version(PROJECT VARIABLE_PREFIX KITINERARY
    VERSION_HEADER kitinerary_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPimItineraryConfigVersion.cmake"
)

find_package(Qt5 REQUIRED COMPONENTS Test Gui Qml)
find_package(KF5 REQUIRED COMPONENTS I18n)
find_package(KF5Mime CONFIG REQUIRED)
find_package(KF5CalendarCore CONFIG)
find_package(KF5Contacts CONFIG)
if (NOT ANDROID)
    set_package_properties(KF5CalendarCore PROPERTIES TYPE REQUIRED)
    set_package_properties(KF5Contacts PROPERTIES TYPE REQUIRED)
endif()
find_package(KPimPkPass CONFIG REQUIRED)
find_package(Poppler COMPONENTS Core)
set_package_properties("Poppler" PROPERTIES TYPE OPTIONAL PURPOSE "Support for extraction from PDF booking confirmations.")
find_package(zxing CONFIG)
set_package_properties("zxing" PROPERTIES TYPE OPTIONAL PURPOSE "Support for barcode decoding." URL "https://github.com/glassechidna/zxing-cpp")
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Support for decoding UIC 918-3 train tickets.")

if(TARGET Poppler::Core)
    set(HAVE_POPPLER ON)
    if (${Poppler_VERSION} VERSION_GREATER 0.57)
        set(HAVE_POPPLER_0_58 ON)
    endif()
endif()
if (TARGET zxing::libzxing)
    set(HAVE_ZXING ON)
endif()
if (TARGET KF5::CalendarCore)
    set(HAVE_KCAL ON)
endif()
if (TARGET KF5::Contacts)
    set(HAVE_KCONTACTS ON)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kitinerary\")
add_subdirectory(src)
if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPimItinerary")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimItineraryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPimItineraryConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPimItineraryConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPimItineraryConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KPimItineraryTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KPimItineraryTargets.cmake
        NAMESPACE KPim::
)
install(FILES org_kde_kitinerary.categories DESTINATION ${KDE_INSTALL_CONFDIR})
