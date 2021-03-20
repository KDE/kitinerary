# SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(vdv/certs)
configure_file(config-kitinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

set(kitinerary_lib_srcs
    datatypes/action.cpp
    datatypes/brand.cpp
    datatypes/bustrip.cpp
    datatypes/creativework.cpp
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

    engine/abstractextractor.cpp
    engine/extractordocumentnode.cpp
    engine/extractordocumentprocessor.cpp
    engine/extractorfilter.cpp
    engine/extractorresult.cpp
    era/ssbticket.cpp

    extractors/iatabcbpextractor.cpp

    generic/genericextractor.cpp
    generic/genericicalextractor.cpp
    generic/genericpdfextractor.cpp
    generic/genericpkpassextractor.cpp
    generic/genericuic918extractor.cpp
    generic/genericvdvextractor.cpp
    generic/structureddataextractor.cpp

    jsapi/barcode.cpp
    jsapi/bitarray.cpp
    jsapi/context.cpp
    jsapi/jsonld.cpp

    knowledgedb/alphaid.cpp
    knowledgedb/airportdb.cpp
    knowledgedb/countrydb.cpp
    knowledgedb/iatacode.cpp
    knowledgedb/knowledgedb.cpp
    knowledgedb/stationidentifier.cpp
    knowledgedb/timezonedb.cpp
    knowledgedb/trainstationdb.cpp

    pdf/pdfdocument.cpp
    pdf/pdfextractoroutputdevice.cpp
    pdf/pdfimage.cpp
    pdf/pdfvectorpicture.cpp
    pdf/popplerglobalparams.cpp
    pdf/popplerutils.cpp

    scripts/extractors.qrc

    tlv/berelement.cpp

    uic9183/rct2ticket.cpp
    uic9183/uic9183block.cpp
    uic9183/uic9183head.cpp
    uic9183/uic9183header.cpp
    uic9183/uic9183parser.cpp
    uic9183/uic9183ticketlayout.cpp
    uic9183/uic9183utils.cpp
    uic9183/vendor0080block.cpp
    uic9183/vendor0080vublockdata.cpp

    vdv/iso9796_2decoder.cpp
    vdv/vdvcertificate.cpp
    vdv/vdvticket.cpp
    vdv/vdvticketcontent.cpp
    vdv/vdvticketparser.cpp
    vdv/certs/vdv-certs.qrc

    barcodedecoder.cpp
    calendarhandler.cpp
    documentutil.cpp
    extractor.cpp
    extractorcapabilities.cpp
    extractorengine.cpp
    extractorinput.cpp
    extractorpostprocessor.cpp
    extractorrepository.cpp
    extractorutil.cpp
    extractorvalidator.cpp
    file.cpp
    flightpostprocessor.cpp
    htmldocument.cpp
    iatabcbpparser.cpp
    jsonlddocument.cpp
    jsonldimportfilter.cpp
    locationutil.cpp
    mergeutil.cpp
    qimagepurebinarizer.cpp
    sortutil.cpp
    stringutil.cpp
)
kde_source_files_enable_exceptions(barcodedecoder.cpp qimagepurebinarizer.cpp)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER logging.h IDENTIFIER KItinerary::Log CATEGORY_NAME org.kde.kitinerary)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER compare-logging.h IDENTIFIER KItinerary::CompareLog CATEGORY_NAME org.kde.kitinerary.comparator)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER validator-logging.h IDENTIFIER KItinerary::ValidatorLog CATEGORY_NAME org.kde.kitinerary.extractorValidator)

add_library(KPimItinerary ${kitinerary_lib_srcs})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPimItinerary PROPERTIES UNITY_BUILD ON)
endif()
add_library(KPim::Itinerary ALIAS KPimItinerary)
generate_export_header(KPimItinerary BASE_NAME KItinerary)
set_target_properties(KPimItinerary PROPERTIES
    VERSION ${KITINERARY_VERSION_STRING}
    SOVERSION ${KITINERARY_SOVERSION}
    EXPORT_NAME Itinerary
)
target_include_directories(KPimItinerary INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_PIM}>")
target_include_directories(KPimItinerary PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KPimItinerary PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(KPimItinerary
    PUBLIC
        Qt5::Core
        KF5::Mime
    PRIVATE
        Qt5::Qml
        KF5::Archive
        KF5::I18n
        KF5::Contacts
        KPim::PkPass
        ${ZLIB_LIBRARIES}
)
if (HAVE_POPPLER)
    target_link_libraries(KPimItinerary PRIVATE Poppler::Core)
endif()

if (TARGET ZXing::ZXing)
    target_link_libraries(KPimItinerary PRIVATE ZXing::ZXing)
elseif (TARGET ZXing::Core)
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
if (HAVE_OPENSSL_RSA)
    target_link_libraries(KPimItinerary PRIVATE OpenSSL::Crypto)
endif()

ecm_generate_headers(KItinerary_FORWARDING_HEADERS
    HEADER_NAMES
        BarcodeDecoder
        CalendarHandler
        DocumentUtil
        Extractor
        ExtractorCapabilities
        ExtractorEngine
        ExtractorInput
        ExtractorPostprocessor
        ExtractorRepository
        ExtractorValidator
        File
        HtmlDocument
        IataBcbpParser
        JsonLdDocument
        LocationUtil
        MergeUtil
        SortUtil
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_HEADERS
)
ecm_generate_headers(KItinerary_KnowledgeDb_FORWARDING_HEADERS
    HEADER_NAMES
        AlphaId
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
        CreativeWork
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
ecm_generate_headers(KItinerary_Engine_FORWARDING_HEADERS
    HEADER_NAMES
        AbstractExtractor
        ExtractorDocumentNode
        ExtractorFilter
        ExtractorResult
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Engine_HEADERS
    RELATIVE engine
)
ecm_generate_headers(KItinerary_Pdf_FORWARDING_HEADERS
    HEADER_NAMES
        PdfDocument
        PdfImage
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Pdf_HEADERS
    RELATIVE pdf
)
ecm_generate_headers(KItinerary_Uic9183_FORWARDING_HEADERS
    HEADER_NAMES
        Rct2Ticket
        Uic9183Block
        Uic9183Parser
        Uic9183TicketLayout
        Uic9183Utils
        Vendor0080Block
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Uic9183_HEADERS
    RELATIVE uic9183
)

ecm_generate_headers(KItinerary_Vdv_FORWARDING_HEADERS
    HEADER_NAMES
        VdvTicket
        VdvTicketParser
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Vdv_HEADERS
    RELATIVE vdv
)

install(TARGETS KPimItinerary EXPORT KPimItineraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KItinerary_FORWARDING_HEADERS}
    ${KItinerary_KnowledgeDb_FORWARDING_HEADERS}
    ${KItinerary_Datatypes_FORWARDING_HEADERS}
    ${KItinerary_Engine_FORWARDING_HEADERS}
    ${KItinerary_Pdf_FORWARDING_HEADERS}
    ${KItinerary_Uic9183_FORWARDING_HEADERS}
    ${KItinerary_Vdv_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/KItinerary
)
install(FILES
    ${KItinerary_HEADERS}
    ${KItinerary_AirportDb_HEADERS}
    ${KItinerary_Datatypes_HEADERS}
    ${KItinerary_KnowledgeDb_HEADERS}
    ${KItinerary_Engine_HEADERS}
    ${KItinerary_Pdf_HEADERS}
    ${KItinerary_Uic9183_HEADERS}
    ${KItinerary_Vdv_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary
)
if (NOT ANDROID)
    install(FILES application-vnd-kde-itinerary.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
    update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
endif()
