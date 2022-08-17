# SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(vdv/certs)
configure_file(config-kitinerary.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

add_library(KPimItinerary ${kitinerary_lib_srcs})
add_library(KPim::Itinerary ALIAS KPimItinerary)

target_sources(KPimItinerary PRIVATE
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

    era/ssbticketbase.cpp
    era/ssbticketreader.cpp
    era/ssbv1ticket.cpp
    era/ssbv2ticket.cpp
    era/ssbv3ticket.cpp

    extractors/genericboardingpassextractor.cpp

    iata/iatabcbp.cpp
    iata/iatabcbpparser.cpp
    iata/iatabcbpsections.cpp

    jsapi/barcode.cpp
    jsapi/bitarray.cpp
    jsapi/bytearray.cpp
    jsapi/jsonld.cpp

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

    processors/barcodedocumentprocessorhelper.cpp
    processors/binarydocumentprocessor.cpp
    processors/externalprocessor.cpp
    processors/htmldocumentprocessor.cpp
    processors/iatabcbpdocumentprocessor.cpp
    processors/icaldocumentprocessor.cpp
    processors/imagedocumentprocessor.cpp
    processors/jsonlddocumentprocessor.cpp
    processors/mimedocumentprocessor.cpp
    processors/pdfdocumentprocessor.cpp
    processors/pkpassdocumentprocessor.cpp
    processors/ssbdocumentprocessor.cpp
    processors/textdocumentprocessor.cpp
    processors/uic9183documentprocessor.cpp
    processors/vdvdocumentprocessor.cpp

    scripts/extractors.qrc

    text/addressparser.cpp
    text/terminalfinder.cpp
    text/timefinder.cpp

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
)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KPimItinerary PROPERTIES UNITY_BUILD ON)
endif()

kde_source_files_enable_exceptions(barcodedecoder.cpp qimagepurebinarizer.cpp)
ecm_qt_declare_logging_category(KPimItinerary
    HEADER logging.h
    IDENTIFIER KItinerary::Log
    CATEGORY_NAME org.kde.kitinerary
    DESCRIPTION "KItinerary"
    EXPORT KITINERARY
)
ecm_qt_declare_logging_category(KPimItinerary
    HEADER compare-logging.h
    IDENTIFIER KItinerary::CompareLog
    CATEGORY_NAME org.kde.kitinerary.comparator
    DESCRIPTION "KItinerary Comparison/Merging Logging"
    EXPORT KITINERARY
)
ecm_qt_declare_logging_category(KPimItinerary
    HEADER validator-logging.h
    IDENTIFIER KItinerary::ValidatorLog
    CATEGORY_NAME org.kde.kitinerary.extractorValidator
    DESCRIPTION "KItinerary Extractor Validator Logging"
    EXPORT KITINERARY
)

generate_export_header(KPimItinerary BASE_NAME KItinerary)
set_target_properties(KPimItinerary PROPERTIES
    VERSION ${KITINERARY_VERSION}
    SOVERSION ${KITINERARY_SOVERSION}
    EXPORT_NAME Itinerary
)
target_include_directories(KPimItinerary INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_PIM}>")
target_include_directories(KPimItinerary PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KPimItinerary PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(KPimItinerary
    PUBLIC
        Qt::Core
        KF5::Mime
    PRIVATE
        Qt::Qml
        KF5::Archive
        KF5::I18n
        KF5::I18nLocaleData
        KF5::CalendarCore
        KF5::Codecs
        KF5::Contacts
        KPim::PkPass
        Poppler::Core
        OpenSSL::Crypto
        ZLIB::ZLIB
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(KPimItinerary PRIVATE Qt5::QmlPrivate)  # for QByteArray -> ArrayBuffer conversion
endif()

if (TARGET ZXing::ZXing)
    target_link_libraries(KPimItinerary PRIVATE ZXing::ZXing)
elseif (TARGET ZXing::Core)
    target_link_libraries(KPimItinerary PRIVATE ZXing::Core)
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
        SSBTicketBase
        SSBv1Ticket
        SSBv2Ticket
        SSBv3Ticket
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Era_HEADERS
    RELATIVE era
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
ecm_generate_headers(KItinerary_Tlv_FORWARDING_HEADERS
    HEADER_NAMES
        BERElement
    PREFIX KItinerary
    REQUIRED_HEADERS KItinerary_Tlv_HEADERS
    RELATIVE tlv
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

install(TARGETS KPimItinerary EXPORT KPimItineraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${KItinerary_FORWARDING_HEADERS}
    ${KItinerary_KnowledgeDb_FORWARDING_HEADERS}
    ${KItinerary_Datatypes_FORWARDING_HEADERS}
    ${KItinerary_Engine_FORWARDING_HEADERS}
    ${KItinerary_Era_FORWARDING_HEADERS}
    ${KItinerary_Iata_FORWARDING_HEADERS}
    ${KItinerary_Pdf_FORWARDING_HEADERS}
    ${KItinerary_Tlv_FORWARDING_HEADERS}
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
    ${KItinerary_Era_HEADERS}
    ${KItinerary_Iata_HEADERS}
    ${KItinerary_Tlv_HEADERS}
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
ecm_qt_install_logging_categories(
    EXPORT KITINERARY
    FILE org_kde_kitinerary.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_QCH)
    ecm_add_qch(
        KPimItinerary_QCH
        NAME KPimItinerary
        BASE_NAME KPimItinerary
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
            Qt5Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KSIEVEUI_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
