/**
 * @file transform.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Transform` structure.
 */

#pragma once

#include <ivulk/config.hpp>
#include <ivulk/glm.hpp>

#include <ivulk/utils/units/length.hpp>

#include <utility>

namespace ivulk {
    /**
	 * @brief A 3D transform with translation, rotation and scale components.
	 */
    struct Transform
    {
        /**
		 * @brief The translation component of the transform
		 */
        Position translate;

        /**
		 * @brief The rotation component of the transform
		 */
        glm::quat rotation = glm::quat_identity<float, glm::defaultp>();

        /**
		 * @brief The scale component of the transform
		 */
        glm::vec3 scale = {1, 1, 1};

        /**
		 * @brief Convert to a 4x4 matrix suitable for use as a view matrix in shaders
		 * 
		 * @return A model matrix created from this transform
		 */
        [[nodiscard]] glm::mat4 modelMatrix() const { return matrix(false); }

        /**
		 * @brief Convert to a 4x4 matrix suitable for use as a view matrix in shaders
		 * 
		 * @return A view matrix created from this transform
		 */
        [[nodiscard]] glm::mat4 viewMatrix() const { return matrix(true); }

        /**
		 * @brief Convert to a 4x4 matrix
		 * 
		 * @param localRotation If @c true, treat @c rotation as relative to the transform, 
		 *                      otherwise @c rotation is in world space.
		 * @return A matrix created from this transform
		 */
        [[nodiscard]] glm::mat4 matrix(bool localRotation) const
        {
            glm::mat4 m = glm::scale(glm::mat4(1), scale);
            m           = m * glm::mat4(glm::mat3_cast(rotation));
            if (localRotation)
                m = m * glm::translate(glm::mat4(1), translate.toVec());
            else
                m = glm::translate(glm::mat4(1), translate.toVec()) * m;

            return m;
        }

        /**
		 * @brief Get a vector containing euler angles from the rotation component
		 * 
		 * @return The euler angles equivalent of @c rotation.
		 */
        [[nodiscard]] const glm::vec3 eulerAngles() const { return glm::eulerAngles(rotation); }

        /**
		 * @brief Get a copy of this @c Transform with a different translation component.
		 *
		 * @param xlate The value for the translation component of the output.
		 * @return A copy of this transform with @c translate set to @c xlate.
		 */
        [[nodiscard]] const Transform withTranslate(const Position xlate) const
        {
            return {
                .translate = std::move(xlate),
                .rotation  = rotation,
                .scale     = scale,
            };
        }

        /**
		 * @brief Get a copy of this @c Transform with a different rotation component.
		 *
		 * @param rot The value for the translation component of the output.
		 * @return A copy of this transform with @c rotation set to @c rot.
		 */
        [[nodiscard]] const Transform withRotation(const glm::quat rot) const
        {
            return {
                .translate = translate,
                .rotation  = std::move(rot),
                .scale     = scale,
            };
        }

        /**
		 * @brief Get a copy of this @c Transform with a different rotation component.
		 *
		 * @param euler The euler angles to use for the rotation component of the output.
		 * @return A copy of this transform with @c rotation set a quaternion made from @c euler.
		 */
        [[nodiscard]] const Transform withRotation(const glm::vec3 euler) const
        {
            return {
                .translate = translate,
                .rotation  = glm::quat(euler),
                .scale     = scale,
            };
        }

        /**
		 * @brief Get a copy of this @c Transform with a different scale component.
		 *
		 * @param scl The value for the scale component of the output.
		 * @return A copy of this transform with @c scale set to @c scl.
		 */
        [[nodiscard]] const Transform withScale(const glm::vec3 scl) const
        {
            return {
                .translate = translate,
                .rotation  = rotation,
                .scale     = std::move(scl),
            };
        }

        /**
		 * @brief Get a copy of this @c Transform with the rotation set to a direction vector.
		 *
		 * @param direction The forward direction vector the output should be rotated along.
		 * @param up The up direction vector. Defaults to `{0, 0, 1}`.
		 * @return A copy of this transform with @c rotion set to a quaternion 
		 *         made from @c direction and @up.
		 */
        [[nodiscard]] const Transform withDirection(const glm::vec3 direction,
                                                    const glm::vec3 up = {0, 0, 1}) const
        {
            auto _direction = glm::normalize(direction);
            auto _up        = glm::normalize(up);
            return {
                .translate = translate,
                .rotation  = glm::inverse(glm::quatLookAt(_direction, _up)),
                .scale     = scale,
            };
        }

        /**
		 * @brief Get a copy of this @c Transform with position and rotation components for a look-at transformtion.
		 *
		 * @param eye The position of the camera for the look-at transformation.
		 * @param target The position to face the camera towards for the look-at transformation.
		 * @param up The up direction vector. Defaults to `{0, 0, 1}`.
		 * @return A copy of this @c Transform with position and rotation components for a look-at transformtion.
		 */
        [[nodiscard]] const Transform
        withLookAt(const Position eye, const Position target, const glm::vec3 up = {0, 0, 1}) const
        {
            const auto direction = target - eye;
            Transform t {
                .translate = (eye * meter {-1}),
                .scale     = scale,
            };
            return t.withDirection(direction.toVec(), up);
        }
    };
} // namespace ivulk
