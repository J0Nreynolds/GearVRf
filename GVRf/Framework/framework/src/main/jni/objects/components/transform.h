/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/***************************************************************************
 * Containing data about how to position an object.
 ***************************************************************************/

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <mutex>
#include <memory>

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "objects/lazy.h"
#include "objects/components/component.h"

namespace gvr {
class Transform: public Component {
public:
    Transform();
    virtual ~Transform();

    static long long getComponentType() {
        return COMPONENT_TYPE_TRANSFORM;
    }

    void invalidate(bool rotationUpdated);
    glm::mat4 getModelMatrix(bool forceRecalculate = false);
    glm::mat4 getLocalModelMatrix();
    void translate(float x, float y, float z);
    void setRotationByAxis(float angle, float x, float y, float z);
    void rotate(float w, float x, float y, float z);
    void rotateByAxis(float angle, float x, float y, float z);
    void rotateByAxisWithPivot(float angle, float axis_x, float axis_y,
            float axis_z, float pivot_x, float pivot_y, float pivot_z);
    void rotateWithPivot(float w, float x, float y, float z, float pivot_x,
            float pivot_y, float pivot_z);
    void setModelMatrix(glm::mat4 mat);

    const glm::vec3 &position() const;
    float position_x() const;
    float position_y() const;
    float position_z() const;

    void set_position(const glm::vec3 &position);
    void set_position(float x, float y, float z);
    void set_position_x(float x);
    void set_position_y(float y);
    void set_position_z(float z);

    const glm::quat &rotation() const;
    float rotation_w() const;
    float rotation_x() const;
    float rotation_y() const;
    float rotation_z() const;
    float rotation_yaw() const;
    float rotation_pitch() const;
    float rotation_roll() const;

    void set_rotation(float w, float x, float y, float z);
    void set_rotation(const glm::quat &rotation);

    const glm::vec3 &scale() const;
    float scale_x() const;
    float scale_y() const;
    float scale_z() const;

    void set_scale(const glm::vec3 &scale);
    void set_scale(float x, float y, float z);
    void set_scale_x(float x);
    void set_scale_y(float y);
    void set_scale_z(float z);

    bool isModelMatrixValid();

private:
    Transform(const Transform& transform);
    Transform(Transform&& transform);
    Transform& operator=(const Transform& transform);
    Transform& operator=(Transform&& transform);

private:
    glm::vec3 position_;
    glm::quat rotation_;
    glm::vec3 scale_;

    Lazy<glm::mat4> model_matrix_;

    std::mutex mutex_;
    mutable std::mutex position_mutex_;
    mutable std::mutex rotation_mutex_;
    mutable std::mutex scale_mutex_;
};

}
#endif
