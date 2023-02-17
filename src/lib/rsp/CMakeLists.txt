# SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/keys/dummy.json
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/rsp6-key-downloader.py --output ${CMAKE_CURRENT_SOURCE_DIR}/keys/
    COMMENT "Updating RSP-6 public keys"
)
add_custom_target(rsp6-update-keys DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/keys/dummy.json)
