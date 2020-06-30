# SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause


find_program(OSMCONVERT_EXECUTABLE osmconvert)
find_program(OSMFILTER_EXECUTABLE osmfilter)
find_program(OSMUPDATE_EXECUTABLE osmupdate)

find_program(WGET_EXECUTABLE wget) # needed by osmupdate
find_program(RSYNC_EXECUTABLE rsync) # needed for the initial download


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OsmTools
    FOUND_VAR OsmTools_FOUND
    REQUIRED_VARS OSMCONVERT_EXECUTABLE OSMFILTER_EXECUTABLE OSMUPDATE_EXECUTABLE WGET_EXECUTABLE RSYNC_EXECUTABLE
)

if (OSMCONVERT_EXECUTABLE AND NOT TARGET OSM::convert)
    add_executable(OSM::convert IMPORTED)
    set_target_properties(OSM::convert PROPERTIES IMPORTED_LOCATION ${OSMCONVERT_EXECUTABLE})
endif()
if (OSMFILTER_EXECUTABLE AND NOT TARGET OSM::filter)
    add_executable(OSM::filter IMPORTED)
    set_target_properties(OSM::filter PROPERTIES IMPORTED_LOCATION ${OSMFILTER_EXECUTABLE})
endif()
if (OSMUPDATE_EXECUTABLE AND NOT TARGET OSM::update)
    add_executable(OSM::update IMPORTED)
    set_target_properties(OSM::update PROPERTIES IMPORTED_LOCATION ${OSMUPDATE_EXECUTABLE})
endif()

set_package_properties(OsmTools PROPERTIES
    URL "https://gitlab.com/osm-c-tools/osmctools"
    DESCRIPTION "Tols to convert, filter and update OpenStreetMap data files"
)

mark_as_advanced(OSMCONVERT_EXECUTABLE OSMFILTER_EXECUTABLE OSMCONVERT_EXECUTABLE WGET_EXECUTABLE)

set(OSM_PLANET_DIR "" CACHE PATH "Directory containing the planet-latest.o5m file, and enough space to store processing results in.")
set(OSM_MIRROR "ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org" CACHE STRING "Base URL of the preferred OSM download mirror.")

# create initial download and incremental update targets for the OSM planet file
if (OSM_PLANET_DIR)
    set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM ON) # avoid cleaning the expensive full planet files
    add_custom_command(
        OUTPUT ${OSM_PLANET_DIR}/planet-latest.osm.pbf
        COMMAND ${RSYNC_EXECUTABLE} -Lvz --partial --progress rsync://${OSM_MIRROR}/pbf/planet-latest.osm.pbf ${OSM_PLANET_DIR}/planet-latest.osm.pbf
        WORKING_DIRECTORY ${OSM_PLANET_DIR}
        COMMENT "Downloading full OSM plant file (~60GB)"
    )
    add_custom_command(
        OUTPUT ${OSM_PLANET_DIR}/planet-latest.o5m
        COMMAND OSM::convert ${OSM_PLANET_DIR}/planet-latest.osm.pbf --drop-author --drop-version --out-o5m -o=${OSM_PLANET_DIR}/planet-latest.o5m
        WORKING_DIRECTORY ${OSM_PLANET_DIR}
        COMMENT "Converting full OSM planet file to o5m format (takes ~30min and needs ~80GB of extra disk space)"
    )

    add_custom_target(osm-update-planet
        COMMAND OSM::update --base-url=https://${OSM_MIRROR}/replication --day --verbose ${OSM_PLANET_DIR}/planet-latest.o5m ${OSM_PLANET_DIR}/new-planet-latest.o5m
        COMMAND ${CMAKE_COMMAND} -E rename ${OSM_PLANET_DIR}/new-planet-latest.o5m ${OSM_PLANET_DIR}/planet-latest.o5m
        WORKING_DIRECTORY ${OSM_PLANET_DIR}
        DEPENDS ${OSM_PLANET_DIR}/planet-latest.o5m
        COMMENT "Updating OSM planet file"
    )
endif()

if (TARGET OSM::convert)

# Convert the given input file to the output file with determining the output format
# from the file extension
# Arguments:
#   INPUT input file, assumed to be in OSM_PLANET_DIR
#   OUTPUT output file, assumed to be in OSM_PLANET_DIR
#   ADD_BBOX bool, enable injection of bounding box tags
function(osm_convert)
    set(optionArgs ADD_BBOX)
    set(oneValueArgs INPUT OUTPUT)
    cmake_parse_arguments(osm_convert "${optionArgs}" "${oneValueArgs}" "" ${ARGN})
    get_filename_component(format ${osm_convert_OUTPUT} LAST_EXT)
    string(SUBSTRING ${format} 1 -1 format)

    set(extra_args "")
    if (osm_convert_ADD_BBOX)
        set(extra_args "--add-bbox-tags")
    endif()

    add_custom_command(
        OUTPUT ${OSM_PLANET_DIR}/${osm_convert_OUTPUT}
        COMMAND OSM::convert ${OSM_PLANET_DIR}/${osm_convert_INPUT} --drop-author --drop-version ${extra_args} --out-${format} -o=${OSM_PLANET_DIR}/${osm_convert_OUTPUT}
        WORKING_DIRECTORY ${OSM_PLANET_DIR}
        COMMENT "Converting ${osm_convert_INPUT} to ${format} format"
        DEPENDS ${OSM_PLANET_DIR}/${osm_convert_INPUT}
    )
endfunction()

endif()


if (TARGET OSM::filter)

# Filter the given input file by the given filter arguments
# Arguments:
#   INPUT input file, assumed to be in OSM_PLANET_DIR (default is planet-latest.o5m)
#   OUTPUT output file, assumed to be in OSM_PLANET_DIR
#   FILTER filter arguments for osmfilter
function(osm_filter)
    set(oneValueArgs INPUT OUTPUT)
    set(multiValueArgs FILTER)
    cmake_parse_arguments(osm_filter "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    get_filename_component(format ${osm_filter_OUTPUT} LAST_EXT)
    string(SUBSTRING ${format} 1 -1 format)

    if (NOT osm_filter_INPUT)
        set(osm_filter_INPUT planet-latest.o5m)
    endif()

    add_custom_command(
        OUTPUT ${OSM_PLANET_DIR}/${osm_filter_OUTPUT}
        COMMAND OSM::filter ${OSM_PLANET_DIR}/${osm_filter_INPUT} --drop-author --drop-version ${osm_filter_FILTER} --out-${format} -o=${OSM_PLANET_DIR}/${osm_filter_OUTPUT}
        WORKING_DIRECTORY ${OSM_PLANET_DIR}
        COMMENT "Filtering ${osm_filter_INPUT}"
        DEPENDS ${OSM_PLANET_DIR}/${osm_filter_INPUT}
    )
endfunction()

endif()
