/**
 * @file glm.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Helper header to include GLM with all options set.
 */

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/functions.hpp>
#include <glm/gtx/quaternion.hpp>

#define LAYOUT_SCALAR alignas(4)
#define LAYOUT_FLOAT LAYOUT_SCALAR
#define LAYOUT_INT LAYOUT_SCALAR
#define LAYOUT_UINT LAYOUT_SCALAR
#define LAYOUT_STRUCT alignas(16)
#define LAYOUT_VEC2 alignas(8)
#define LAYOUT_VEC3 alignas(16)
#define LAYOUT_VEC4 alignas(16)
#define LAYOUT_MAT2 alignas(8)
#define LAYOUT_MAT3 alignas(16)
#define LAYOUT_MAT4 alignas(16)
