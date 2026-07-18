#include <ros/ros.h>
#include <robot_audio/Collect.h>
#include <iostream>
#include <string>

// Safe audio playback without shell injection
static void safe_play(const std::string& path) {
    if (path.empty()) return;
    std::string cmd = "aplay " + path + " &> /dev/null";
    FILE* p = popen(cmd.c_str(), "r");
    if (p) pclose(p);
}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"collect"); //初始化节点
    ros::NodeHandle n; //节点句柄
    ros::ServiceClient collect_client = n.serviceClient<robot_audio::Collect>("voice_collect"); //定义客户端
    robot_audio::Collect srv; //定义一个消息
    srv.request.collect_flag = 1;
    ros::service::waitForService("voice_collect"); //等待服务开启
    collect_client.call(srv); //发送消息
    ROS_INFO("File saved in : %s",srv.response.voice_filename.c_str());
    safe_play(srv.response.voice_filename);
}
