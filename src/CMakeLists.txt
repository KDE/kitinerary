set(KDE_INSTALL_INCLUDEDIR_PIM ${KDE_INSTALL_INCLUDEDIR}/KPim)

add_subdirectory(cli)
add_subdirectory(knowledgedb-generator)
configure_file(config-kitinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

set(kitinerary_lib_srcs
    datatypes/action.cpp
    datatypes/brand.cpp
    datatypes/bustrip.cpp
    datatypes/event.cpp
    datatypes/flight.cpp
    datatypes/organization.cpp
    datatypes/person.cpp
    datatypes/place.cpp
    datatypes/reservation.cpp
    datatypes/taxi.cpp
    datatypes/ticket.cpp
    datatypes/traintrip.cpp
    datatypes/rentalcar.cpp
    datatypes/visit.cpp

    jsapi/barcode.cpp
    jsapi/bitarray.cpp
    jsapi/context.cpp
    jsapi/jsonld.cpp

    knowledgedb/airportdb.cpp
    knowledgedb/countrydb.cpp
    knowledgedb/iatacode.cpp
    knowledgedb/knowledgedb.cpp
    knowledgedb/timezonedb.cpp
    knowledgedb/trainstationdb.cpp

    pdf/pdfdocument.cpp
    pdf/pdfextractoroutputdevice.cpp
    pdf/pdfimage.cpp
    pdf/pdfvectorpicture.cpp
    pdf/popplerutils.cpp

    barcodedecoder.cpp
    calendarhandler.cpp
    extractor.cpp
    extractorengine.cpp
    extractorfilter.cpp
    extractorpostprocessor.cpp
    extractorrepository.cpp
    extractorutil.cpp
    flightpostprocessor.cpp
    genericpdfextractor.cpp
    genericpkpassextractor.cpp
    genericuic918extractor.cpp
    htmldocument.cpp
    iatabcbpparser.cpp
    jsonlddocument.cpp
    jsonldimportfilter.cpp
    locationutil.cpp
    mergeutil.cpp
    qimagepurebinarizer.cpp
    rct2ticket.cpp
    sortutil.cpp
    stringutil.cpp
    structureddataextractor.cpp
    uic9183parser.cpp
)
qt5_add_resources(kitinerary_lib_srcs extractors/extractors.qrc)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER logging.h IDENTIFIER KItinerary::Log CATEGORY_NAME org.kde.kitinerary)

add_library(KPimItinerary ${kitinerary_lib_srcs})
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
    PRIVATE
        Qt5::Qml
        KF5::I18n
        KF5::Contacts
        KPim::PkPass
        ${ZLIB_LIBRARIES}
)
if (HAVE_POPPLER)
    target_link_libraries(KPimItinerary PRIVATE Poppler::Core)
endif()

if (HAVE_ZXING)
    target_link_libraries(KPimItinerary PRIVATE ZXing::Core)
endif()

if (HAVE_KCAL)
    target_link_libraries(KPimItinerary PUBLIC KF5::CalendarCore)
endif()
if (HAVE_LIBXML2)
    target_compile_definitions(KPimItinerary PRIVATE ${LIBXML2_DEFINITIONS})
    target_include_directories(KPimItinerary PRIVATE ${LIBXML2_INCLUDE_DIR})
    target_link_libraries(KPimItinerary PRIVATE ${LIBXML2_LIBRARIES})
endif()
if (HAVE_PHONENUMBER)
    target_link_libraries(KPimItinerary PRIVATE PhoneNumber::PhoneNumber)
endif()

ecm_generate_headers(KItinerary_FORWARDING_HEADERS
    HEADER_NAMES
        BarcodeDecoder
        CalendarHandler
        Extractor
        ExtractorEngine
        ExtractorPostprocessor
        HtmlDocument
        IataBcbpParser
        JsonLdDocument
        LocationUtil
        MergeUtil
        Rct2Ticket
        SortUtil
        Uic9183Parser
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_HEADERS
)
ecm_generate_headers(KItinerary_KnowledgeDb_FORWARDING_HEADERS
    HEADER_NAMES
        CountryDb
        KnowledgeDb
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_KnowledgeDb_HEADERS
    RELATIVE knowledgedb
)
ecm_generate_headers(KItinerary_Datatypes_FORWARDING_HEADERS
    HEADER_NAMES
        Action
        Brand
        BusTrip
        Datatypes
        Event
        Flight
        Organization
        Reservation
        RentalCar
        Person
        Place
        Taxi
        Ticket
        TrainTrip
        Visit
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Datatypes_HEADERS
    RELATIVE datatypes
)
ecm_generate_headers(KItinerary_Pdf_FORWARDING_HEADERS
    HEADER_NAMES
        PdfDocument
        PdfImage
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Pdf_HEADERS
    RELATIVE pdf
)

install(TARGETS KPimItinerary EXPORT KPimItineraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KItinerary_FORWARDING_HEADERS}
    ${KItinerary_KnowledgeDb_FORWARDING_HEADERS}
    ${KItinerary_Datatypes_FORWARDING_HEADERS}
    ${KItinerary_Pdf_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/KItinerary
)
install(FILES
    ${KItinerary_HEADERS}
    ${KItinerary_AirportDb_HEADERS}
    ${KItinerary_Datatypes_HEADERS}
    ${KItinerary_KnowledgeDb_HEADERS}
    ${KItinerary_Pdf_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary
)
