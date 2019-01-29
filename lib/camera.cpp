#include "camera.hpp"
#include "macros.h"
#include "util.hpp"
#include <glm/gtx/transform.hpp>
#include <cmath>


Camera::Camera() {
    side_ = glm::vec3(1.0f, 0.0f, 0.0f);
    up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    pos_= glm::vec3(0.0f, 0.0f, 1.0f);
    pivot_= glm::vec3(0);
    drag_ = 1; 
    pitch_ = 0;
    yaw_ = 0;
    pitchVel_ = 0;
    yawVel_ = 0;
    fov_ = 45;
    aspect_ = 1;
    width_ = 1;
    height_ = 1;
    zNear_ = 0.01;
    zFar_ = 100;
    viewMode_ = PIVOT;
    projMode_ = PERSPECTIVE;
    updateRotation();
    updateProjection();
}

glm::vec3 Camera::getPos() {
    glm::vec3 posRotated = glm::vec3(rotation_ * glm::vec4(pos_, 1.0f)); 
    return posRotated;
    //return pos_;
}

glm::mat4 Camera::getView() {
    return view_;    
}

glm::mat4 Camera::getProj() {
    return projection_;
}

glm::mat4 Camera::get() {
    return matrix_;
}

/*
void Camera::setPos(float x, float y, float z) {
    translation_ = glm::translate(glm::vec3(x,y,z));
    updateView();
}
*/

void Camera::setPos(float x, float y, float z) {
    pos_ = glm::vec3(x,y,z);
    updateView();
}

void Camera::moveLocal(float x, float y, float z) {
    glm::vec4 localDir(x,y,z,1.0f);
    glm::vec4 globalDir = glm::inverse(rotation_) * localDir;
    pos_ += glm::vec3(globalDir);
    updateView();
}

void Camera::lookAt(float x, float y, float z) {
    pivot_ = glm::vec3(x,y,z);
    updateView();
}

void Camera::setDist(float dist) {
    pos_ = dist * glm::normalize(pos_); 
    updateView();
}

void Camera::setDrag(float drag) {
    drag_ = drag;
}

void Camera::updateRotation() {
    rotation_ = glm::rotate(yaw_, up_) * glm::rotate(pitch_, side_);
    updateView();
}

void Camera::setRot(float pitch, float yaw) {
    pitch_ = util::deg2rad(pitch);
    yaw_ = util::deg2rad(yaw);
    updateRotation();
}

void Camera::setRotVel(float pitchVel, float yawVel) {
    pitchVel_ = pitchVel;
    yawVel_ = yawVel;
}

void Camera::update(float dt) { 
    pitch_ += dt * pitchVel_;
    yaw_ += dt * yawVel_;
    pitch_ = util::clamp(pitch_, -45, 45);
    pitchVel_ *= std::exp( - dt * drag_ );
    yawVel_ *= std::exp( - dt * drag_ ); 
    setRot(pitch_, yaw_);
}

void Camera::setPerspective(float fov, float aspect, float near, float far) {
    setProjMode(PERSPECTIVE);
    fov_ = fov;
    aspect_ = aspect;
    zNear_ = near;
    zFar_ = far;
    updateProjection();
    //projection_ = glm::perspective(fov, aspect, near, far);
}

void Camera::setFov(float fov) {
    fov_ = fov;
    updateProjection();
}

void Camera::setAspect(float aspect) {
    aspect_ = aspect;
    updateProjection();
}

void Camera::setWidth(float width) {
    width_ = width;
    updateProjection();
}

void Camera::setHeight(float height) {
    height_ = height;
    updateProjection();
}

void Camera::setDepth(float zNear, float zFar) {
    zNear_ = zNear;
    zFar_ = zFar;
    updateProjection();
}

void Camera::setViewMode(ViewMode mode) {
    viewMode_ = mode;
    updateView();
}

Camera::ViewMode Camera::viewMode() {
    return viewMode_;
}

Camera::ProjMode Camera::projMode() {
    return projMode_;
}

void Camera::setProjMode(ProjMode mode) {
    projMode_ = mode;
    updateProjection();
}

void Camera::updateProjection() {
    switch( projMode_ ) {
        case PERSPECTIVE: updatePerspective(); break;
        case FRUSTUM: updateFrustum(); break;
        case ORTHO: updateOrtho(); break;
    }
    updateMatrix();
}

void Camera::updatePerspective() {
    projection_ = glm::perspective(fov_, aspect_, zNear_, zFar_);
}

void Camera::updateFrustum() {
    projection_ = glm::frustum(-width_/2, width_/2, -height_/2, height_/2, zNear_, zFar_);
}

void Camera::updateOrtho() {
    projection_ = glm::ortho(-width_/2, width_/2, -height_/2, height_/2, zNear_, zFar_);
}

void Camera::setFrustum(float left, float right, float bottom, float top, float near, float far) {
    projection_= glm::frustum(left, right, bottom, top, near, far);
    updateMatrix();
}
void Camera::setOrtho(float left, float right, float bottom, float top, float near, float far) {
    projection_ = glm::ortho(left, right, bottom, top, near, far);
    updateMatrix();
}

void Camera::updatePivotView() {
    glm::vec3 posRotated = glm::vec3(rotation_ * glm::vec4(pos_, 1.0f)); 
    view_= glm::lookAt(posRotated, pivot_, up_);
}

void Camera::updateFreeView() {
    view_ = glm::inverse(glm::translate(pos_) * rotation_);
}

void Camera::updateView() {
    switch( viewMode_ ) {
        case FREE: updateFreeView(); break;
        case PIVOT: updatePivotView(); break;
    }
    updateMatrix();
}

void Camera::updateMatrix() {
    matrix_ = projection_ * view_;
}
