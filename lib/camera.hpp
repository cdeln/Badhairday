#ifndef __CAMERA__
#define __CAMERA__

#include <glm/glm.hpp>


class Camera {
    public:

        enum ViewMode {
            FREE,
            PIVOT
        };

        enum ProjMode {
            PERSPECTIVE,
            FRUSTUM,
            ORTHO
        };

        Camera();
        void setPos(float x, float y, float z);
        void lookAt(float x, float y, float z);
        void setDrag(float drag);
        void setDist(float dist);
        void setRot(float pitch, float yaw);
        void setRotVel(float pitchVel, float yawVel);
        void setPerspective(float fov, float aspect, float near, float far);
        void setFrustum(float left, float right, float bottom, float top, float near, float far);
        void setOrtho(float left, float right, float bottom, float top, float near, float far);
        void setFov(float fov);
        void setAspect(float aspect);
        void setWidth(float width);
        void setHeight(float height);
        void setDepth(float zNear, float zFar);
        void setViewMode(ViewMode mode); 
        void setProjMode(ProjMode mode); 

        void moveLocal(float x, float y, float z);

        ViewMode viewMode();
        ProjMode projMode();

        glm::vec3 getPos();
        glm::mat4 getView();
        glm::mat4 getProj();
        glm::mat4 get();

        void update(float dt);

    private:
        //glm::mat4 translation_;
        glm::mat4 rotation_;
        glm::mat4 projection_;
        glm::mat4 view_;
        glm::mat4 matrix_;
        glm::vec3 pos_;
        glm::vec3 side_; 
        glm::vec3 up_; 
        glm::vec3 pivot_;

        ViewMode viewMode_;
        ProjMode projMode_;

        float width_;
        float height_;
        float zNear_;
        float zFar_;
        float fov_;
        float aspect_;
        
        float pitch_;
        float yaw_;
        float pitchVel_;
        float yawVel_;
        float drag_;

        void updateRotation();

        void updatePivotView();
        void updateFreeView();
        void updateView();

        void updatePerspective();
        void updateFrustum();
        void updateOrtho();
        void updateProjection();

        void updateMatrix();
};

#endif
