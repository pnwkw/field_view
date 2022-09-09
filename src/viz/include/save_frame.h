#ifndef VIZ_SAVE_FRAME_H
#define VIZ_SAVE_FRAME_H

#include <string>

namespace viz {
    class save_frame {
    private:
        static void saveBmp(std::string fileName);
        static void savePng(std::string fileName);
    public:
        enum Type {
            BMP,
            PNG
        };

        static void save(std::string fileName, Type type);
    };
}// namespace viz

#endif //VIZ_SAVE_FRAME_H
