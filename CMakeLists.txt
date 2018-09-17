cmake_minimum_required(VERSION 3.0)
set(PIM_VERSION "5.9.47")
project(KItinerary VERSION ${PIM_VERSION})

set(CMAKE_CXX_STANDARD 14)
set(KF5_VERSION "5.50.0")

find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)
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

set(QT_REQUIRED_VERSION "5.9.0")


find_package(Qt5 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS Test Gui Qml)
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS I18n)

set(KMIME_VERSION "5.9.40")
set(KCALENDARCORE_LIB_VERSION "5.9.41")
set(KCONTACTS_LIB_VERSION "5.9.40")


find_package(KF5Mime ${KMIME_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPimPkPass CONFIG REQUIRED)
find_package(Poppler COMPONENTS Core)
set_package_properties("Poppler" PROPERTIES TYPE OPTIONAL PURPOSE "Support for extraction from PDF booking confirmations.")
find_package(zxing CONFIG)
set_package_properties("zxing" PROPERTIES TYPE OPTIONAL PURPOSE "Support for barcode decoding." URL "https://github.com/glassechidna/zxing-cpp")
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Support for decoding UIC 918-3 train tickets.")
find_package(LibXml2 MODULE)
set_package_properties("LibXml2" PROPERTIES PURPOSE "Support for extraction from HTML booking confirmations." URL "http://libxml.org")

if (NOT ANDROID)
    set_package_properties(KF5CalendarCore PROPERTIES TYPE REQUIRED)
    set_package_properties(LibXml2 PROPERTIES TYPE REQUIRED)
endif()

if(TARGET Poppler::Core)
    set(HAVE_POPPLER ON)
    if (${Poppler_VERSION} VERSION_GREATER 0.57)
        set(HAVE_POPPLER_0_58 ON)
    endif()
    if (${Poppler_VERSION} VERSION_GREATER 0.68)
        set(HAVE_POPPLER_0_69 ON)
    endif()
endif()
if (TARGET zxing::libzxing)
    set(HAVE_ZXING ON)
endif()
if (TARGET KF5::CalendarCore)
    set(HAVE_KCAL ON)
endif()
if (LIBXML2_FOUND)
    set(HAVE_LIBXML2 ON)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kitinerary\")
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)

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
