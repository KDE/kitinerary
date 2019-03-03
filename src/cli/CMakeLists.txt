if (ANDROID)
    return()
endif()

add_executable(kitinerary-extractor main.cpp)
target_include_directories(kitinerary-extractor PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(kitinerary-extractor
    KPimItinerary
    KPim::PkPass
)
install(TARGETS kitinerary-extractor ${INSTALL_TARGETS_DEFAULT_ARGS})
