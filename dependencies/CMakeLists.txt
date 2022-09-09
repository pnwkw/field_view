include(FetchContent)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG 9aae45eb4a05a5a1f96be1ef37eb503a12ceb889 # 1.88
)

add_subdirectory(imgui)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e # 0.9.9.8
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        imguizmo
        GIT_REPOSITORY https://github.com/BrutPitt/imGuIZMO.quat
        GIT_TAG fcec1fcb28c2a18cfbaaa8d48cf09718e2235246 # v3.00
)

add_subdirectory(imguizmo)

FetchContent_Declare(
        imguicolortextedit
        GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit
        GIT_TAG master
)

add_subdirectory(imguicolortextedit)

FetchContent_Declare(
        glbinding
        GIT_REPOSITORY https://github.com/cginternals/glbinding
        GIT_TAG 28d32d9bbc72aedf815f18113b0bd3aa7b354108 # 3.10
)

add_subdirectory(glbinding)

if (DEFINED USE_GLFW_CONTEXT)
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw
            GIT_TAG 45ce5ddd197d5c58f50fdd3296a5131c894e5527 # 3.3.7
    )

    add_subdirectory(glfw)
endif (DEFINED USE_GLFW_CONTEXT)

FetchContent_Declare(
        egl
        GIT_REPOSITORY https://github.com/KhronosGroup/EGL-Registry
        GIT_TAG main
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
)

add_subdirectory(egl)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG 6aebf09233951e4ce30a63919186a70b2b195756 # v3.10.5
)

FetchContent_MakeAvailable(json)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog
        GIT_TAG 76fb40d95455f249bd70824ecfcae7a8f0930fa3 # 1.10.0
)

FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
        lodepng
        GIT_REPOSITORY https://github.com/lvandeve/lodepng
        GIT_TAG master
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
)

add_subdirectory(lodepng)

add_subdirectory(bitmap_image)
