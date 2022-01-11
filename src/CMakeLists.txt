# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(osm)
if (TARGET Qt${QT_MAJOR_VERSION}::Network AND NOT CMAKE_CROSSCOMPILING AND OSM_PLANET_DIR AND OsmTools_FOUND)
    add_subdirectory(knowledgedb-generator)
endif()
add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(tools)
