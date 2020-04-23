add_library(KOSM STATIC
    datatypes.cpp
    element.cpp
    geomath.cpp
    xmlparser.cpp
)

target_include_directories(KOSM PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KOSM PUBLIC Qt5::Core)
