# SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (BUILD_TOOLS)
    add_subdirectory(rsp)
    add_subdirectory(vdv/certs)
endif()
configure_file(config-kitinerary.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-kitinerary.h)

add_library(KPim6Itinerary ${kitinerary_lib_srcs})
add_library(KPim6::Itinerary ALIAS KPim6Itinerary)

target_sources(KPim6Itinerary PRIVATE
    asn1/berelement.cpp asn1/berelement.h
    asn1/bitvectorview.cpp asn1/bitvectorview.h
    asn1/uperdecoder.cpp asn1/uperdecoder.h
    asn1/uperelement.cpp asn1/uperelement.h

    datatypes/action.cpp datatypes/action.h
    datatypes/boattrip.cpp datatypes/boattrip.h
    datatypes/brand.cpp datatypes/brand.h
    datatypes/bustrip.cpp datatypes/bustrip.h
    datatypes/creativework.cpp datatypes/creativework.h
    datatypes/event.cpp datatypes/event.h
    datatypes/flight.cpp datatypes/flight.h
    datatypes/organization.cpp datatypes/organization.h
    datatypes/person.cpp datatypes/person.h
    datatypes/place.cpp datatypes/place.h
    datatypes/programmembership.cpp datatypes/programmembership.h
    datatypes/reservation.cpp datatypes/reservation.h
    datatypes/taxi.cpp datatypes/taxi.h
    datatypes/ticket.cpp datatypes/ticket.h
    datatypes/token.cpp datatypes/token.h
    datatypes/traintrip.cpp datatypes/traintrip.h
    datatypes/rentalcar.cpp datatypes/rentalcar.h
    datatypes/visit.cpp datatypes/visit.h

    engine/abstractextractor.cpp engine/abstractextractor.h
    engine/extractordocumentnode.cpp engine/extractordocumentnode.h
    engine/extractordocumentnodefactory.cpp engine/extractordocumentnodefactory.h
    engine/extractordocumentprocessor.cpp engine/extractordocumentprocessor.h
    engine/extractorengine.cpp engine/extractorengine.h
    engine/extractorfilter.cpp engine/extractorfilter.h
    engine/extractorrepository.cpp engine/extractorrepository.h
    engine/extractorresult.cpp engine/extractorresult.h
    engine/extractorscriptengine.cpp engine/extractorscriptengine_p.h
    engine/scriptextractor.cpp engine/scriptextractor.h

    era/dosipas1.cpp
    era/dosipas2.cpp
    era/dosipasfactory.cpp
    era/elbticket.cpp era/elbticket.h
    era/fcbextractor.cpp
    era/fcbticket1.cpp era/fcbticket1.h
    era/fcbticket2.cpp
    era/fcbticket3.cpp
    era/fcbutil.cpp era/fcbutil.h
    era/ssbticketbase.cpp era/ssbticketbase.h
    era/ssbticketreader.cpp era/ssbticketreader.h
    era/ssbv1ticket.cpp era/ssbv1ticket.h
    era/ssbv2ticket.cpp era/ssbv2ticket.h
    era/ssbv3ticket.cpp era/ssbv3ticket.h

    extractors/activitypubextractor.cpp extractors/activitypubextractor.h
    extractors/genericboardingpassextractor.cpp extractors/genericboardingpassextractor.h

    http/httpresponse.cpp http/httpresponse.h

    iata/iatabcbp.cpp iata/iatabcbp.h
    iata/iatabcbpparser.cpp iata/iatabcbpparser.h
    iata/iatabcbpsections.cpp iata/iatabcbpsections.h

    jsapi/barcode.cpp jsapi/barcode.h
    jsapi/bitarray.cpp jsapi/bitarray.h
    jsapi/bytearray.cpp jsapi/bytearray.h
    jsapi/extractorengine.cpp jsapi/extractorengine.h
    jsapi/jsonld.cpp jsapi/jsonld.h

    json/jsonld.cpp json/jsonld.h
    json/jsonldfilterengine.cpp json/jsonldfilterengine.h
    json/jsonldimportfilter.cpp json/jsonldimportfilter.h

    knowledgedb/alphaid.cpp knowledgedb/alphaid.h
    knowledgedb/airportdb.cpp knowledgedb/airportdb.h
    knowledgedb/airportnametokenizer.cpp knowledgedb/airportnametokenizer_p.h
    knowledgedb/countrydb.cpp knowledgedb/countrydb.h
    knowledgedb/iatacode.cpp knowledgedb/iatacode.h
    knowledgedb/knowledgedb.cpp knowledgedb/knowledgedb.h
    knowledgedb/stationidentifier.cpp knowledgedb/stationidentifier.h
    knowledgedb/timezonedb.cpp knowledgedb/timezonedb_p.h
    knowledgedb/trainstationdb.cpp knowledgedb/trainstationdb.h

    pdf/pdfbarcodeutil.cpp pdf/pdfbarcodeutil_p.h
    pdf/pdfdocument.cpp pdf/pdfdocument.h
    pdf/pdfextractoroutputdevice.cpp pdf/pdfextractoroutputdevice_p.h
    pdf/pdfimage.cpp pdf/pdfimage.h
    pdf/pdflink.cpp  pdf/pdflink.h
    pdf/pdfvectorpicture.cpp pdf/pdfvectorpicture_p.h
    pdf/popplerglobalparams.cpp pdf/popplerglobalparams_p.h
    pdf/popplerutils.cpp pdf/popplerutils_p.h

    plist/plistreader.cpp plist/plistreader_p.h

    processors/barcodedocumentprocessorhelper.cpp processors/barcodedocumentprocessorhelper.h
    processors/binarydocumentprocessor.cpp processors/binarydocumentprocessor.h
    processors/dosipasdocumentprocessor.cpp
    processors/eradocumentprocessor.cpp processors/eradocumentprocessor.h
    processors/externalprocessor.cpp processors/externalprocessor.h
    processors/genericpriceextractorhelper.cpp processors/genericpriceextractorhelper_p.h
    processors/htmldocumentprocessor.cpp processors/htmldocumentprocessor.h
    processors/httpresponseprocessor.cpp processors/httpresponseprocessor.h
    processors/iatabcbpdocumentprocessor.cpp processors/iatabcbpdocumentprocessor.h
    processors/icaldocumentprocessor.cpp processors/icaldocumentprocessor.h
    processors/imagedocumentprocessor.cpp processors/imagedocumentprocessor.h
    processors/jsonlddocumentprocessor.cpp processors/jsonlddocumentprocessor.h
    processors/mimedocumentprocessor.cpp processors/mimedocumentprocessor.h
    processors/pdfdocumentprocessor.cpp processors/pdfdocumentprocessor.h
    processors/pkpassdocumentprocessor.cpp processors/pkpassdocumentprocessor.h
    processors/plistdocumentprocessor.cpp processors/plistdocumentprocessor.h
    processors/textdocumentprocessor.cpp processors/textdocumentprocessor.h
    processors/uic9183documentprocessor.cpp processors/uic9183documentprocessor.h
    processors/vdvdocumentprocessor.cpp processors/vdvdocumentprocessor.h

    protobuf/protobufstreamreader.cpp protobuf/protobufstreamreader.h

    rsp/rsp6decoder.cpp rsp/rsp6decoder.h
    rsp/keys/rsp6-keys.qrc

    scripts/extractors.qrc

    text/addressparser.cpp text/addressparser_p.h
    text/nameoptimizer.cpp text/nameoptimizer_p.h
    text/pricefinder.cpp text/pricefinder_p.h
    text/terminalfinder.cpp text/terminalfinder_p.h
    text/timefinder.cpp text/timefinder_p.h

    uic9183/rct2ticket.cpp uic9183/rct2ticket.h
    uic9183/uic9183block.cpp uic9183/uic9183block.h
    uic9183/uic9183flex.cpp
    uic9183/uic9183head.cpp uic9183/uic9183head.h
    uic9183/uic9183header.cpp uic9183/uic9183header.h
    uic9183/uic9183parser.cpp uic9183/uic9183parser.h
    uic9183/uic9183ticketlayout.cpp uic9183/uic9183ticketlayout.h
    uic9183/uic9183utils.cpp uic9183/uic9183utils.h
    uic9183/vendor0080block.cpp uic9183/vendor0080block.h
    uic9183/vendor0080vublockdata.cpp uic9183/vendor0080vublockdata.h
    uic9183/vendor1154block.cpp uic9183/vendor1154block.h

    vdv/iso9796_2decoder.cpp vdv/iso9796_2decoder_p.h
    vdv/vdvcertificate.cpp vdv/vdvcertificate_p.h
    vdv/vdvticket.cpp vdv/vdvticket.h
    vdv/vdvticketcontent.cpp vdv/vdvticketcontent.h
    vdv/vdvticketparser.cpp vdv/vdvticketparser.h
    vdv/certs/vdv-certs.qrc

    barcodedecoder.cpp barcodedecoder.h
    calendarhandler.cpp calendarhandler.h
    documentutil.cpp documentutil.h
    extractorcapabilities.cpp extractorcapabilities.h
    extractorpostprocessor.cpp extractorpostprocessor.h
    extractorutil.cpp extractorutil.h
    extractorvalidator.cpp extractorvalidator.h
    file.cpp file.h
    flightpostprocessor.cpp flightpostprocessor_p.h
    flightutil.cpp flightutil_p.h
    htmldocument.cpp htmldocument.h
    jsonlddocument.cpp jsonlddocument.h
    locationutil.cpp locationutil.h
    mergeutil.cpp mergeutil.h
    priceutil.cpp priceutil.h
    reservationconverter.cpp reservationconverter.h
    sortutil.cpp sortutil.h
    stringutil.cpp stringutil.h
    tickettokencomparator.cpp tickettokencomparator_p.h
)
# doesn't build in unity mode
set_target_properties(KPim6Itinerary PROPERTIES UNITY_BUILD OFF)

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
if (TARGET libphonenumber::phonenumber-shared)
    target_link_libraries(KPim6Itinerary PRIVATE libphonenumber::phonenumber-shared)
elseif (TARGET PhoneNumber::PhoneNumber)
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
        PriceUtil
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
        Uic9183Head
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
    datatypes/datatypes_impl.h
    ${CMAKE_CURRENT_BINARY_DIR}/kitinerary_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_PIM}/kitinerary
)
install(FILES
    internal/instance_counter.h
    internal/parameter_type.h
    internal/strict_equal.h
    internal/strict_less.h
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
            KITINERARY_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
