#include "AppNode.h"
#include <windows.h>
#include <map>
#include <vector>
#include <iostream>

AppNode* AppNode::instance = nullptr;
AppNode* AppNode::GetInstance()
{
	if (instance == nullptr) {
		instance = new AppNode();
	}
	return instance;
}

void AppNode::FreeAll()
{
	if (m_tracked_pose_sub) {
		delete m_cmd_vel_pub;
		m_cmd_vel_pub = nullptr;
		delete m_tracked_pose_sub;
		m_tracked_pose_sub = nullptr;
		delete m_back_plan_sub;
		m_back_plan_sub = nullptr;
		delete m_cmd_plan_pub;
		m_cmd_plan_pub = nullptr;
		delete m_plan_pub;
		m_plan_pub = nullptr;
	}
	if (nh) {
		delete nh;
		nh = nullptr;
	}
}

AppNode::AppNode() :
	isClient(false)
{
}

AppNode::~AppNode()
{
}

void tracked_pose_callback(const geometry_msgs::PoseStamped& pose)
{
	AppNode* appNode = AppNode::GetInstance();
	appNode->OnTrackedPoseCallback(pose);
}

void plan_callback(const std_msgs::String& str)
{
	AppNode* appNode = AppNode::GetInstance();
	appNode->OnPlanFeedback(str);
}


void AppNode::setOnTrackedPoseCallback(std::function<void (const geometry_msgs::PoseStamped& pose)> callback)
{	
	OnTrackedPoseCallback = std::move(callback);
}

void AppNode::setOnPlanFeedback(std::function<void(const std_msgs::String& str)> callback)
{
	OnPlanFeedback = std::move(callback);
}

bool AppNode::init(char* master_url)
{
	bool ret;
	if (!isClient) {
		isClient = true;
		nh = new ros::NodeHandle();
		nh->initNode(master_url);
		nh->setSpinTimeout(10000);

		m_cmd_vel_pub = new ros::Publisher("cmd_vel", &twist_msg);
		nh->advertise(*m_cmd_vel_pub);

		m_tracked_pose_sub = new ros::Subscriber<geometry_msgs::PoseStamped>("/tracked_pose", &tracked_pose_callback);
		nh->subscribe(*m_tracked_pose_sub);

		m_cmd_plan_pub = new ros::Publisher("/manager/cmd_plan", &cmd_plan_msg);
		nh->advertise(*m_cmd_plan_pub);

		m_plan_pub = new ros::Publisher("/manager/plan", &plan_msg);
		nh->advertise(*m_plan_pub);

		m_back_plan_sub = new ros::Subscriber<std_msgs::String>("/manager/feedback", &plan_callback);
		nh->subscribe(*m_back_plan_sub);
	}
	return true;
}

bool AppNode::destroy()
{
	if (isClient) {
		isClient = false;
		FreeAll();
	}

	return true;
}

void AppNode::move(char key, float speed_linear, float speed_angular)
{
	if (!isClient)
		return;
	std::map<char, std::vector<float>> moveBases{
		{'Q', { 1, 0, 0, 1}}, {'W', { 1, 0, 0, 0}}, {'E', { 1, 0, 0,-1}},
		{'A', { 0, 0, 0, 1}}, {'S', { 0, 0, 0, 0}}, {'D', { 0, 0, 0,-1}},
		{'Z', {-1, 0, 0,-1}}, {'X', {-1, 0, 0, 0}}, {'C', {-1, 0, 0, 1}},
	};

	float direction_x = moveBases[key][0];
	float direction_y = moveBases[key][1];
	float direction_z = moveBases[key][2];
	float direction_yaw = moveBases[key][3];

	twist_msg.linear.x = direction_x * speed_linear;
	twist_msg.linear.y = direction_y * speed_linear;
	twist_msg.linear.z = direction_z * speed_linear;
	twist_msg.angular.x = 0.0;
	twist_msg.angular.y = 0.0;
	twist_msg.angular.z = direction_yaw * speed_angular;

	m_cmd_vel_pub->publish(&twist_msg);
	nh->spinOnce();
}

void AppNode::pubPlan(manager_msgs::Goal& plan_goal)
{
	plan_msg = plan_goal;
	m_plan_pub->publish(&plan_msg);

	Sleep(200);
	nh->spinOnce();
}

void AppNode::pubCmdPlan(manager_msgs::Status& plan_cmd)
{
	cmd_plan_msg = plan_cmd;
	m_cmd_plan_pub->publish(&cmd_plan_msg);

	Sleep(200);
	nh->spinOnce();
}

void AppNode::run()
{
	int ret = 0;
	while (!isStoped())
	{
		if (isClient && nh) {
			ret = nh->spinOnce();
			Sleep(100);
		}
		else {
			Sleep(500);
		}
	}
}
