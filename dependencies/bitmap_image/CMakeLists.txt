add_library(bitmap_image INTERFACE)
add_library(bitmap_image::bitmap_image ALIAS bitmap_image)

target_include_directories(bitmap_image INTERFACE include)