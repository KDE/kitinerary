add_executable(generate-airportdb generatedb.cpp timezones.cpp)
target_link_libraries(generate-airportdb Qt5::Network Qt5::Gui)

find_program(XSLTPROC_EXECUTABLE xsltproc)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/timezones.colormap
    COMMAND
        ${XSLTPROC_EXECUTABLE} --novalid
            ${CMAKE_CURRENT_SOURCE_DIR}/timezones.xsl
            ${CMAKE_CURRENT_SOURCE_DIR}/timezones.qgs
            > ${CMAKE_CURRENT_SOURCE_DIR}/timezones.colormap
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.xsl
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.qgs
)

add_custom_target(
    rebuild-airportdb
    COMMAND generate-airportdb -o ${CMAKE_CURRENT_SOURCE_DIR}/airportdb_p.cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.colormap
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.png
)
