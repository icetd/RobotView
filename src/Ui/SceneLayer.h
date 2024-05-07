#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../Core/Layer.h"
#include "../Core/Scene.h"
#include "../Graphics/Buffers/FrameBuffer.h"
#include "../Graphics/Objects/Model.h"
#include "../Graphics/Objects/Object.h"
#include "../Graphics/Renderer/Camera.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Graphics/Renderer/Shader.h"
#include "../Ros/AppNode.h"

class SceneLayer : public Layer {
protected:
    virtual void OnAttach() override;
    virtual void OnUpdate(float ts) override;
    virtual void OnDetach() override;
    virtual void OnUIRender() override;

private:
    std::vector<ObjectStructure *> m_model;
    std::vector<Model *> m_objectModels;

    glm::mat4 m_proj = glm::mat4(1.0f);
    glm::mat4 m_view = glm::mat4(1.0f);

    Camera *m_camera;
    FrameBuffer *m_frameBuffer;

    Shader *m_TextureLightShader;
    Shader *m_TextureShader;
    Shader *m_ColorLightShader;
    Shader *m_ColorShader;

    Scene *m_sence;

    float m_width;
    float m_height;
    int m_selected = 0;

    void ShowMapSence();
    void ShowModelPosition();
    void ShowModelLoad();
    void convertPath(char *path);

    geometry_msgs::PoseStamped m_pose;
    void OnTrackedPoseCallback(const geometry_msgs::PoseStamped& pose);
};

#endif