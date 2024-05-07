#include "Scene.h"

#include <stb_image.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

static Scene *Instance = nullptr;

Scene::Scene(std::vector<ObjectStructure *> &objectStructures,
             std::vector<Model *> &objectModels)
    : m_objectStructures(objectStructures), m_objectModels(objectModels)
{
    Instance = this;
    GenerateGrid(slices, slices, 1, glm::vec3(0.1f, 0.1f, 0.1f));
    m_girdRender = new Renderer(m_verticesGird, m_indicesGird, m_emptyTexture);

    loadPGMAndGenerateMap("res/map/map.yaml", "res/map/map.pgm", 0.05);
    m_mapRender = new Renderer(m_verticesMap, m_indicesMap, m_mapTextureList);
}
Scene::~Scene()
{
    delete m_girdRender;
    delete m_mapRender;
}

Scene *Scene::GetInstance() { return Instance; }

void Scene::UpdateModel(Shader &shader, Camera &camera)
{
    for (int i = 0; i < m_objectStructures.size(); i++)
    {
        if (!m_objectStructures[i]->modelDefined)
        {
            m_model = new Model(m_objectStructures[i]);
            m_objectModels.push_back(m_model);
            m_objectStructures[i]->modelDefined = true;
        }
    }

    for (int i = 0; i < m_objectModels.size(); i++)
    {
        shader.Bind();
        glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
        shader.SetUniformMat4f("model", m_objectStructures[i]->objModel);
        shader.SetUniformMat4f("projection", camera.GetProjMatrix());
        shader.SetUniformMat4f("view", camera.GetViewMatrix());
        shader.SetUniformVec3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
        shader.SetUniformVec3f("lightPos", lightPos);
        shader.SetUniformVec3f("camPos", camera.GetEye());
        m_objectModels[i]->Draw(shader, GL_FILL);
    }
}

void Scene::UpdateMap(Shader &shader, Camera &camera)
{
    DrawMap(shader, camera);
}

void Scene::UpdateGird(Shader &shader, Camera &camera)
{
    DrawGrid(shader, camera);
}

void Scene::UpdatePlan(Shader &shader, Camera &camera)
{
    if (m_verticesPath.size() < 2)
        return; // 如果轨迹点少于两个，无法形成轨迹，直接返回
    DrawPath(shader, camera);
}

void Scene::DrawGrid(Shader &shader, Camera &camera)
{
    shader.Bind();
    glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
    shader.SetUniformMat4f("model", m_modelPlatform);
    shader.SetUniformMat4f("projection", camera.GetProjMatrix());
    shader.SetUniformMat4f("view", camera.GetViewMatrix());
    m_girdRender->DrawLine(shader);
}

void Scene::DrawMap(Shader &shader, Camera &camera)
{
    shader.Bind();
    glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
    shader.SetUniformMat4f("model", m_modelPlatform);
    shader.SetUniformMat4f("projection", camera.GetProjMatrix());
    shader.SetUniformMat4f("view", camera.GetViewMatrix());
    shader.SetUniformVec3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetUniformVec3f("lightPos", lightPos);
    shader.SetUniformVec3f("camPos", camera.GetEye());
    m_mapRender->DrawTriangle(shader, GL_FILL);
}

// 渲染模型轨迹的函数
void Scene::DrawPath(Shader &shader, Camera &camera)
{
    shader.Bind();
    glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
    shader.SetUniformMat4f("model", m_modelPlatform);
    shader.SetUniformMat4f("projection", camera.GetProjMatrix());
    shader.SetUniformMat4f("view", camera.GetViewMatrix());
    shader.SetUniformVec3f("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetUniformVec3f("lightPos", lightPos);
    shader.SetUniformVec3f("camPos", camera.GetEye());

    Renderer renderpath(m_verticesPath, m_indicesPath, m_emptyTexture);
    renderpath.DrawLineStrip(shader, m_verticesPath.size());
}

void Scene::GenerateGrid(int rows, int cols, float spacing, glm::vec3 color)
{
    // Calculate the center position of the grid
    float centerX = -0.5f * cols * spacing;
    float centerY = -0.5f * rows * spacing;

    // Calculate the number of vertices and indices
    int numVertices = (rows + 1) * (cols + 1);
    int numIndices = rows * cols * 6;

    // Reserve space for vertices and indices
    m_verticesGird.reserve(numVertices);
    m_indicesGird.reserve(numIndices);

    // Generate vertex data
    for (int i = 0; i <= rows; ++i)
    {
        for (int j = 0; j <= cols; ++j)
        {
            float x = centerX + static_cast<float>(j) * spacing;
            float y = centerY + static_cast<float>(i) * spacing;
            float z = -0.05f; // You can adjust the height of the grid

            m_verticesGird.push_back(Vertex{glm::vec3(x, y, z), color,
                                            glm::vec3(0.0f), glm::vec2(0.0f)});
        }
    }

    // Generate index data
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            int topLeft = i * (cols + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (cols + 1) + j;
            int bottomRight = bottomLeft + 1;

            // Add indices for two triangles
            m_indicesGird.push_back(topLeft);
            m_indicesGird.push_back(topRight);
            m_indicesGird.push_back(bottomLeft);

            m_indicesGird.push_back(bottomLeft);
            m_indicesGird.push_back(topRight);
            m_indicesGird.push_back(bottomRight);
        }
    }
}

void Scene::loadPGMAndGenerateMap(const std::string &yamlname,
                                  const std::string &pgmname, float spacing)
{
    // load map info
    YAML::Node config = YAML::LoadFile(yamlname.c_str());

    float resolution;
    glm::vec3 origin(0.0f);

    if (config["resolution"])
    {
        resolution = config["resolution"].as<float>();
    }

    if (config["origin"])
    {
        std::vector<float> originValues =
            config["origin"].as<std::vector<float>>();
        if (originValues.size() == 3)
        {
            origin.x = originValues[0];
            origin.y = originValues[1];
            origin.z = originValues[2];
        }
    }

    int width, height, channels;
    unsigned char *image =
        stbi_load(pgmname.c_str(), &width, &height, &channels, 0);

    if (!image)
    {
        std::cerr << "Failed to load PGM file: " << pgmname << std::endl;
        return;
    }

    m_verticesMap.clear();
    m_verticesMap.push_back(Vertex{glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec3(1.0f),
                                   glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec2(0.0f, 0.0f),
                                   glm::vec3(0.0f),
                                   glm::vec3(0.0f),
                                   {0},
                                   {1.0f}});
    m_verticesMap.push_back(Vertex{glm::vec3(width * resolution, 0.0f, 0.0f),
                                   glm::vec3(1.0f),
                                   glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec2(1.0f, 0.0f),
                                   glm::vec3(0.0f),
                                   glm::vec3(0.0f),
                                   {0},
                                   {1.0f}});
    m_verticesMap.push_back(Vertex{glm::vec3(0.0f, height * resolution, 0.0f),
                                   glm::vec3(1.0f),
                                   glm::vec3(0.0f, 0.0f, 0.0f),
                                   glm::vec2(0.0f, 1.0f),
                                   glm::vec3(0.0f),
                                   glm::vec3(0.0f),
                                   {0},
                                   {1.0f}});
    m_verticesMap.push_back(
        Vertex{glm::vec3(width * resolution, height * resolution, 0.0f),
               glm::vec3(1.0f),
               glm::vec3(0.0f, 0.0f, 0.0f),
               glm::vec2(1.0f, 1.0f),
               glm::vec3(0.0f),
               glm::vec3(0.0f),
               {0},
               {1.0f}});

    for (Vertex &vertex : m_verticesMap)
    {
        vertex.position += origin;
    }

    m_indicesMap.clear();
    m_indicesMap = {0, 1, 2, 3, 2, 1};

    stbi_image_free(image);

    // Split path and filename
    size_t lastSlashPos = pgmname.find_last_of('/');
    std::string filepath = pgmname.substr(0, lastSlashPos);
    std::string filename = pgmname.substr(lastSlashPos + 1);

    m_mapTextureList.push_back(
        Texture(filepath, filename, "texture_diffuse", true));
}
