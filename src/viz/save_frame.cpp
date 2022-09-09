#include "save_frame.h"

#include <glbinding/gl46core/gl.h>
#include <bitmap_image.h>
#include <config.h>
#include <lodepng.h>
#include <logger.h>

void viz::save_frame::saveBmp(std::string fileName) {
    struct Pixel {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
    };

    std::vector<Pixel> pixels(common::windowWidth * common::windowHeight);

    gl::glPixelStorei(gl::GL_PACK_ALIGNMENT, 1);
    gl::glReadPixels(0, 0, common::windowWidth, common::windowHeight, gl::GL_RGB, gl::GL_UNSIGNED_BYTE, pixels.data());

    bitmap_image image(common::windowWidth, common::windowHeight);

    auto data = reinterpret_cast<uint8_t *>(pixels.data());

    for (std::size_t y = 0; y < image.height(); ++y) {
        for (std::size_t x = 0; x < image.width(); ++x) {
            const auto r = *(data++);
            const auto g = *(data++);
            const auto b = *(data++);

            image.set_pixel(x, y, r, g, b);
        }
    }

    image.save_image(fileName);
}

void viz::save_frame::savePng(std::string fileName) {
    std::vector<std::uint8_t> pixels(common::windowWidth * common::windowHeight * 4);

    gl::glPixelStorei(gl::GL_PACK_ALIGNMENT, 1);
    gl::glReadPixels(0, 0, common::windowWidth, common::windowHeight, gl::GL_RGBA, gl::GL_UNSIGNED_BYTE, pixels.data());

    unsigned error = lodepng::encode(fileName.c_str(), pixels.data(), common::windowWidth, common::windowHeight);

    if (error) {
        std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
    }

    spdlog::info("Frame saved as {}", fileName);
}

void viz::save_frame::save(std::string fileName, Type type) {
    switch (type) {
        case Type::BMP:
            return saveBmp(fileName + ".bmp");
        case Type::PNG:
            return savePng(fileName + ".png");
    }
}