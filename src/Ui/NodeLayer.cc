#include "NodeLayer.h"

void NodeLayer::OnAttach()
{
    m_config = new INIReader("./configs/url.ini");
    m_master_url = m_config->Get("ROS", "URL_MASTER", "192.168.2.162:11311");

    m_AppNode = AppNode::GetInstance();
    m_AppNode->start();
}

void NodeLayer::OnUpdate(float ts)
{
    ShowNodeControl();
}

void NodeLayer::OnDetach()
{
}

void NodeLayer::OnUIRender()
{
}

void NodeLayer::ShowNodeControl()
{
    ImGui::Begin(u8"机器人");
    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
    ImGui::AlignTextToFramePadding();

    ImGui::SeparatorText(u8"连接");
    ImGui::Text(u8"\t\t机器人地址:\t%s", m_master_url.c_str());
    static int radio_master = 0;
    ImGui::RadioButton(u8"连接机器人", &radio_master, 1);
    ImGui::SameLine(0, 20);
    ImGui::RadioButton(u8"断开机器人", &radio_master, 0);
    ImGui::SameLine(0, 20);

    if (radio_master) {
        m_AppNode->init((char *)m_master_url.c_str());
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Start");
    } else {
        m_AppNode->destroy();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stop");
    }
    
    ImGui::PopItemWidth();
    ImGui::End();
}