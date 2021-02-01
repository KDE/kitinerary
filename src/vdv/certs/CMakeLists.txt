if (NOT HAVE_OPENSSL_RSA)
    return()
endif()

add_executable(vdv-cert-downloader
    cert-downloader.cpp
    ../vdvcertificate.cpp
    ../iso9796_2decoder.cpp
    ../../tlv/berelement.cpp
)

target_include_directories(vdv-cert-downloader PRIVATE ${CMAKE_BINARY_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(vdv-cert-downloader PRIVATE Qt5::Core OpenSSL::Crypto)

add_custom_target(vdv-download-certs COMMAND vdv-cert-downloader WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
