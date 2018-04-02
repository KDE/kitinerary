set(KDE_INSTALL_INCLUDEDIR_PIM ${KDE_INSTALL_INCLUDEDIR}/KPim)

add_subdirectory(airportdb)

if(TARGET Poppler::Qt5)
    set(HAVE_POPPLER ON)
endif()
configure_file(config-kitinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

set(kitinerary_lib_srcs
    airportdb/airportdb.cpp

    datatypes/bustrip.cpp
    datatypes/flight.cpp
    datatypes/person.cpp
    datatypes/place.cpp
    datatypes/reservation.cpp
    datatypes/ticket.cpp
    datatypes/traintrip.cpp

    calendarhandler.cpp
    extractor.cpp
    extractorengine.cpp
    extractorfilter.cpp
    extractorpreprocessor.cpp
    extractorpostprocessor.cpp
    extractorrepository.cpp
    iatabcbpparser.cpp
    jsonlddocument.cpp
    structureddataextractor.cpp
)
qt5_add_resources(kitinerary_lib_srcs extractors/extractors.qrc)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER logging.h IDENTIFIER KItinerary::Log CATEGORY_NAME org.kde.kitinerary)

add_library(KPimItinerary SHARED ${kitinerary_lib_srcs})
add_library(KPim::Itinerary ALIAS KPimItinerary)
generate_export_header(KPimItinerary BASE_NAME KItinerary)
set_target_properties(KPimItinerary PROPERTIES
    VERSION ${KITINERARY_VERSION_STRING}
    SOVERSION ${KITINERARY_SOVERSION}
    EXPORT_NAME Itinerary
)
target_include_directories(KPimItinerary INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_PIM}>")
target_include_directories(KPimItinerary PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
target_link_libraries(KPimItinerary
    PUBLIC
        Qt5::Core
        KF5::Mime
        KF5::CalendarCore
    PRIVATE
        Qt5::Qml
        KF5::I18n
        KPim::PkPass
)
if (HAVE_POPPLER)
    target_link_libraries(KPimItinerary PRIVATE Poppler::Qt5)
endif()

ecm_generate_headers(KItinerary_FORWARDING_HEADERS
    HEADER_NAMES
        CalendarHandler
        Extractor
        ExtractorEngine
        ExtractorPreprocessor
        ExtractorPostprocessor
        ExtractorRepository
        JsonLdDocument
        StructuredDataExtractor
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_HEADERS
)
ecm_generate_headers(KItinerary_AirportDb_FORWARDING_HEADERS
    HEADER_NAMES
        AirportDb
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_AirportDb_HEADERS
    RELATIVE airportdb
)
ecm_generate_headers(KItinerary_Datatypes_FORWARDING_HEADERS
    HEADER_NAMES
        BusTrip
        Datatypes
        Flight
        Reservation
        Person
        Place
        Ticket
        TrainTrip
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Datatypes_HEADERS
    RELATIVE datatypes
)

install(TARGETS KPimItinerary EXPORT KPimItineraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KItinerary_FORWARDING_HEADERS}
    ${KItinerary_AirportDb_FORWARDING_HEADERS}
    ${KItinerary_Datatypes_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/KItinerary
)
install(FILES
    ${KItinerary_HEADERS}
    ${KItinerary_AirportDb_HEADERS}
    ${KItinerary_Datatypes_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary
)
