FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp)
    add_library(imgui::imgui ALIAS imgui)

    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/backends)

    add_library(imgui_glfw STATIC ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
    add_library(imgui::imgui_glfw ALIAS imgui_glfw)
    target_link_libraries(imgui_glfw PUBLIC imgui)
    target_link_libraries(imgui_glfw PRIVATE glfw)
endif()
