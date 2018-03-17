add_subdirectory(airportdb)

if(TARGET Poppler::Qt5)
    set(HAVE_POPPLER ON)
endif()
configure_file(config-semantic.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-semantic.h)

set(kitinerary_lib_srcs
    airportdb/airportdb.cpp
    calendarhandler.cpp
    datatypes.cpp
    extractor.cpp
    extractorengine.cpp
    extractorfilter.cpp
    extractorpreprocessor.cpp
    extractorpostprocessor.cpp
    extractorrepository.cpp
    jsonlddocument.cpp
    structureddataextractor.cpp
)
qt5_add_resources(kitinerary_lib_srcs extractors/extractors.qrc)
ecm_qt_declare_logging_category(kitinerary_lib_srcs HEADER semantic_debug.h IDENTIFIER SEMANTIC_LOG CATEGORY_NAME org.kde.kitinerary)

add_library(KItinerary STATIC ${kitinerary_lib_srcs})
target_link_libraries(KItinerary
    PUBLIC
        Qt5::Core
        KF5::Mime
        KF5::CalendarCore
    PRIVATE
        Qt5::Qml
        KF5::I18n
)
if (HAVE_POPPLER)
    target_link_libraries(KItinerary PRIVATE Poppler::Qt5)
endif()

install(TARGETS KItinerary EXPORT KItineraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
