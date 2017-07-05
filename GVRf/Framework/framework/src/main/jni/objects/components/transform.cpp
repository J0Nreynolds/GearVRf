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

#include "transform.h"

#include "glm/gtc/type_ptr.hpp"

#include "objects/scene_object.h"

namespace gvr {

Transform::Transform() :
        Component(Transform::getComponentType()), position_(glm::vec3(0.0f, 0.0f, 0.0f)),
        rotation_(
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), scale_(
                glm::vec3(1.0f, 1.0f, 1.0f)), model_matrix_(
                Lazy<glm::mat4>(glm::mat4())) {
}

Transform::~Transform() {
}

void Transform::invalidate(bool rotationUpdated) {
    owner_object()->setTransformDirty();
    if (isModelMatrixValid()) {
        mutex_.lock();
        model_matrix_.invalidate();
        mutex_.unlock();
        std::vector<SceneObject*> childrenCopy = owner_object()->children();
        for (auto it = childrenCopy.begin(); it != childrenCopy.end(); ++it) {
            Transform* const t = (*it)->transform();
            if (nullptr != t) {
                t->invalidate(false);
            }
        }
    }

    if (rotationUpdated) {
        // scale rotation_ if needed to avoid overflow
        static const float threshold = sqrt(FLT_MAX) / 2.0f;
        static const float scale_factor = 0.5f / sqrt(FLT_MAX);
        rotation_mutex_.lock();
        if (rotation_.w > threshold || rotation_.x > threshold ||
            rotation_.y > threshold || rotation_.z > threshold)
        {
            rotation_.w *= scale_factor;
            rotation_.x *= scale_factor;
            rotation_.y *= scale_factor;
            rotation_.z *= scale_factor;
        }
        rotation_mutex_.unlock();
    }

    if(owner_object()) {
        owner_object()->dirtyHierarchicalBoundingVolume();
    }
}

glm::mat4 Transform::getModelMatrix(bool forceRecalculate) {
    if (!isModelMatrixValid() || forceRecalculate) {
        position_mutex_.lock();
        glm::mat4 translation_matrix = glm::translate(glm::mat4(), position_);
        position_mutex_.unlock();
        rotation_mutex_.lock();
        glm::mat4 rotation_matrix = glm::mat4_cast(rotation_);
        rotation_mutex_.unlock();
        scale_mutex_.lock();
        glm::mat4 scale_matrix = glm::scale(glm::mat4(), scale_);
        scale_mutex_.unlock();

        glm::mat4 trs_matrix = translation_matrix * rotation_matrix
                * scale_matrix;
        if (owner_object()->parent() != 0) {
            Transform *const t = owner_object()->parent()->transform();
            if (nullptr != t) {
                glm::mat4 model_matrix = t->getModelMatrix() * trs_matrix;
                mutex_.lock();
                model_matrix_.validate(model_matrix);
                mutex_.unlock();
            }
        } else {
            mutex_.lock();
            model_matrix_.validate(trs_matrix);
            mutex_.unlock();
        }
    }
    mutex_.lock();
    glm::mat4 elem = model_matrix_.element();
    mutex_.unlock();
    return elem;
}

glm::mat4 Transform::getLocalModelMatrix() {
    position_mutex_.lock();
    glm::mat4 translation_matrix = glm::translate(glm::mat4(), position_);
    position_mutex_.unlock();
    rotation_mutex_.lock();
    glm::mat4 rotation_matrix = glm::mat4_cast(rotation_);
    rotation_mutex_.unlock();
    scale_mutex_.lock();
    glm::mat4 scale_matrix = glm::scale(glm::mat4(), scale_);
    scale_mutex_.unlock();
    glm::mat4 trs_matrix = translation_matrix * rotation_matrix
            * scale_matrix;
    return trs_matrix;
}

void Transform::setModelMatrix(glm::mat4 matrix) {

	glm::vec3 new_position(matrix[3][0], matrix[3][1], matrix[3][2]);

    glm::vec3 Xaxis(matrix[0][0], matrix[0][1], matrix[0][2]);
    glm::vec3 Yaxis(matrix[1][0], matrix[1][1], matrix[1][2]);
    glm::vec3 Zaxis(matrix[2][0], matrix[2][1], matrix[2][2]);

    double zs = glm::dot(glm::cross(Xaxis, Yaxis), Zaxis);
    double ys = glm::dot(glm::cross(Zaxis, Xaxis), Yaxis);
    double xs = glm::dot(glm::cross(Yaxis, Zaxis), Xaxis);


    xs = std::signbit(xs);
    ys = std::signbit(ys);
    zs = std::signbit(zs);

    xs = (xs > 0.0 ? -1 : 1);
    ys = (ys > 0.0 ? -1 : 1);
    zs = (zs > 0.0 ? -1 : 1);

    glm::vec3 new_scale;
    new_scale.x = xs * glm::sqrt(
                    matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1]
                            + matrix[0][2] * matrix[0][2]);
    new_scale.y = ys * glm::sqrt(
                    matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1]
                            + matrix[1][2] * matrix[1][2]);
    new_scale.z = zs * glm::sqrt(
                    matrix[2][0] * matrix[2][0] + matrix[2][1] * matrix[2][1]
                            + matrix[2][2] * matrix[2][2]);


    glm::mat3 rotation_mat(matrix[0][0] / new_scale.x,
            matrix[0][1] / new_scale.y, matrix[0][2] / new_scale.z,
            matrix[1][0] / new_scale.x, matrix[1][1] / new_scale.y,
            matrix[1][2] / new_scale.z, matrix[2][0] / new_scale.x,
            matrix[2][1] / new_scale.y, matrix[2][2] / new_scale.z);

    position_mutex_.lock();
    position_ = new_position;
    position_mutex_.unlock();
    scale_mutex_.lock();
    scale_ = new_scale;
    scale_mutex_.unlock();
    rotation_mutex_.lock();
    rotation_ = glm::quat_cast(rotation_mat);
    rotation_mutex_.unlock();

    invalidate(true);
}

void Transform::translate(float x, float y, float z) {
    position_mutex_.lock();
    position_ += glm::vec3(x, y, z);
    position_mutex_.unlock();
    invalidate(false);
}

// angle in radians
void Transform::setRotationByAxis(float angle, float x, float y, float z) {
    rotation_mutex_.lock();
    rotation_ = glm::angleAxis(angle, glm::vec3(x, y, z));
    rotation_mutex_.unlock();
    invalidate(true);
}

void Transform::rotate(float w, float x, float y, float z) {
    rotation_mutex_.lock();
    rotation_ = glm::quat(w, x, y, z) * rotation_;
    rotation_mutex_.unlock();
    invalidate(true);
}

// angle in radians
void Transform::rotateByAxis(float angle, float x, float y, float z) {
    rotation_mutex_.lock();
    rotation_ = glm::angleAxis(angle, glm::vec3(x, y, z)) * rotation_;
    rotation_mutex_.unlock();
    invalidate(true);
}

// angle in radians
void Transform::rotateByAxisWithPivot(float angle, float axis_x, float axis_y,
                                      float axis_z, float pivot_x, float pivot_y,
                                      float pivot_z) {
    glm::quat axis_rotation = glm::angleAxis(angle,
            glm::vec3(axis_x, axis_y, axis_z));
    glm::vec3 pivot(pivot_x, pivot_y, pivot_z);
    rotation_mutex_.lock();
    rotation_ = axis_rotation * rotation_;
    rotation_mutex_.unlock();
    position_mutex_.lock();
    glm::vec3 relative_position = position_ - pivot;
    relative_position = glm::rotate(axis_rotation, relative_position);
    position_ = relative_position + pivot;
    position_mutex_.unlock();
    invalidate(true);
}

void Transform::rotateWithPivot(float w, float x, float y, float z,
        float pivot_x, float pivot_y, float pivot_z) {
    glm::quat rotation(w, x, y, z);
    glm::vec3 pivot(pivot_x, pivot_y, pivot_z);
    rotation_mutex_.lock();
    rotation_ = rotation * rotation_;
    rotation_mutex_.unlock();
    position_mutex_.lock();
    glm::vec3 relative_position = position_ - pivot;
    relative_position = glm::rotate(rotation, relative_position);
    position_ = relative_position + pivot;
    position_mutex_.unlock();
    invalidate(true);
}

const glm::vec3& Transform::position() const {
    return position_;
}

float Transform::position_x() const {
    std::lock_guard<std::mutex> lock(position_mutex_);
    return position_.x;
}

float Transform::position_y() const {
    std::lock_guard<std::mutex> lock(position_mutex_);
    return position_.y;
}

float Transform::position_z() const {
    std::lock_guard<std::mutex> lock(position_mutex_);
    return position_.z;
}

void Transform::set_position(const glm::vec3& position) {
    {
        std::lock_guard<std::mutex> lock(position_mutex_);
        position_ = position;
    }
    invalidate(false);
}

void Transform::set_position(float x, float y, float z) {
    {
        std::lock_guard<std::mutex> lock(position_mutex_);
        position_.x = x;
        position_.y = y;
        position_.z = z;
    }
    invalidate(false);
}

void Transform::set_position_x(float x) {
    {
        std::lock_guard<std::mutex> lock(position_mutex_);
        position_.x = x;
    }
    invalidate(false);
}

void Transform::set_position_y(float y) {
    {
        std::lock_guard<std::mutex> lock(position_mutex_);
        position_.y = y;
    }
    invalidate(false);
}

void Transform::set_position_z(float z) {
    {
        std::lock_guard<std::mutex> lock(position_mutex_);
        position_.z = z;
    }
    invalidate(false);
}

const glm::quat& Transform::rotation() const {
    return rotation_;
}

float Transform::rotation_w() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return rotation_.w;
}

float Transform::rotation_x() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return rotation_.x;
}

float Transform::rotation_y() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return rotation_.y;
}

float Transform::rotation_z() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return rotation_.z;
}

// in radians
float Transform::rotation_yaw() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return glm::yaw(rotation_);
}

// in radians
float Transform::rotation_pitch() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return glm::pitch(rotation_);
}

// in radians
float Transform::rotation_roll() const {
    std::lock_guard<std::mutex> lock(rotation_mutex_);
    return glm::roll(rotation_);
}

void Transform::set_rotation(float w, float x, float y, float z) {
    {
        std::lock_guard<std::mutex> lock(rotation_mutex_);
        rotation_.w = w;
        rotation_.x = x;
        rotation_.y = y;
        rotation_.z = z;
    }
    invalidate(true);
}

void Transform::set_rotation(const glm::quat& rotation) {
    {
        std::lock_guard<std::mutex> lock(rotation_mutex_);
        rotation_ = rotation;
    }
    invalidate(true);
}

const glm::vec3& Transform::scale() const {
    return scale_;
}

float Transform::scale_x() const {
    std::lock_guard<std::mutex> lock(scale_mutex_);
    return scale_.x;
}

float Transform::scale_y() const {
    std::lock_guard<std::mutex> lock(scale_mutex_);
    return scale_.y;
}

float Transform::scale_z() const {
    std::lock_guard<std::mutex> lock(scale_mutex_);
    return scale_.z;
}

void Transform::set_scale(const glm::vec3& scale) {
    {
        std::lock_guard<std::mutex> lock(scale_mutex_);
        scale_ = scale;
    }
    invalidate(false);
}

void Transform::set_scale(float x, float y, float z) {
    {
        std::lock_guard<std::mutex> lock(scale_mutex_);
        scale_.x = x;
        scale_.y = y;
        scale_.z = z;
    }
    invalidate(false);
}

void Transform::set_scale_x(float x) {
    {
        std::lock_guard<std::mutex> lock(scale_mutex_);
        scale_.x = x;
    }
    invalidate(false);
}

void Transform::set_scale_y(float y) {
    {
        std::lock_guard<std::mutex> lock(scale_mutex_);
        scale_.y = y;
    }
    invalidate(false);
}

void Transform::set_scale_z(float z) {
    {
        std::lock_guard<std::mutex> lock(scale_mutex_);
        scale_.z = z;
    }
    invalidate(false);
}

bool Transform::isModelMatrixValid() {
    std::lock_guard<std::mutex> lock(mutex_);
    return model_matrix_.isValid();
}

}
