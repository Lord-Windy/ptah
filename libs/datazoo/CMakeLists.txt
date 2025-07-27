ptah_add_library(datazoo
    SOURCES
        src/honeycomb.c
    PUBLIC_HEADERS
        include/datazoo.h
    DEPENDENCIES
        samrena
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(datazoo_test
        test/datazoo_test.c
        test/honeycomb_tests.c
    )
    target_link_libraries(datazoo_test PRIVATE datazoo samrena)
    add_test(NAME datazoo_test COMMAND datazoo_test)
endif()