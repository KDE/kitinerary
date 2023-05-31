# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(ticket-barcode-dump ticket-barcode-dump.cpp)
target_include_directories(ticket-barcode-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(ticket-barcode-dump KPim${KF_MAJOR_VERSION}Itinerary)

add_executable(extractor-document-dump extractor-document-dump.cpp)
target_include_directories(extractor-document-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(extractor-document-dump KPim${KF_MAJOR_VERSION}Itinerary)

add_executable(uper-strings uper-strings.cpp)
target_include_directories(uper-strings PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(uper-strings KPim${KF_MAJOR_VERSION}Itinerary)

add_executable(plist-dump plist-dump.cpp)
target_include_directories(plist-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(plist-dump KPim${KF_MAJOR_VERSION}Itinerary)
