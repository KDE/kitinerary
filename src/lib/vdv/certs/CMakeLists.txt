# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(vdv-cert-downloader
    cert-downloader.cpp
    ../vdvcertificate.cpp
    ../iso9796_2decoder.cpp
)

target_include_directories(vdv-cert-downloader PRIVATE ${CMAKE_BINARY_DIR}/src/lib ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(vdv-cert-downloader PRIVATE Qt${QT_MAJOR_VERSION}::Core OpenSSL::Crypto KPimItinerary)

add_custom_target(vdv-download-certs COMMAND vdv-cert-downloader WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
