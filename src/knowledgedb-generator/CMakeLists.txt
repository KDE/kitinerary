# SPDX-FileCopyrightText: 2018-2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(generate-knowledgedb
    main.cpp
    codegen.cpp
    timezones.cpp
    wikidata.cpp
    airportdbgenerator.cpp
    countrydbgenerator.cpp
    osmairportdb.cpp
    trainstationdbgenerator.cpp
    util.cpp
    ../lib/stringutil.cpp
    ../lib/knowledgedb/airportnametokenizer.cpp
    ../lib/knowledgedb/alphaid.cpp
    ../lib/knowledgedb/iatacode.cpp
    ../lib/knowledgedb/stationidentifier.cpp
)
target_compile_definitions(generate-knowledgedb PRIVATE "KITINERARY_STATIC_DEFINE")
target_include_directories(generate-knowledgedb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/knowledgedb
    ${CMAKE_CURRENT_BINARY_DIR}/../lib
)
target_link_libraries(generate-knowledgedb PRIVATE Qt${QT_MAJOR_VERSION}::Network Qt${QT_MAJOR_VERSION}::Gui KOSM KF5::Codecs)
# extract all elements we are interested in (airports, terminals, stations at airports)
osm_filter(OUTPUT airports.o5m FILTER --keep=\"iata=* or aeroway=terminal or public_transport=station or railway=station or railway=halt or railway=tram_stop\")

# augment bounding box information
osm_convert(INPUT airports.o5m OUTPUT airports-bbox.o5m ADD_BBOX ON)

# convert the output to OSM XML format, so the code generator can consume it
osm_convert(INPUT airports-bbox.o5m OUTPUT airports-bbox.osm)

set(outfiles "")
function(generate_db dbtype outfile)
    set(extra_args "")
    if (ARGN)
        set(extra_args "--osm-data=${ARGN}")
    endif()
    add_custom_command(
        OUTPUT ${outfile}
        COMMAND generate-knowledgedb -o ${CMAKE_CURRENT_SOURCE_DIR}/../lib/knowledgedb/${outfile} -d ${dbtype} ${extra_args}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS
        ${ARGN}
        generate-knowledgedb
    )
    list(APPEND outfiles ${outfile})
    set(outfiles ${outfiles} PARENT_SCOPE)
endfunction()
generate_db(country countrydb_data.cpp)
generate_db(airport airportdb_data.cpp ${OSM_PLANET_DIR}/airports-bbox.osm)
generate_db(trainstation trainstationdb_data.cpp)

add_custom_target(rebuild-knowledgedb DEPENDS ${outfiles})
