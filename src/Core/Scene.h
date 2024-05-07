#ifndef SENCE_H
#define SENCE_H

#include "../Graphics/Objects/Model.h"
#include "../Graphics/Objects/Object.h"
#include "../Graphics/Renderer/Camera.h"
#include "../Graphics/Renderer/Shader.h"

class Scene
{
public:
    Scene(std::vector<ObjectStructure *> &objectStructures,
          std::vector<Model *> &objectModels);
    ~Scene();

    static Scene *GetInstance();

    void UpdateModel(Shader &shader, Camera &camera);
    void UpdateMap(Shader &shader, Camera &camera);
    void UpdateGird(Shader &shader, Camera &camera);
    void UpdatePlan(Shader &shader, Camera &camera);

    std::vector<ObjectStructure *> GetModel() const{ 
        return m_objectStructures; 
    }

    void setPlan(std::vector<Vertex> &goalList) { m_verticesPath = goalList; }

private:
    std::vector<ObjectStructure *> &m_objectStructures;
    std::vector<Model *> &m_objectModels;
    Model *m_model;

    std::vector<Texture> m_emptyTexture;

    std::vector<Vertex> m_verticesGird;
    std::vector<GLuint> m_indicesGird;
    Renderer *m_girdRender;
    void GenerateGrid(int rows, int cols, float spacing, glm::vec3 color);
    void DrawGrid(Shader &shader, Camera &camera);

    std::vector<Texture> m_mapTextureList;
    std::vector<Vertex> m_verticesMap;
    std::vector<GLuint> m_indicesMap;
    Renderer *m_mapRender;
    void loadPGMAndGenerateMap(const std::string &yamlname,
                               const std::string &pgmname, float spacing);
    void DrawMap(Shader &shader, Camera &camera);

    std::vector<Vertex> m_verticesPath;
    std::vector<GLuint> m_indicesPath;
    void DrawPath(Shader &shader, Camera &camera);

    float m_platfromSize = 1000.0f;
    int slices = m_platfromSize;
    glm::mat4 m_modelPlatform = glm::mat4(1.0f);

    uint8_t *m_pgmData;
};

#endif