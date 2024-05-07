#include "LogLayer.h"
#include "../Ui/PlanManagerLayer.h"

void LogLayer::OnAttach()
{
    AppNode::GetInstance()->setOnPlanFeedback(std::bind(&LogLayer::OnPlanCallback, this, std::placeholders::_1));
}

void LogLayer::OnUpdate(float ts)
{
    ShowFeedbackLog();
}

void LogLayer::OnDetach()
{
}

void LogLayer::OnUIRender()
{
}

void LogLayer::OnPlanCallback(const std_msgs::String &str)
{
    int temp = 0;
    int current_goal = -1;
    
    int ret = sscanf(str.data, "Plan MOVE [%d] successed", &temp);
    if (ret)
        current_goal = temp - 1;

    ret = sscanf(str.data, "Plan TASK [%d] successed", &temp);
    if (ret)
        current_goal = temp - 1;

    ret = sscanf(str.data, "Plan STOP [%d] successed", &temp);
    if (ret)
        current_goal = temp - 1;

    PlanManagerLayer::SetCurrentGoal(current_goal);

    std::string plan_feedback = str.data;
    m_log.AddLog("%s\n", plan_feedback.c_str());
}

void LogLayer::ShowFeedbackLog()
{
    ImGui::Begin(u8"日志");
    m_log.Draw(u8"日志");
	ImGui::End();
}