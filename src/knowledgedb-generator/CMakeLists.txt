add_executable(generate-knowledgedb
    main.cpp
    codegen.cpp
    timezones.cpp
    wikidata.cpp
    airportdbgenerator.cpp
    countrydbgenerator.cpp
    timezonedbgenerator.cpp
    trainstationdbgenerator.cpp
    util.cpp
)
target_include_directories(generate-knowledgedb PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../knowledgedb)
target_link_libraries(generate-knowledgedb PRIVATE Qt5::Network Qt5::Gui)

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

set(outfiles "")
macro(generate_db dbtype outfile)
    add_custom_command(
        OUTPUT ${outfile}
        COMMAND generate-knowledgedb -o ${CMAKE_CURRENT_SOURCE_DIR}/../knowledgedb/${outfile} -d ${dbtype}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.colormap
        ${CMAKE_CURRENT_SOURCE_DIR}/timezones.png
    )
    list(APPEND outfiles ${outfile})
endmacro()
generate_db(country countrydb_data.cpp)
generate_db(timezone timezonedb_data.cpp)
generate_db(timezoneheader timezonedb_data_p.h)
generate_db(airport airportdb_data.cpp)
generate_db(trainstation trainstationdb_data.cpp)

add_custom_target(rebuild-knowledgedb DEPENDS ${outfiles} ${CMAKE_CURRENT_SOURCE_DIR}/timezones.png)
