# SPDX-FileCopyrightText: 2021 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (ANDROID)
    return()
endif()

add_executable(kitinerary-extractor main.cpp)
target_include_directories(kitinerary-extractor PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kitinerary-extractor
    KPimItinerary
    KPim::PkPass
)

option(KITINERARY_STANDALONE_CLI_EXTRACTOR "Build stand-alone command line extractor (this should be off, unless you are building the dedicated Flatpak for this" OFF)

if (KITINERARY_STANDALONE_CLI_EXTRACTOR)
    install(TARGETS kitinerary-extractor DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
    install(PROGRAMS org.kde.kitinerary-extractor.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.kitinerary-extractor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
else()
    install(TARGETS kitinerary-extractor DESTINATION ${KF5_LIBEXEC_INSTALL_DIR})
endif()
