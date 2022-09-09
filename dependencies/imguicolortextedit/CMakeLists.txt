FetchContent_GetProperties(imguicolortextedit)

if(NOT imguicolortextedit_POPULATED)
    FetchContent_Populate(imguicolortextedit)
    add_library(imguicolortextedit STATIC ${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp)
    add_library(imgui::textedit ALIAS imguicolortextedit)

    target_link_libraries(imguicolortextedit PUBLIC imgui::imgui)

    target_include_directories(imguicolortextedit PUBLIC ${imguicolortextedit_SOURCE_DIR})
endif()
