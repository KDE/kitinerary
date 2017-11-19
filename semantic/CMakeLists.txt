# static lib for use by unit test
set(semantic_lib_srcs
    datatypes.cpp
    extractor.cpp
    extractorcontext.cpp
    extractorengine.cpp
    extractorfilter.cpp
    extractorpreprocessor.cpp
    extractorrepository.cpp
    extractorrule.cpp
    jsonlddocument.cpp
    structureddataextractor.cpp
)
qt5_add_resources(semantic_lib_srcs rules/rules.qrc)
ecm_qt_declare_logging_category(semantic_lib_srcs HEADER semantic_debug.h IDENTIFIER SEMANTIC_LOG CATEGORY_NAME org.kde.pim.messageviewer.semantic)
add_library(semantic_extractor STATIC ${semantic_lib_srcs})
set_target_properties(semantic_extractor PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(semantic_extractor PUBLIC Qt5::Core KF5::Mime)


set(semantic_plugin_srcs
    semantic_plugin.cpp
    semanticmemento.cpp
    semanticprocessor.cpp
    semanticrenderer.cpp
    semanticurlhandler.cpp
)
qt5_add_resources(semantic_plugin_srcs templates.qrc)

add_library(messageviewer_bodypartformatter_semantic MODULE ${semantic_plugin_srcs})
target_compile_definitions(messageviewer_bodypartformatter_semantic PRIVATE -DTRANSLATION_DOMAIN=\"messageviewer_semantic_plugin\")
target_link_libraries(messageviewer_bodypartformatter_semantic
  semantic_extractor
  KF5::MessageViewer
  KF5::CalendarSupport
  KF5::I18n
  Grantlee5::Templates
  Qt5::DBus
)
install(TARGETS messageviewer_bodypartformatter_semantic DESTINATION ${KDE_INSTALL_PLUGINDIR}/messageviewer/bodypartformatter)
