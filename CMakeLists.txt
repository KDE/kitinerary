# SPDX-FileCopyrightText: 2018-2025 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2018-2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "11")
set (RELEASE_SERVICE_VERSION_MICRO "70")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

set(PIM_VERSION "6.5.40")
project(KItinerary VERSION ${PIM_VERSION})

set(QT_REQUIRED_VERSION "6.7.0")
set(KF_MIN_VERSION "6.9.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 20)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)
include(ECMAddQch)


option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")


ecm_setup_version(PROJECT VARIABLE_PREFIX KITINERARY
    VERSION_HEADER kitinerary_version.h
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KPim6ItineraryConfigVersion.cmake"
    SOVERSION 6
)

find_package(Qt6 ${QT_REQUIRED_VERSION} REQUIRED COMPONENTS Gui Qml)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Contacts CalendarCore I18n)
if (NOT ANDROID)
    find_package(SharedMimeInfo 1.3 REQUIRED)
endif()

set(KMIME_VERSION "6.4.80")
set(PIM_PKPASS "6.4.80")

find_package(KPim6Mime ${KMIME_VERSION} CONFIG REQUIRED)
find_package(KPim6PkPass ${PIM_PKPASS} CONFIG REQUIRED)
find_package(Poppler COMPONENTS Core)
set_package_properties("Poppler" PROPERTIES TYPE REQUIRED PURPOSE "Support for extraction from PDF booking confirmations.")
find_package(ZXing 2.0 CONFIG)
if (NOT TARGET ZXing::ZXing)
  find_package(ZXing 1.1.1 CONFIG)
endif()
set_package_properties("ZXing" PROPERTIES TYPE REQUIRED PURPOSE "Support for barcode decoding." URL "https://github.com/nu-book/zxing-cpp")
find_package(ZLIB REQUIRED)
set_package_properties("ZLIB" PROPERTIES PURPOSE "Support for decoding UIC 918-3 train tickets.")
find_package(LibXml2 MODULE)
set_package_properties("LibXml2" PROPERTIES PURPOSE "Support for extraction from HTML booking confirmations." URL "http://libxml.org")
find_package(PhoneNumber OPTIONAL_COMPONENTS PhoneNumber QUIET)
set_package_properties("PhoneNumber" PROPERTIES PURPOSE "Parsing and geo-coding of phone numbers.")
find_package(OpenSSL 1.1 REQUIRED COMPONENTS Crypto)
find_package(OsmTools)
set_package_properties(OsmTools PROPERTIES TYPE OPTIONAL PURPOSE "Needed only for regenereating the airport database (ie. you most likely don't need this)")

if (NOT ANDROID)
    set_package_properties(KF6CalendarCore PROPERTIES TYPE REQUIRED)
    set_package_properties(LibXml2 PROPERTIES TYPE REQUIRED)
endif()

if(TARGET Poppler::Core)
    # check if we have private Poppler headers
    find_file(HAVE_POPPLER_UNSTABLE_HEADERS "OutputDev.h" PATHS ${Poppler_INCLUDE_DIRS} NO_DEFAULT_PATH)
    if (NOT HAVE_POPPLER_UNSTABLE_HEADERS)
        message(FATAL_ERROR "Poppler was not build with ENABLE_UNSTABLE_API_ABI_HEADERS!")
    endif()
endif()
string(REGEX MATCH "([0-9]+)\.0*([0-9]+)\.0*([0-9]+)" _match ${Poppler_VERSION})
set(POPPLER_VERSION_MAJOR ${CMAKE_MATCH_1})
set(POPPLER_VERSION_MINOR ${CMAKE_MATCH_2})
set(POPPLER_VERSION_PATCH ${CMAKE_MATCH_3})

if (LIBXML2_FOUND)
    set(HAVE_LIBXML2 ON)
endif()
if (TARGET PhoneNumber::PhoneNumber)
    set(HAVE_PHONENUMBER ON)
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"kitinerary6\")

add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
ecm_set_disabled_deprecation_versions(QT 6.9.0 KF 6.10.0)



set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KPim6Itinerary")
set(KITINERARY_KF6_COMPAT FALSE)
set(KDE_INSTALL_INCLUDEDIR_PIM ${KDE_INSTALL_INCLUDEDIR}/KPim6)

option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
endif()

add_subdirectory(src)
if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KPim6Itinerary_QCH
        FILE KPim6ItineraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KPim6ItineraryQchTargets.cmake\")")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KPimItineraryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KPim6ItineraryConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6ItineraryConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/KPim6ItineraryConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT KPim6ItineraryTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KPim6ItineraryTargets.cmake
        NAMESPACE KPim6::
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM} COMPONENT Devel)

ki18n_install(po)

# for static extractor build packaging
file(WRITE ${CMAKE_BINARY_DIR}/version.txt ${RELEASE_SERVICE_VERSION})
