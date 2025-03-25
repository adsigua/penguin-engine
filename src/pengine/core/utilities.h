#pragma once
#ifndef PENGUIN_UTILITIES_H
#define PENGUIN_UTILITIES_H

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
//
//
//#define VK_CHECK(x)                                                     \
//    do {                                                                \
//        VkResult err = x;                                               \
//        if (err) {                                                      \
//            fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
//            abort();                                                    \
//        }                                                               \
//    } while (0)

namespace PenguinEngine {
    

namespace Utilities {
    std::string GetMatrixString(glm::mat4 mat) {
        return
            "\n{ " + std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + ", " + std::to_string(mat[3][0]) + "\n" +
            "  " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[3][1]) + "\n" +
            "  " + std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[3][2]) + "\n" +
            "  " + std::to_string(mat[0][3]) + ", " + std::to_string(mat[1][3]) + ", " + std::to_string(mat[2][3]) + ", " + std::to_string(mat[3][3]) + " }";
    }
}
}

#endif