cmake_minimum_required(VERSION 3.5)
set(PIM_VERSION "5.16.40")
project(KItinerary VERSION ${PIM_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(KF5_MIN_VERSION "5.75.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
if (MSVC)
    add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)
endif()
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

set(QT_REQUIRED_VERSION "5.13.0")


find_package(Qt5 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS Gui Qml)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS I18n)
if (NOT ANDROID)
    find_package(SharedMimeInfo 1.3 REQUIRED)
endif()

set(KMIME_VERSION "5.15.80")
set(PIM_PKPASS "5.15.80")

find_package(KF5Mime ${KMIME_VERSION} CONFIG REQUIRED)
find_package(KF5CalendarCore ${KF5_MIN_VERSION} CONFIG)
find_package(KF5Contacts ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KPimPkPass ${PIM_PKPASS} CONFIG REQUIRED)
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
find_package(OpenSSL 1.1)
set_package_properties("OpenSSL" PROPERTIES TYPE OPTIONAL PURPOSE "VDV ticket decoding." URL "https://openssl.org")
find_package(OsmTools)
set_package_properties(OsmTools PROPERTIES TYPE OPTIONAL PURPOSE "Needed only for regenereating the airport database (ie. you most likely don't need this)")

if (NOT ANDROID)
    set_package_properties(KF5CalendarCore PROPERTIES TYPE REQUIRED)
    set_package_properties(LibXml2 PROPERTIES TYPE REQUIRED)
else()
    # force Poppler on Android, to avoid silent failures on binary factory when its
    # Poppler build broke, which then results in feature regressions in KDE Itinerary
    set_package_properties(Poppler PROPERTIES TYPE REQUIRED)
endif()

if(TARGET Poppler::Core)
    # check if we have private Poppler headers
    find_file(HAVE_POPPLER_UNSTABLE_HEADERS "OutputDev.h" PATHS ${Poppler_INCLUDE_DIRS} NO_DEFAULT_PATH)
    if (NOT HAVE_POPPLER_UNSTABLE_HEADERS)
        message(WARNING "Poppler was not build with ENABLE_UNSTABLE_API_ABI_HEADER - building without PDF support!")
        set(HAVE_POPPLER FALSE)
    else()
        set(HAVE_POPPLER TRUE)
    endif()
endif()
if (HAVE_POPPLER)
    string(REGEX MATCH "([0-9]+)\.0*([0-9]+)\.0*([0-9]+)" _match ${Poppler_VERSION})
    set(POPPLER_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(POPPLER_VERSION_MINOR ${CMAKE_MATCH_2})
    set(POPPLER_VERSION_PATCH ${CMAKE_MATCH_3})
endif()

if (TARGET ZXing::ZXing OR TARGET ZXing::Core)
    set(HAVE_ZXING ON)
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
if (TARGET OpenSSL::Crypto)
    set(HAVE_OPENSSL_RSA ON)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kitinerary\")

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054C00)

add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_NO_KEYWORDS)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KPimItinerary")
set(KDE_INSTALL_INCLUDEDIR_PIM ${KDE_INSTALL_INCLUDEDIR}/KPim)

add_subdirectory(src)
if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

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
install(FILES org_kde_kitinerary.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM} COMPONENT Devel)
ki18n_install(po)
