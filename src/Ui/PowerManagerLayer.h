#ifndef POWER_MANAGER_LAYER_H
#define POWER_MANAGER_LAYER_H

#include "../Core/Layer.h"
#include "../Core/MTimer.h"
#include "../Commuication/TcpClient.h"
#include "../Utils/INIReader.h"

class PowerManagerLayer : public Layer
{
protected:
    virtual void OnAttach() override;
    virtual void OnUpdate(float ts) override;
    virtual void OnDetach() override;
    virtual void OnUIRender() override;

private:
	MTimer *m_timer;
	void TimerSendToPowerControl();

	typedef enum {
		UREF_BOARD = 0,
		IREF_CHG,
		IREF_RUN,
		IREF_POWR0,
		IREF_CTRL,
		IREF_OIL,
		IREF_XYZ
	} CurrentType_t;

	std::string m_powerControl_message;
	std::string m_powerControl_url;
	uint16_t m_powerControl_port;
	bool isPowerControlCliented;
	TcpClient *m_powerControl_client;
	float m_robot_voltage;
	float m_robot_power;
	float m_robot_current[7];
	float m_robot_total_current;

	bool m_power_run;
	bool m_power_ctrl;
	bool m_power_xyz;
	bool m_power_oil;
	bool m_power_led_far;
	bool m_power_led_near;
	bool m_power_led_oil;

	int16_t m_current_index;
	uint16_t m_voltage;

    void OnPowerControlMessage(std::string message);

    void ShowPowerControl();
    void ShowPowerStatus();

    INIReader *m_config;
};

#endif