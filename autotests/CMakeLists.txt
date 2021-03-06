# SPDX-FileCopyrightText: 2018-2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt5Test ${QT_REQUIRED_VERSION} CONFIG REQUIRED)
add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

ecm_add_test(stringutiltest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(datatypestest.cpp LINK_LIBRARIES Qt::Test Qt::Qml KPim::Itinerary)
ecm_add_test(jsonlddocumenttest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(mergeutiltest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(locationutiltest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(knowledgedbtest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(airportdbtest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractorresulttest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractordocumentnodetest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractorfiltertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractorrepositorytest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractorscriptenginetest.cpp extractorscriptenginetest.qrc TEST_NAME extractorscriptenginetest LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(berdecodertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(berencodertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(bcbpparsertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(uic9183parsertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(vdvtickettest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(rct2parsertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(jsapitest.cpp ../src/lib/jsapi/jsonld.cpp TEST_NAME jsapitest LINK_LIBRARIES Qt::Test KPim::Itinerary Qt::Qml)
ecm_add_test(bitarraytest.cpp ../src/lib/jsapi/bitarray.cpp TEST_NAME bitarraytest LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(structureddataextractortest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(pdfdocumenttest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary Qt::Gui)
ecm_add_test(htmldocumenttest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(barcodedecodertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary Qt::Gui)
ecm_add_test(pkpassextractortest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary KPim::PkPass)
ecm_add_test(extractorutiltest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(postprocessortest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(extractorvalidatortest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
if (TARGET KF5::CalendarCore)
    ecm_add_test(calendarhandlertest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
    ecm_add_test(extractortest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary KPim::PkPass)
endif()
ecm_add_test(documentutiltest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary)
ecm_add_test(filetest.cpp LINK_LIBRARIES Qt::Test KPim::Itinerary KPim::PkPass)
