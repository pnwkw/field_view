FetchContent_GetProperties(imguizmo)
FetchContent_GetProperties(imgui)

if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
    add_library(imguizmo STATIC ${imguizmo_SOURCE_DIR}/imGuIZMO.quat/imGuIZMOquat.cpp)
    add_library(imgui::imguizmo ALIAS imguizmo)

    target_compile_definitions(imguizmo PUBLIC IMGUIZMO_IMGUI_FOLDER=${imgui_SOURCE_DIR}/)
    target_compile_definitions(imguizmo PUBLIC VGIZMO_USES_GLM)

    target_link_libraries(imguizmo PUBLIC glm::glm)

    target_include_directories(imguizmo PUBLIC ${imguizmo_SOURCE_DIR}/imGuIZMO.quat)
endif()
