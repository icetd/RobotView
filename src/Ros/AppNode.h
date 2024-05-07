#ifndef APP_NODE_H
#define APP_NODE_H

#include <ros.h>
#include <string>
#include <manager_msgs/Status.h>
#include <manager_msgs/Goal.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/MapMetaData.h>
#include <std_msgs/String.h>
#include <functional>

#include "../Core/MThread.h"

class AppNode : public MThread
{
public:
	AppNode();
	virtual ~AppNode();

	AppNode(AppNode& other) = delete;
	void operator = (const AppNode&) = delete;

	static AppNode* GetInstance();

	bool init(char* master_url);
	bool destroy();

	void move(char key, float speed_linear, float speed_angular);
	void pubPlan(manager_msgs::Goal& plan_goal);
	void pubCmdPlan(manager_msgs::Status& plan_cmd);

	void setClientStatus(bool is_client) { isClient = is_client; }
	bool getClientStatus() { return isClient; }

	std::function<void(const geometry_msgs::PoseStamped&)> OnTrackedPoseCallback;
	void setOnTrackedPoseCallback(std::function<void (const geometry_msgs::PoseStamped& pose)> callback);

	std::function<void(const std_msgs::String&)> OnPlanFeedback;
	void setOnPlanFeedback(std::function<void(const std_msgs::String& str)> callback);

protected:

	virtual void run() override;

private:
	static AppNode* instance;

	ros::NodeHandle* nh;
	std::string url;

	geometry_msgs::Twist twist_msg;
	ros::Publisher* m_cmd_vel_pub = nullptr;

	
	manager_msgs::Status cmd_plan_msg;
	ros::Publisher* m_cmd_plan_pub = nullptr;

	manager_msgs::Goal plan_msg;
	ros::Publisher* m_plan_pub = nullptr;

	ros::Subscriber<geometry_msgs::PoseStamped>* m_tracked_pose_sub = nullptr;
	ros::Subscriber<std_msgs::String>* m_back_plan_sub = nullptr;

	bool isClient;
	void FreeAll();
};


#endif