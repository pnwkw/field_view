FetchContent_GetProperties(egl)

if(NOT egl_POPULATED)
    FetchContent_Populate(egl)
    add_library(egl INTERFACE)
    add_library(egl::egl ALIAS egl)
    target_include_directories(egl INTERFACE ${egl_SOURCE_DIR}/api)
endif()
