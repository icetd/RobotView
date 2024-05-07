#include "PlanManagerLayer.h"
#include "../Ros/AppNode.h"

int PlanManagerLayer::m_current_goal = -1;

void PlanManagerLayer::OnAttach()
{
    m_PlanManager = std::make_unique<PlanManager>();
    m_PlanManager->pullPlans();
    m_current_goal = -1;
    std::string default_name = "default";
    m_make_plan = std::make_unique<NavPlan>(default_name);
    m_make_plan->Clear();
    m_make_goal_num = 1;
    m_make_plan_num = 1;
}
void PlanManagerLayer::OnUpdate(float ts)
{
    UpdatePlan();
    ShowNavPlan();
    ShowMakePlan();
}

void PlanManagerLayer::OnDetach() {}

void PlanManagerLayer::OnUIRender() {}
/**
 * @brief ShowNacPlan 导航界面，选择路线 
 * 
 */
void PlanManagerLayer::ShowNavPlan()
{
    ImGui::Begin(u8"导航");
    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
    ImGui::AlignTextToFramePadding();

    if (AppNode::GetInstance()->getClientStatus() || m_isMakePlanMode)
    {
        ImGui::SeparatorText(u8"线路选择");
        // 尝试将c_str() 返回的指针添加到 std::vector<const char*> 中，
        // 但这可能导致问题。c_str()
        // 返回的指针是指向字符串内部缓冲区的指针，而这个缓冲区是临时的，当
        // GetName() 的结果超出作用域时就会被销毁，从而导致 planNameList
        // 中的指针成为悬空指针。 将字符串复制到 std::vector<std::string>
        // 中，然后使用 std::vector<const char*> 作为 ImGui Combo
        // 的参数。这样可以确保字符串的生命周期足够长，避免悬空指针的问题。
        if (m_PlanManager->GetPlanNum() == 0) {
            ImGui::PopItemWidth();
            ImGui::End();
            return;
        }

        std::vector<std::string> planNameList;
        std::map<int, NavPlan> planList = m_PlanManager->GetPlanList();
        m_make_plan_num = planList.size() + 1;
        for (auto &plan : planList)
        {
            planNameList.push_back(plan.second.GetShowName());
        }
        std::vector<const char *> planNamesArray;
        for (const auto &str : planNameList)
        {
            planNamesArray.push_back(str.c_str());
        }
        ImGui::Combo(u8"线路选择", &m_current_plan, planNamesArray.data(),
                     static_cast<int>(planNamesArray.size()));

        if (m_current_plan >= m_make_plan_num - 1)
            return;

        std::vector<std::string> goalNameList;
        std::map<int, std::string> goalList = m_PlanManager->GetPlanList().find(m_current_plan + 1)->second.GetGoalNameList();
        for (auto &goalname : goalList)
        {
            goalNameList.push_back(goalname.second);
        }
        std::vector<const char *> goalNamesArray;
        for (const auto &str : goalNameList)
        {
            goalNamesArray.push_back(str.c_str());
        }

        static int item_current_idx = 0;
        if (ImGui::BeginListBox(u8"目标点"))
        {
            for (int n = 0; n < static_cast<int>(goalNamesArray.size()); n++)
            {
                if (n <= m_current_goal)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s",
                                       goalNamesArray[n]);
                }
                else
                {
                    ImGui::Text("%s", goalNamesArray[n]);
                }
            }
            ImGui::EndListBox();
        }
        ImGui::NewLine();

        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        const float buttonWidth = (contentSize.x - 20) / 2;

        if (ImGui::Button(u8"发布路线", ImVec2(buttonWidth, 30)) &&
            m_PlanManager->GetCurrentPlanId() > 0) {
            manager_msgs::Status status;
            status.status = status.DELETEALL;
            AppNode::GetInstance()->pubCmdPlan(status);
            m_current_goal = -1;

            for (auto &plan : m_PlanManager->GetPlanList().at(m_current_plan + 1).GetGoalList()) {
                AppNode::GetInstance()->pubPlan(plan.second);
            }
        }

        ImGui::SameLine(0, 10);
        if (ImGui::Button(u8"开始执行", ImVec2(buttonWidth, 30)) &&
            m_PlanManager->GetCurrentPlanId() > 0)
        {
            m_current_goal = -1;
            manager_msgs::Status status;
            status.status = status.START;
            AppNode::GetInstance()->pubCmdPlan(status);
        }

        if (ImGui::Button(u8"暂停导航", ImVec2(buttonWidth, 30)) &&
            m_PlanManager->GetCurrentPlanId() > 0)
        {
            manager_msgs::Status status;
            status.status = status.CANCEL;
            AppNode::GetInstance()->pubCmdPlan(status);
        }

        ImGui::SameLine(0, 10);
        if (ImGui::Button(u8"继续导航", ImVec2(buttonWidth, 30)) &&
            m_PlanManager->GetCurrentPlanId() > 0)
        {
            manager_msgs::Status status;
            status.status = status.CONTINUE;
            AppNode::GetInstance()->pubCmdPlan(status);
        }
    }
    ImGui::PopItemWidth();
    ImGui::End();
}
/**
 * @brief ShowMakePlan 显示制作路线界面 
 * 
 */
void PlanManagerLayer::ShowMakePlan()
{
    ImGui::Begin(u8"路线");
    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
    ImGui::AlignTextToFramePadding();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    float buttonWidth = (contentSize.x - 20);

    m_isMakePlanMode = false;
    static char text[128] = {}; // 设置输入框默认参数
    ImGui::Text(u8"制作路线请输入密码:");
    ImGui::SameLine(0, 20);
    ImGui::InputText(u8"输入密码", text, IM_ARRAYSIZE(text), ImGuiInputTextFlags_Password, 0, text);
    if (strcmp((char *)text, "123") == 0) {
        m_isMakePlanMode = true;
    }

    if (m_isMakePlanMode)
    {
        ImGui::NewLine();
        ImGui::SeparatorText(u8"制作路线");
 
        
        static char temp_name[128];
        sprintf(temp_name, "plan_%d", m_make_plan_num);
        ImGui::InputTextWithHint(u8"路线编号", "input plan name here",
                                 temp_name, IM_ARRAYSIZE(temp_name));

        static char temp_show_name[128];
        ImGui::InputTextWithHint(u8"路线名", "input plan name here",
                                 temp_show_name, IM_ARRAYSIZE(temp_show_name));

        const char *goal_type_items[] = {"STOP", "PAUSE", "MOVE", "ACTION"};
        static int goal_type = 2;
        ImGui::Combo(u8"目标点类型", &goal_type, goal_type_items,
                     IM_ARRAYSIZE(goal_type_items));

        const char *action_id_items[] = {"NULL", u8"OIL1", u8"OIL2", u8"OIL3",
                                         u8"OIL4", u8"OIL5", u8"OIL6"};
        static int action_id = 0;
        ImGui::Combo("ACTION_ID", &action_id, action_id_items,
                     IM_ARRAYSIZE(action_id_items));

        static char temp_goal_name[128] = {};
        ImGui::InputTextWithHint(u8"目标点名称", u8"input goal name here",
                                 temp_goal_name, IM_ARRAYSIZE(temp_goal_name));

        static char needle_capacity[128] = {};
        static char alignment_tank_id[128] = {};
        static char alignment_offset_x[128] = {};
        static char alignment_offset_y[128] = {};

        ImGui::InputTextWithHint(u8"针管容量", u8"input needle capacity (ml)",
                                 needle_capacity,
                                 IM_ARRAYSIZE(needle_capacity));
        ImGui::InputTextWithHint(u8"取油箱ID", u8"input alignment offset (mm)",
                                 alignment_tank_id,
                                 IM_ARRAYSIZE(alignment_tank_id));
        ImGui::InputTextWithHint(u8"对准偏移X", u8"input alignment offset (mm)",
                                 alignment_offset_x,
                                 IM_ARRAYSIZE(alignment_offset_x));
        ImGui::InputTextWithHint(u8"对准偏移Y", u8"input alignment offset (mm)",
                                 alignment_offset_y,
                                 IM_ARRAYSIZE(alignment_offset_y));

        std::string plan_name = temp_name;
        std::string plan_name_show = temp_show_name;
        std::string goal_name = temp_goal_name;
        
        buttonWidth = (contentSize.x - 20) / 4;
        if (ImGui::Button(u8"添加goal", ImVec2(buttonWidth, 30)))
        {
            manager_msgs::Goal goal;
            goal.pose.position.x = Scene::GetInstance()->GetModel()[0]->objTranslation.x;
            goal.pose.position.y = Scene::GetInstance()->GetModel()[0]->objTranslation.y;
            goal.pose.position.z = Scene::GetInstance()->GetModel()[0]->objTranslation.z;
            goal.pose.orientation.w = Scene::GetInstance()->GetModel()[0]->objQuaternions.w;
            goal.pose.orientation.z = Scene::GetInstance()->GetModel()[0]->objQuaternions.z;
            goal.type.status = goal_type;
            goal.id = m_make_goal_num;
            goal.action_id = action_id;
            goal.needle_capacity = atoi(needle_capacity);
            goal.alignment_tank_id = atoi(alignment_tank_id);
            goal.alignment_offset_x = atoi(alignment_offset_x);
            goal.alignment_offset_y = atoi(alignment_offset_y);

            m_make_plan->Addgoal(goal, goal_name);
            m_make_goal_num++;
        }
        ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "goal_%d", m_make_goal_num);

        ImGui::SameLine(0, 20);
        if (ImGui::Button(u8"删除goal", ImVec2(buttonWidth, 30)) && m_make_goal_num > 1) {
            m_make_goal_num--;
            m_make_plan->Deletegoal(m_make_goal_num);
        }
        ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "goal_%d", m_make_goal_num - 1);

        if (ImGui::Button(u8"添加Plan", ImVec2(buttonWidth, 30)) &&
            m_make_goal_num > 1)
        {
            m_make_plan->SetName(plan_name);
            m_make_plan->SetShowName(plan_name_show);
            m_PlanManager->AddPlan(*m_make_plan);
            m_make_plan->Clear();
            m_make_goal_num = 1;
            m_make_plan_num++;
        }
        ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "plan_%d", m_make_plan_num);

        ImGui::SameLine(0, 20);
        if (ImGui::Button(u8"删除Plan", ImVec2(buttonWidth, 30)) && m_make_plan_num > 1) {
            m_make_plan_num--;
            m_PlanManager->DeletePlan(m_make_plan_num);
            m_make_plan->Clear();
            m_make_goal_num = 1;
        }
        ImGui::SameLine(0, 20);
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "plan_%d",
                           m_make_plan_num - 1);

        buttonWidth = (contentSize.x - 40) / 2;
        ImGui::NewLine();
        if (ImGui::Button(u8"提交路线到数据库", ImVec2(buttonWidth, 30)) &&
            m_make_plan_num > 1)
        {
            m_PlanManager->pushPlans();
        }

        ImGui::SameLine(0, 20);
        if (ImGui::Button(u8"从数据库更新路线", ImVec2(buttonWidth, 30)))
        {
            int ret = 0;
            m_make_plan->Clear();
            m_make_plan_num = 1;
            m_make_goal_num = 1;
            m_PlanManager->ClearPlan();
            ret = m_PlanManager->pullPlans();
        }

        if (ImGui::Button(u8"清空本地路线", ImVec2(buttonWidth, 30)))
        {
            m_make_plan->Clear();
            m_make_plan_num = 1;
            m_make_goal_num = 1;
            m_PlanManager->ClearPlan();
        }

        ImGui::SameLine(0, 20);
        if (ImGui::Button(u8"清空数据库", ImVec2(buttonWidth, 30)))
        {
            m_PlanManager->pushClearAll();
        }
    }
    ImGui::PopItemWidth();
    ImGui::End();
}
/**
 * @brief UpdatePlan 从Plan manager 获取当前plan ,更新当前路线所有点的 vertex
 *        并发布给sence
 * 
 */
void PlanManagerLayer::UpdatePlan()
{
    if (m_current_plan >= m_make_plan_num - 1)
        return;

    m_current_plan_vertex.clear();

	for (auto& goal : m_PlanManager->GetPlanList().find(m_current_plan + 1)->second.GetGoalList()) {
        Vertex vertex;
        vertex.position.x = goal.second.pose.position.x;
		vertex.position.y = goal.second.pose.position.y;
        vertex.position.z = 0.01f;
        vertex.color.g = 0.1f * goal.first;
        m_current_plan_vertex.push_back(vertex);
	}

    Scene::GetInstance()->setPlan(m_current_plan_vertex);
}