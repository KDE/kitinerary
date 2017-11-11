set(semantic_srcs
    datatypes.cpp
    jsonlddocument.cpp
    semantic_plugin.cpp
    semanticmemento.cpp
    semanticprocessor.cpp
    semanticrenderer.cpp
    semanticurlhandler.cpp
    structureddataextractor.cpp
)
qt5_add_resources(semantic_srcs templates.qrc)
ecm_qt_declare_logging_category(semantic_srcs HEADER semantic_debug.h IDENTIFIER SEMANTIC_LOG CATEGORY_NAME org.kde.pim.messageviewer.semantic)

add_library(messageviewer_bodypartformatter_semantic MODULE ${semantic_srcs})
target_compile_definitions(messageviewer_bodypartformatter_semantic PRIVATE -DTRANSLATION_DOMAIN=\"messageviewer_semantic_plugin\")
target_link_libraries(messageviewer_bodypartformatter_semantic
  KF5::MessageViewer
  KF5::I18n
  Grantlee5::Templates
  Qt5::DBus
)
install(TARGETS messageviewer_bodypartformatter_semantic DESTINATION ${KDE_INSTALL_PLUGINDIR}/messageviewer/bodypartformatter)
