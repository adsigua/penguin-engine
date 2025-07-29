#pragma once

#include "core_defines.h"
#include <glm/gtx/string_cast.hpp>

namespace penguin_engine {
namespace utilities {
    std::string GetMatrixString(glm::mat4 mat) {
        return
            "\n{ " + std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + ", " + std::to_string(mat[3][0]) + "\n" +
            "  " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[3][1]) + "\n" +
            "  " + std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[3][2]) + "\n" +
            "  " + std::to_string(mat[0][3]) + ", " + std::to_string(mat[1][3]) + ", " + std::to_string(mat[2][3]) + ", " + std::to_string(mat[3][3]) + " }";
    }
}
}
