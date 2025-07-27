ptah_add_library(samrena
    SOURCES
        src/samrena.c
        src/samrena_vector.c
    PUBLIC_HEADERS
        include/samrena.h
)

# Build tests if testing is enabled
if(BUILD_TESTING)
    enable_testing()
    
    add_executable(samrena_test
        test/samrena_test.c
    )
    target_link_libraries(samrena_test PRIVATE samrena)
    add_test(NAME samrena_test COMMAND samrena_test)
    
    add_executable(samrena_vector_test
        test/samrena_vector_test.c
    )
    target_link_libraries(samrena_vector_test PRIVATE samrena)
    add_test(NAME samrena_vector_test COMMAND samrena_vector_test)
endif()