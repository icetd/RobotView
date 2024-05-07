#include "PowerManagerLayer.h"
#include "Utils/VoltageCalculate.h"
#include "Utils/Utils.h"

void PowerManagerLayer::OnAttach() 
{
	isPowerControlCliented = false;
	m_power_run = false;
	m_power_ctrl = false;
	m_power_xyz = false;
	m_power_oil = false;
	m_power_led_far = false;
	m_power_led_near = false;
	m_power_led_oil = false;
	m_robot_total_current = 0;

	m_config = new INIReader("./configs/url.ini");
	m_powerControl_url = m_config->Get("ROSPOWERCONTROL", "URL", "192.168.2.200");
	m_powerControl_port = m_config->GetInteger("ROSPOWERCONTROL", "PORT", 23);
}

void PowerManagerLayer::OnUpdate(float ts) 
{
    ShowPowerControl();
    ShowPowerStatus();
}

void PowerManagerLayer::OnDetach() 
{

}

void PowerManagerLayer::OnUIRender() 
{

}

void PowerManagerLayer::TimerSendToPowerControl()
{
	static int channel = 0;
	
	char tempbuf[8];
	if (channel > 6) {
		channel = 0;
	}

	sprintf(tempbuf, "g%d", channel);
	channel ++;

    m_powerControl_client->SendData(tempbuf, 8);
}

void PowerManagerLayer::OnPowerControlMessage(std::string message)
{
	sscanf(message.c_str(), "U%hd=%hd", &m_current_index, &m_voltage);
	static float temp;
	switch (m_current_index) {
	case CurrentType_t::UREF_BOARD:
		temp = (float)m_voltage / 4096.0f * 3.0 * 11.8;
		m_robot_voltage = calcaulateVoltage(temp);
		m_robot_power = calculatePower(m_robot_voltage);
		break;
	case CurrentType_t::IREF_CHG:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.4 * 1.04; // 1.04 is test error
		break;
	case CurrentType_t::IREF_RUN:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.2 * 1.23;  // 1.23 is test error
		break;
	case CurrentType_t::IREF_POWR0:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.84 * 1.03; // 1.03 is test error
		break;
	case CurrentType_t::IREF_CTRL:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.84 * 1.03;
		break;
	case CurrentType_t::IREF_OIL:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.51 * 1.03;
		break;
	case CurrentType_t::IREF_XYZ:
		m_robot_current[m_current_index] = (float)m_voltage / 4096.0f * 3.0 / 0.51 * 1.03;
		break;
	}

	m_robot_total_current = m_robot_current[2] + 
							m_robot_current[3] + 
							m_robot_current[4] + 
							m_robot_current[5] + 
							m_robot_current[6];
}

void PowerManagerLayer::ShowPowerControl() 
{
    ImGui::Begin(u8"电源控制");
    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
    ImGui::AlignTextToFramePadding();

	ImGui::SeparatorText(u8"电源控制");
	ImGui::Text(u8"\t\t电源控制地址:\t%s:%d", m_powerControl_url.c_str(), m_powerControl_port);
	static int radio_power;
	ImGui::RadioButton(u8"连接电源控制", &radio_power, 1); ImGui::SameLine(0, 20);
	ImGui::RadioButton(u8"断开电源控制", &radio_power, 0); ImGui::SameLine(0, 20);

	if (radio_power) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Start");
		if (!isPowerControlCliented) {
			m_powerControl_client = new TcpClient(m_powerControl_url, m_powerControl_port);
			if (m_powerControl_client->Init() != 0)
				exit(1);
			m_powerControl_client->SetMessageCallback(std::bind(&PowerManagerLayer::OnPowerControlMessage, this, std::placeholders::_1));
			m_powerControl_client->Start();

			m_timer = new MTimer(166);
			m_timer->SetTimerCallback(std::bind(&PowerManagerLayer::TimerSendToPowerControl, this));
			m_timer->Start();

			isPowerControlCliented = true;
		}
	} else {
		if (isPowerControlCliented) {
			m_timer->stop();
			m_powerControl_client->stop();
		}
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Stop");
		isPowerControlCliented = false;
	}

    if (!isPowerControlCliented) {
        ImGui::PopItemWidth();
        ImGui::End();
        return;
    }

    ImGui::NewLine();
	ImGui::SeparatorText(u8"电源开关");
	static bool btn_power_run;
	ImGui::Text(u8"行走电源"); ImGui::SameLine(0, 20);
	MToggleButton(u8"行走电源_", &btn_power_run);
	if (btn_power_run && !m_power_run) {
		m_powerControl_client->SendData("s1", 2);
		m_power_run = true;
	} else if (!btn_power_run && m_power_run){
		m_powerControl_client->SendData("r1", 2);
		m_power_run = false;
	}

	ImGui::SameLine(0, 40);
	static bool btn_power_ctrl;
	ImGui::Text(u8"控制电源"); ImGui::SameLine(0, 20);
	MToggleButton(u8"控制电源_", &btn_power_ctrl);
	if (btn_power_ctrl && !m_power_ctrl) {
		m_powerControl_client->SendData("s2", 2);
		m_power_ctrl = true;
	} else if (!btn_power_ctrl && m_power_ctrl){
		m_powerControl_client->SendData("r2", 2);
		m_power_ctrl = false;
	}

	static bool btn_power_xyz;
	ImGui::Text(u8"对接电源"); ImGui::SameLine(0, 20);
	MToggleButton(u8"对接电源_", &btn_power_xyz);
	if (btn_power_xyz && !m_power_xyz) {
		m_powerControl_client->SendData("s3", 2);
		m_power_xyz = true;
	} else if (!btn_power_xyz && m_power_xyz){
		m_powerControl_client->SendData("r3", 2);
		m_power_xyz = false;
	}
	
	ImGui::SameLine(0, 40);
	static bool btn_power_oil;
	ImGui::Text(u8"取油电源"); ImGui::SameLine(0, 20);
	MToggleButton(u8"取油电源_", &btn_power_oil);
	if (btn_power_oil && !m_power_oil) {
		m_powerControl_client->SendData("s4", 2);
		m_power_oil = true;
	} else if (!btn_power_oil && m_power_oil){
		m_powerControl_client->SendData("r4", 2);
		m_power_oil = false;
	}

	ImGui::NewLine();
	ImGui::SeparatorText(u8"灯光开关");
	static bool btn_power_led_near;
	ImGui::Text(u8"车近光灯"); ImGui::SameLine(0, 20);
	MToggleButton(u8"车近光灯_", &btn_power_led_near);
	if (btn_power_led_near && !m_power_led_near) {
		m_powerControl_client->SendData("o7", 2);
		m_power_led_near = true;
	} else if (!btn_power_led_near && m_power_led_near){
		m_powerControl_client->SendData("c7", 2);
		m_power_led_near = false;
	}

	ImGui::SameLine(0, 40);
	static bool btn_power_led_far;
	ImGui::Text(u8"车远光灯"); ImGui::SameLine(0, 20);
	MToggleButton(u8"车远光灯_", &btn_power_led_far);
	if (btn_power_led_far && !m_power_led_far) {
		m_powerControl_client->SendData("o8", 2);
		m_power_led_far = true;
	} else if (!btn_power_led_far && m_power_led_far){
		m_powerControl_client->SendData("c8", 2);
		m_power_led_far = false;
	}

	static bool btn_power_led_oil;
	ImGui::Text(u8"取油室灯"); ImGui::SameLine(0, 20);
	MToggleButton(u8"取油室灯_", &btn_power_led_oil);
	if (btn_power_led_oil && !m_power_led_oil) {
		m_powerControl_client->SendData("o5", 2);
		m_power_led_oil = true;
	} else if (!btn_power_led_oil && m_power_led_oil){
		m_powerControl_client->SendData("c5", 2);
		m_power_led_oil = false;
	}

    ImGui::PopItemWidth();
    ImGui::End();
}

void PowerManagerLayer::ShowPowerStatus()
{
    ImGui::Begin(u8"电源状态");
    ImGui::PushItemWidth(-ImGui::GetWindowWidth() * 0.2f);
    ImGui::AlignTextToFramePadding();
 
    ImGui::SeparatorText(u8"电源状态");
    static ScrollingBuffer dataV;
    static ScrollingBuffer dataC1, dataC2, dataC3, dataC4, dataC5, dataC6, dataCAll;
	
	static float t = 0;
    t += ImGui::GetIO().DeltaTime;
	dataV.AddPoint(t, m_robot_power);
    dataC1.AddPoint(t, m_robot_current[1]);
    dataC2.AddPoint(t, m_robot_current[2]);
    dataC3.AddPoint(t, m_robot_current[3]);
    dataC4.AddPoint(t, m_robot_current[4]);
    dataC5.AddPoint(t, m_robot_current[5]);
    dataC6.AddPoint(t, m_robot_current[6]);
    dataCAll.AddPoint(t, m_robot_total_current);
    
	static float history = 10.0f;

    if (ImPlot::BeginPlot("##ScrollVoltage", ImVec2(-1, 150) )) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels , ImPlotAxisFlags_LockMax  | ImPlotAxisFlags_LockMin);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 100);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
		ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.5f, 0.1f, 0.1f, 0.9f));
		ImPlot::PlotShaded(u8"电量(%)", &dataV.Data[0].x, &dataV.Data[0].y, dataV.Data.size(), -INFINITY, 0, dataV.Offset, 2 * sizeof(float));
		ImPlot::PopStyleColor();
        ImPlot::EndPlot();
    }

    if (ImPlot::BeginPlot("##ScrollCurrent", ImVec2(-1, 250) )) {
        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, 10);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
			
		ImPlot::PlotLine(u8"充电电流(A)", &dataC1.Data[0].x, &dataC1.Data[0].y, dataC1.Data.size(), 0, dataC1.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"行走电流(A)", &dataC2.Data[0].x, &dataC2.Data[0].y, dataC2.Data.size(), 0, dataC2.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"待机电流(A)", &dataC3.Data[0].x, &dataC3.Data[0].y, dataC3.Data.size(), 0, dataC3.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"控制电流(A)", &dataC4.Data[0].x, &dataC4.Data[0].y, dataC4.Data.size(), 0, dataC4.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"取油电流(A)", &dataC5.Data[0].x, &dataC5.Data[0].y, dataC5.Data.size(), 0, dataC5.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"对接电流(A)", &dataC6.Data[0].x, &dataC6.Data[0].y, dataC6.Data.size(), 0, dataC6.Offset, 2 * sizeof(float));
        ImPlot::PlotLine(u8"总电流(A)",   &dataCAll.Data[0].x, &dataCAll.Data[0].y, dataCAll.Data.size(), 0, dataCAll.Offset, 2 * sizeof(float));

		ImPlot::EndPlot();
    }

    ImGui::SliderFloat(u8"历史时间", &history, 1, 30, "%.1f s");

    ImGui::PopItemWidth();
    ImGui::End();
}