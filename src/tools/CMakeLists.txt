add_executable(ticket-barcode-dump ticket-barcode-dump.cpp)
target_include_directories(ticket-barcode-dump PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(ticket-barcode-dump KPimItinerary)
