FetchContent_GetProperties(lodepng)

if(NOT lodepng_POPULATED)
    FetchContent_Populate(lodepng)
    add_library(lodepng STATIC ${lodepng_SOURCE_DIR}/lodepng.cpp)
    add_library(lodepng::lodepng ALIAS lodepng)

    target_include_directories(lodepng PUBLIC ${lodepng_SOURCE_DIR})
endif()
