#include "ros/ros.h"
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
int main(int argc,char** argv)
{
  ros::init(argc,argv,"nav_goal");
  ros::NodeHandle nh;
  actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> ac("move_base",true);
  ac.waitForServer();
  move_base_msgs::MoveBaseGoal goal;
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();
  // 从参数服务器读取目标坐标，默认值为演示值
  nh.param<double>("target_x", goal.target_pose.pose.position.x, 2.534);
  nh.param<double>("target_y", goal.target_pose.pose.position.y, 0.111);
  nh.param<double>("target_oz", goal.target_pose.pose.orientation.z, -0.018);
  nh.param<double>("target_ow", goal.target_pose.pose.orientation.w, 0.997);
  ac.sendGoal(goal);
  // 带超时的等待，避免永久阻塞
  if (!ac.waitForResult(ros::Duration(60.0))) {
    ROS_WARN("Navigation timed out");
    ac.cancelGoal();
    return 1;
  }
  if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    ROS_INFO("机器人到达目标点");
  else
    ROS_WARN("Navigation failed: %s", ac.getState().toString().c_str());
  return 0;
}
