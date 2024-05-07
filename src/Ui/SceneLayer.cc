#include "SceneLayer.h"
#include "../Core/Application.h"
#include "../Core/Log.h"
#include <windows.h>

void SceneLayer::OnAttach()
{
    glm::vec3 eye(0.0f, -0.1f, 100.0f);
    glm::vec3 lookat(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector(0.0f, 0.0f, 1.0f);

    float width = Application::GetInstance()->GetWidth();
    float height = Application::GetInstance()->GetHeight();

    m_frameBuffer = new FrameBuffer(width, height);

    m_TextureLightShader = new Shader("res/shaders/TextureLight.shader");
    m_TextureShader = new Shader("res/shaders/Texture.shader");

    m_ColorLightShader = new Shader("res/shaders/ColorLight.shader");
    m_ColorShader = new Shader("res/shaders/Color.shader");

    m_camera = new Camera(eye, lookat, upVector, width, height);
    Application::GetInstance()->InitCamera(m_camera, m_frameBuffer);

    m_sence = new Scene(m_model, m_objectModels);

    ObjectStructure *car = new ObjectStructure();
    car->path = "res/objects/car/car.obj";
    car->name = "car";
    m_model.push_back(car);

    AppNode::GetInstance()->setOnTrackedPoseCallback(std::bind(&SceneLayer::OnTrackedPoseCallback, this, std::placeholders::_1));
}

void SceneLayer::OnUpdate(float ts)
{
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    m_camera->UpdateProjMatrix();
    m_camera->UpdateViewMatrix();
    m_proj = m_camera->GetProjMatrix();
    m_view = m_camera->GetViewMatrix();

    m_frameBuffer->Bind();

    m_sence->UpdateModel(*m_TextureLightShader, *m_camera);
    m_sence->UpdateMap(*m_TextureLightShader, *m_camera);
    m_sence->UpdateGird(*m_ColorShader, *m_camera);
    m_sence->UpdatePlan(*m_ColorShader, *m_camera);

    m_frameBuffer->Unbind();

    ShowMapSence();
    ShowModelPosition();
    // ShowModelLoad();
    m_frameBuffer->Unbind();
}

void SceneLayer::OnDetach()
{
    delete m_camera;
    delete m_frameBuffer;
    delete m_TextureLightShader;
    delete m_TextureShader;
    delete m_ColorLightShader;
    delete m_ColorShader;
    delete m_sence;
}

void SceneLayer::OnUIRender() {}

void SceneLayer::ShowMapSence()
{
    ImGui::Begin("Scene");
    {
        ImGui::BeginChild("GameRender");

        float width = ImGui::GetContentRegionAvail().x;
        float height = ImGui::GetContentRegionAvail().y;

        m_width = width;
        m_height = height;
        ImGui::Image((ImTextureID)m_frameBuffer->GetFrameTexture(), ImGui::GetContentRegionAvail(),
                     ImVec2(0, 1), ImVec2(1, 0));
        if (m_model.size() > 0)
        {
            ImGuizmo::SetDrawlist();
            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            float windowPosX = ImGui::GetWindowPos().x;
            float windowPosY = ImGui::GetWindowPos().y;
            ImGuizmo::SetRect(windowPosX, windowPosY, windowWidth,
                              windowHeight);
            ImGuizmo::AllowAxisFlip(false);
            ImGuizmo::Manipulate(glm::value_ptr(m_camera->GetViewMatrix()),
                                 glm::value_ptr(m_camera->GetProjMatrix()),
                                 ImGuizmo::OPERATION::TRANSLATE,
                                 ImGuizmo::MODE::LOCAL,
                                 glm::value_ptr(m_model[m_selected]->objModel));

            ImGuizmo::Manipulate(glm::value_ptr(m_camera->GetViewMatrix()),
                                 glm::value_ptr(m_camera->GetProjMatrix()),
                                 ImGuizmo::OPERATION::ROTATE_Z,
                                 ImGuizmo::MODE::LOCAL,
                                 glm::value_ptr(m_model[m_selected]->objModel));

            if (ImGuizmo::IsUsing)
            {
                m_model[m_selected]->objTranslation = glm::vec3(m_model[m_selected]->objModel[3]);

                // get Quaternions
                glm::mat4 rotationMatrix = m_model[m_selected]->objModel;
                m_model[m_selected]->objQuaternions = glm::quat_cast(rotationMatrix);

                // get eulerAngles
                m_model[m_selected]->objeulerAngles = glm::eulerAngles(m_model[m_selected]->objQuaternions);
                m_model[m_selected]->objRotation.z = glm::degrees(glm::eulerAngles(m_model[m_selected]->objQuaternions)).z;
            }
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void SceneLayer::ShowModelPosition()
{
    ImGui::Begin(u8"位置");
    if (m_model.size() > 0)
    {
        ImGui::BeginTable("ModelInfoTable", 4);
        // rable head
        ImGui::TableSetupColumn(u8"信息");
        ImGui::TableSetupColumn(u8"X");
        ImGui::TableSetupColumn(u8"Y");
        ImGui::TableSetupColumn(u8"Z");
        ImGui::TableHeadersRow();
        // show translation
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(u8"坐标");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objTranslation.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objTranslation.y);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objTranslation.z);
        // show rotation
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text(u8"方向");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objRotation.x);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objRotation.y);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text(u8"%.2f", m_model[m_selected]->objRotation.z);
        ImGui::EndTable();

        glm::quat rotationQuat = glm::angleAxis(
            m_model[m_selected]->objRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // 获取内容区域的可用大小
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        const float buttonWidth = contentSize.x;

        ImGui::NewLine();
        if (ImGui::Button(u8"初始化", ImVec2(buttonWidth, 25)))
        {
            m_model[m_selected]->objModel = glm::mat4(1.0f);
        }
    }
    ImGui::End();
}

void SceneLayer::ShowModelLoad()
{
    ImGui::Begin(u8"模型加载");
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    const float buttonWidth = contentSize.x;
    if (ImGui::Button(u8"添加", ImVec2(buttonWidth, 25)))
    {
        ImGui::NewLine();
        OPENFILENAME ofn;
        char szFile[260];
        const char *filter = "All\0*.*\0Text\0*.TXT\0";
        HWND hwnd = NULL;
        HANDLE hf;
        // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileName(&ofn) == TRUE)
        {
            char *output = new char[strlen(szFile) + 1];
            strcpy(output, szFile);
            convertPath(output);

            ObjectStructure *obj = new ObjectStructure();
            obj->path = output;

            char *token = NULL;
            char *name = NULL;
            const char *delimeter = "\\";
            token = strtok(output, delimeter);
            while (token != NULL)
            {
                name = token;
                token = strtok(NULL, delimeter);
            }
            LOG(INFO, "TOKEN %s\n", name);

            std::string strName = name;
            size_t lastSlashPos = strName.find_last_of('/');
            std::string filename = strName.substr(lastSlashPos + 1);

            obj->name = filename;
            m_model.push_back(obj);
        }
    }

    ImGui::Separator();
    static int selected = 0;
    for (int n = 0; n < m_model.size(); n++)
    {
        char buf[256];
        sprintf_s(buf, m_model[n]->name.c_str(), n);
        if (ImGui::Selectable(buf, selected == n))
        {
            selected = n;
            LOG(INFO, "Selected Object %d", selected);
            m_selected = n;
        }
    }

    ImGui::End();
}

void SceneLayer::convertPath(char *path)
{
    while (*path != '\0')
    {
        if (*path == '\\')
        {
            *path = '/';
        }
        path++;
    }
}


void SceneLayer::OnTrackedPoseCallback(const geometry_msgs::PoseStamped& pose)
{
    m_model[m_selected]->objTranslation.x = pose.pose.position.x;
    m_model[m_selected]->objTranslation.y = pose.pose.position.y;
    m_model[m_selected]->objTranslation.z = pose.pose.position.z;

    m_model[m_selected]->objQuaternions.w = pose.pose.orientation.w;
    m_model[m_selected]->objQuaternions.x = pose.pose.orientation.x;
    m_model[m_selected]->objQuaternions.y = pose.pose.orientation.y;
    m_model[m_selected]->objQuaternions.z = pose.pose.orientation.z;
    
    // get eulerAngles
    m_model[m_selected]->objeulerAngles = glm::eulerAngles(m_model[m_selected]->objQuaternions);
    m_model[m_selected]->objRotation.z = glm::degrees(glm::eulerAngles(m_model[m_selected]->objQuaternions)).z;

    m_model[m_selected]->objModel = glm::translate(glm::mat4(1.0f), m_model[m_selected]->objTranslation);
    m_model[m_selected]->objModel = glm::rotate(m_model[m_selected]->objModel, glm::radians(m_model[m_selected]->objRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    m_model[m_selected]->objModel = glm::rotate(m_model[m_selected]->objModel, glm::radians(m_model[m_selected]->objRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    m_model[m_selected]->objModel = glm::rotate(m_model[m_selected]->objModel, glm::radians(m_model[m_selected]->objRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
}