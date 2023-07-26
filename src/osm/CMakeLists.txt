# SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KOSM STATIC
    datatypes.cpp datatypes.h
    element.cpp element.h
    geomath.cpp geomath.h
    xmlparser.cpp xmlparser.h
)

target_include_directories(KOSM PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(KOSM PUBLIC Qt::Core)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(KOSM PROPERTIES UNITY_BUILD ON)
endif()
