# SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (BUILD_TOOLS)
    add_subdirectory(rsp)
    add_subdirectory(vdv/certs)
endif()
configure_file(config-kitinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

add_library(KPim6Itinerary ${kitinerary_lib_srcs})
add_library(KPim6::Itinerary ALIAS KPim6Itinerary)

target_sources(KPim6Itinerary PRIVATE
    asn1/berelement.cpp
    asn1/bitvectorview.cpp
    asn1/uperdecoder.cpp
    asn1/uperelement.cpp

    datatypes/action.cpp
    datatypes/boattrip.cpp
    datatypes/brand.cpp
    datatypes/bustrip.cpp
    datatypes/creativework.cpp
    datatypes/event.cpp
    datatypes/flight.cpp
    datatypes/organization.cpp
    datatypes/person.cpp
    datatypes/place.cpp
    datatypes/programmembership.cpp
    datatypes/reservation.cpp
    datatypes/taxi.cpp
    datatypes/ticket.cpp
    datatypes/token.cpp
    datatypes/traintrip.cpp
    datatypes/rentalcar.cpp
    datatypes/visit.cpp

    engine/abstractextractor.cpp
    engine/extractordocumentnode.cpp
    engine/extractordocumentnodefactory.cpp
    engine/extractordocumentprocessor.cpp
    engine/extractorengine.cpp
    engine/extractorfilter.cpp
    engine/extractorrepository.cpp
    engine/extractorresult.cpp
    engine/extractorscriptengine.cpp
    engine/scriptextractor.cpp

    era/elbticket.cpp
    era/fcbticket.cpp
    era/fcbutil.cpp
    era/ssbticketbase.cpp
    era/ssbticketreader.cpp
    era/ssbv1ticket.cpp
    era/ssbv2ticket.cpp
    era/ssbv3ticket.cpp

    extractors/genericboardingpassextractor.cpp

    http/httpresponse.cpp

    iata/iatabcbp.cpp
    iata/iatabcbpparser.cpp
    iata/iatabcbpsections.cpp

    jsapi/barcode.cpp
    jsapi/bitarray.cpp
    jsapi/bytearray.cpp
    jsapi/extractorengine.cpp
    jsapi/jsonld.cpp

    json/jsonld.cpp
    json/jsonldfilterengine.cpp
    json/jsonldimportfilter.cpp

    knowledgedb/alphaid.cpp
    knowledgedb/airportdb.cpp
    knowledgedb/airportnametokenizer.cpp
    knowledgedb/countrydb.cpp
    knowledgedb/iatacode.cpp
    knowledgedb/knowledgedb.cpp
    knowledgedb/stationidentifier.cpp
    knowledgedb/timezonedb.cpp
    knowledgedb/trainstationdb.cpp

    pdf/pdfbarcodeutil.cpp
    pdf/pdfdocument.cpp
    pdf/pdfextractoroutputdevice.cpp
    pdf/pdfimage.cpp
    pdf/pdflink.cpp
    pdf/pdfvectorpicture.cpp
    pdf/popplerglobalparams.cpp
    pdf/popplerutils.cpp

    plist/plistreader.cpp

    processors/barcodedocumentprocessorhelper.cpp
    processors/binarydocumentprocessor.cpp
    processors/eradocumentprocessor.cpp
    processors/externalprocessor.cpp
    processors/htmldocumentprocessor.cpp
    processors/httpresponseprocessor.cpp
    processors/iatabcbpdocumentprocessor.cpp
    processors/icaldocumentprocessor.cpp
    processors/imagedocumentprocessor.cpp
    processors/jsonlddocumentprocessor.cpp
    processors/mimedocumentprocessor.cpp
    processors/pdfdocumentprocessor.cpp
    processors/pkpassdocumentprocessor.cpp
    processors/plistdocumentprocessor.cpp
    processors/textdocumentprocessor.cpp
    processors/uic9183documentprocessor.cpp
    processors/vdvdocumentprocessor.cpp

    protobuf/protobufstreamreader.cpp

    rsp/rsp6decoder.cpp
    rsp/keys/rsp6-keys.qrc

    scripts/extractors.qrc

    text/addressparser.cpp
    text/nameoptimizer.cpp
    text/terminalfinder.cpp
    text/timefinder.cpp

    uic9183/rct2ticket.cpp
    uic9183/uic9183block.cpp
    uic9183/uic9183head.cpp
    uic9183/uic9183header.cpp
    uic9183/uic9183parser.cpp
    uic9183/uic9183ticketlayout.cpp
    uic9183/uic9183utils.cpp
    uic9183/vendor0080block.cpp
    uic9183/vendor0080vublockdata.cpp
    uic9183/vendor1154block.cpp

    vdv/iso9796_2decoder.cpp
    vdv/vdvcertificate.cpp
    vdv/vdvticket.cpp
    vdv/vdvticketcontent.cpp
    vdv/vdvticketparser.cpp
    vdv/certs/vdv-certs.qrc

    barcodedecoder.cpp
    calendarhandler.cpp
    documentutil.cpp
    extractorcapabilities.cpp
    extractorpostprocessor.cpp
    extractorutil.cpp
    extractorvalidator.cpp
    file.cpp
    flightpostprocessor.cpp
    flightutil.cpp
    htmldocument.cpp
    jsonlddocument.cpp
    locationutil.cpp
    mergeutil.cpp
    qimagepurebinarizer.cpp
    sortutil.cpp
    stringutil.cpp
    tickettokencomparator.cpp
)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPim6Itinerary PROPERTIES UNITY_BUILD ON)
endif()

kde_source_files_enable_exceptions(barcodedecoder.cpp qimagepurebinarizer.cpp)
ecm_qt_declare_logging_category(KPim6Itinerary
    HEADER logging.h
    IDENTIFIER KItinerary::Log
    CATEGORY_NAME org.kde.kitinerary
    DESCRIPTION "KItinerary"
    EXPORT KITINERARY
)
ecm_qt_declare_logging_category(KPim6Itinerary
    HEADER compare-logging.h
    IDENTIFIER KItinerary::CompareLog
    CATEGORY_NAME org.kde.kitinerary.comparator
    DESCRIPTION "KItinerary Comparison/Merging Logging"
    EXPORT KITINERARY
)
ecm_qt_declare_logging_category(KPim6Itinerary
    HEADER validator-logging.h
    IDENTIFIER KItinerary::ValidatorLog
    CATEGORY_NAME org.kde.kitinerary.extractorValidator
    DESCRIPTION "KItinerary Extractor Validator Logging"
    EXPORT KITINERARY
)

generate_export_header(KPim6Itinerary BASE_NAME KItinerary)
set_target_properties(KPim6Itinerary PROPERTIES
    VERSION ${KITINERARY_VERSION}
    SOVERSION ${KITINERARY_SOVERSION}
    EXPORT_NAME Itinerary
)
target_include_directories(KPim6Itinerary INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_PIM}>")
target_include_directories(KPim6Itinerary PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KPim6Itinerary PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(KPim6Itinerary
    PUBLIC
        Qt::Core
        KPim6::Mime
    PRIVATE
        Qt::Qml
        KF6::Archive
        KF6::I18n
        KF6::I18nLocaleData
        KF6::CalendarCore
        KF6::Codecs
        KF6::Contacts
        KPim6::PkPass
        Poppler::Core
        OpenSSL::Crypto
        ZLIB::ZLIB
	Qt6::Core5Compat
)
if (TARGET ZXing::ZXing)
    target_link_libraries(KPim6Itinerary PRIVATE ZXing::ZXing)
elseif (TARGET ZXing::Core)
    target_link_libraries(KPim6Itinerary PRIVATE ZXing::Core)
endif()

if (HAVE_LIBXML2)
    target_compile_definitions(KPim6Itinerary PRIVATE ${LIBXML2_DEFINITIONS})
    target_include_directories(KPim6Itinerary PRIVATE ${LIBXML2_INCLUDE_DIR})
    target_link_libraries(KPim6Itinerary PRIVATE ${LIBXML2_LIBRARIES})
endif()
if (HAVE_PHONENUMBER)
    target_link_libraries(KPim6Itinerary PRIVATE PhoneNumber::PhoneNumber)
endif()

ecm_generate_headers(KItinerary_FORWARDING_HEADERS
    HEADER_NAMES
        BarcodeDecoder
        CalendarHandler
        DocumentUtil
        ExtractorCapabilities
        ExtractorPostprocessor
        ExtractorValidator
        File
        HtmlDocument
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
        BoatTrip
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
        ProgramMembership
        Taxi
        Ticket
        Token
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
        ExtractorDocumentNodeFactory
        ExtractorDocumentProcessor
        ExtractorEngine
        ExtractorFilter
        ExtractorRepository
        ExtractorResult
        ScriptExtractor
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Engine_HEADERS
    RELATIVE engine
)
ecm_generate_headers(KItinerary_Era_FORWARDING_HEADERS
    HEADER_NAMES
        ELBTicket
        SSBTicketBase
        SSBv1Ticket
        SSBv2Ticket
        SSBv3Ticket
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Era_HEADERS
    RELATIVE era
)
ecm_generate_headers(KItinerary_Http_FORWARDING_HEADERS
    HEADER_NAMES
        HttpResponse
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Http_HEADERS
    RELATIVE http
)
ecm_generate_headers(KItinerary_Iata_FORWARDING_HEADERS
    HEADER_NAMES
        IataBcbp
        IataBcbpSections
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Iata_HEADERS
    RELATIVE iata
)
ecm_generate_headers(KItinerary_Pdf_FORWARDING_HEADERS
    HEADER_NAMES
        PdfDocument
        PdfImage
        PdfLink
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Pdf_HEADERS
    RELATIVE pdf
)
ecm_generate_headers(KItinerary_Asn1_FORWARDING_HEADERS
    HEADER_NAMES
        BERElement
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Asn1_HEADERS
    RELATIVE asn1
)
ecm_generate_headers(KItinerary_Uic9183_FORWARDING_HEADERS
    HEADER_NAMES
        Rct2Ticket
        Uic9183Header
        Uic9183Block
        Uic9183Parser
        Uic9183TicketLayout
        Uic9183Utils
        Vendor0080Block
        Vendor0080VUBlockData
        Vendor1154Block
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Uic9183_HEADERS
    RELATIVE uic9183
)
ecm_generate_headers(KItinerary_Vdv_FORWARDING_HEADERS
    HEADER_NAMES
        VdvTicket
        VdvBasicTypes
        VdvTicketContent
        VdvTicketParser
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Vdv_HEADERS
    RELATIVE vdv
)

install(TARGETS KPim6Itinerary EXPORT KPim6ItineraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KItinerary_FORWARDING_HEADERS}
    ${KItinerary_Asn1_FORWARDING_HEADERS}
    ${KItinerary_Datatypes_FORWARDING_HEADERS}
    ${KItinerary_Engine_FORWARDING_HEADERS}
    ${KItinerary_Era_FORWARDING_HEADERS}
    ${KItinerary_Http_FORWARDING_HEADERS}
    ${KItinerary_Iata_FORWARDING_HEADERS}
    ${KItinerary_KnowledgeDb_FORWARDING_HEADERS}
    ${KItinerary_Pdf_FORWARDING_HEADERS}
    ${KItinerary_Uic9183_FORWARDING_HEADERS}
    ${KItinerary_Vdv_FORWARDING_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/KItinerary
)
install(FILES
    ${KItinerary_HEADERS}
    ${KItinerary_AirportDb_HEADERS}
    ${KItinerary_Asn1_HEADERS}
    ${KItinerary_Datatypes_HEADERS}
    ${KItinerary_KnowledgeDb_HEADERS}
    ${KItinerary_Engine_HEADERS}
    ${KItinerary_Era_HEADERS}
    ${KItinerary_Http_HEADERS}
    ${KItinerary_Iata_HEADERS}
    ${KItinerary_Pdf_HEADERS}
    ${KItinerary_Uic9183_HEADERS}
    ${KItinerary_Vdv_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary
)
install(FILES
    internal/parameter_type.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary/internal
)
if (NOT ANDROID)
    install(FILES application-vnd-kde-itinerary.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
    update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
endif()
ecm_qt_install_logging_categories(
    EXPORT KITINERARY
    FILE org_kde_kitinerary.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_QCH)
    ecm_add_qch(
        KPim6Itinerary_QCH
        NAME KPim6Itinerary
        BASE_NAME KPim6Itinerary
        VERSION ${PIM_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
        ${KItinerary_HEADERS}
        ${KItinerary_AirportDb_HEADERS}
        ${KItinerary_Datatypes_HEADERS}
        ${KItinerary_KnowledgeDb_HEADERS}
        ${KItinerary_Engine_HEADERS}
        ${KItinerary_Pdf_HEADERS}
        ${KItinerary_Uic9183_HEADERS}
        ${KItinerary_Vdv_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        #IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KSIEVEUI_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
