# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_executable(ticket-barcode-dump ticket-barcode-dump.cpp)
target_include_directories(ticket-barcode-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(ticket-barcode-dump KPim6Itinerary)

add_executable(extractor-document-dump extractor-document-dump.cpp)
target_include_directories(extractor-document-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(extractor-document-dump KPim6Itinerary)

add_executable(uper-strings uper-strings.cpp)
target_include_directories(uper-strings PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(uper-strings KPim6Itinerary)

add_executable(plist-dump plist-dump.cpp)
target_include_directories(plist-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(plist-dump KPim6Itinerary)

add_executable(online-ticket-dump online-ticket-dump.cpp)
target_include_directories(online-ticket-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(online-ticket-dump PRIVATE KPim6Itinerary Qt::Network)
