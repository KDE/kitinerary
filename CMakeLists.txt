cmake_minimum_required(VERSION 3.5)
set(PIM_VERSION "5.11.40")
project(KItinerary VERSION ${PIM_VERSION})

set(CMAKE_CXX_STANDARD 14)
set(KF5_MIN_VERSION "5.56.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
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

set(QT_REQUIRED_VERSION "5.10.0")


find_package(Qt5 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS Test Gui Qml)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n)

set(KMIME_VERSION "5.11.40")
set(KCALENDARCORE_LIB_VERSION "5.11.40")
set(KCONTACTS_LIB_VERSION "5.11.40")


find_package(KF5Mime ${KMIME_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KCALENDARCORE_LIB_VERSION} CONFIG)
find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
find_package(KPimPkPass CONFIG REQUIRED)
find_package(Poppler COMPONENTS Core)
set_package_properties("Poppler" PROPERTIES TYPE OPTIONAL PURPOSE "Support for extraction from PDF booking confirmations.")
find_package(ZXing CONFIG)
set_package_properties("ZXing" PROPERTIES TYPE OPTIONAL PURPOSE "Support for barcode decoding." URL "https://github.com/nu-book/zxing-cpp")
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Support for decoding UIC 918-3 train tickets.")
find_package(LibXml2 MODULE)
set_package_properties("LibXml2" PROPERTIES PURPOSE "Support for extraction from HTML booking confirmations." URL "http://libxml.org")
find_package(PhoneNumber OPTIONAL_COMPONENTS PhoneNumber QUIET)
set_package_properties("PhoneNumber" PROPERTIES PURPOSE "Parsing and geo-coding of phone numbers.")

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
    set(CMAKE_REQUIRED_LIBRARIES Poppler::Core Qt5::Core)
    check_cxx_source_compiles("
    #include <goo/GooString.h>
    #include <QString>
    int main()
    {
        GooString s;
        QString val = QString::fromUtf8(s.c_str());
        return 0;
    }
    " HAVE_POPPLER_0_72)
endif()

if (TARGET ZXing::Core)
    set(HAVE_ZXING ON)
elseif (TARGET zxing::libzxing)
    message(WARNING "Old zxing-cpp variant from https://github.com/glassechidna/zxing-cpp found - support for this will be removed eventually!")
    set(HAVE_ZXING_OLD ON)
endif()

if (TARGET KF5::CalendarCore)
    set(HAVE_KCAL ON)
endif()
if (LIBXML2_FOUND)
    set(HAVE_LIBXML2 ON)
endif()
if (TARGET PhoneNumber::PhoneNumber)
    set(HAVE_PHONENUMBER ON)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kitinerary\")

# workaround for https://bugreports.qt.io/browse/QTBUG-74665 (bug in qt5.13 reevaluate it)
if (${Qt5Core_VERSION} STRGREATER "5.13")
   MESSAGE(STATUS "Qt version: ${Qt5Core_VERSION} DISABLE compile without deprecated methods. bug QTBUG-74665")
else()
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x060000)
endif()

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
