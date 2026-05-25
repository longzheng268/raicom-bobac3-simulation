#include <ros/ros.h>
#include <robot_audio/robot_iat.h>
#include <robot_audio/Collect.h>
#include <robot_audio/robot_tts.h>
#include <actionlib/client/simple_action_client.h>
#include "move_base_msgs/MoveBaseAction.h"
#include <iostream>
#include <string>
using namespace std;
struct Point {
    float x;  // x坐标
    float y;  // y坐标
    float z;  // 姿态z
    float w;  // 姿态w
    string name; // 地点名字
    string present; // 介绍语
    float px;  // 前点x坐标
    float py;  // 前点y坐标
    float pz;  // 前点姿态z
    float pw;  // 前点姿态w
};
struct Point m_point[7] = {
    {1.039, 1.141, 1.000, 0.001, "深圳", "深圳，中国的科创中心", 0.594, 1.163, -0.000, 1.000},
    {1.043, 2.115, -0.007, 1.000, "上海", "上海，中国的经济中心", 0.491, 1.972, 0.113,0.994},
    {2.538, 0.135, 1.000, 0.001, "北京", "北京，中国的首都", 2.006, 0.133, 0.000, 1.000},
    {2.503, 1.117, 0.107, 0.994, "广州", "广州，自古以来都是中国的商都", 2.064, 1.130, -0.001, 1.000},
    {2.520, 2.120, -0.009, 1.000, "吉林", "吉林，位于中国的东北是人参之都", 2.030, 2.012, 0.000, 1.000},
    {0.492,0.042,0.125,0.992, "返回", "返回原位置",0.408, 0.048, 0.003, 1.000}
};
struct Point n_point[7] = {
    { 0.740,1.177,0.999,0.035},
    { 0.578,1.979,0.881,-0.473},
    { 2.258,1.989,0.891,-0.455},
    { 2.315,1.118,1.000,-0.016},
    { 2.128,1.989,0.891,-0.455},
    {0.492,0.042,0.125,0.992}
};
        typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> AC; // 定义用到的action客户端的别名
        class interaction {
            public:
                interaction();
                string voice_collect(); // 语音采集
                string voice_dictation(const char* filename); // 语音听写
                string voice_tts(const char* text); // 语音合成
                void goto_nav(struct Point* point); // 导航到目标位置

            private:
                ros::NodeHandle n; // 创建一个节点句柄
                actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>* ac; // 创建action客户端对象指针
                ros::ServiceClient collect_client, dictation_client, tts_client; // 创建客户端
        };

                            interaction::interaction() {
                                collect_client = n.serviceClient<robot_audio::Collect>("voice_collect"); // 定义语音采集客户端
                                dictation_client = n.serviceClient<robot_audio::robot_iat>("voice_iat"); // 定义语音听写客户端
                                tts_client = n.serviceClient<robot_audio::robot_tts>("voice_tts"); // 定义语音合成客户端
                            }

                                                    string interaction::voice_collect() {
                                                        // 请求"voice_collect"服务，返回音频保存位置
                                                        ros::service::waitForService("voice_collect");
                                                        robot_audio::Collect srv;
                                                        srv.request.collect_flag = 1;
                                                        collect_client.call(srv);
                                                        return srv.response.voice_filename;
                                                    }

                                                    string interaction::voice_dictation(const char* filename) {
                                                        // 请求"voice_dictation"服务，返回听写出的文本
                                                        ros::service::waitForService("voice_iat");
                                                        robot_audio::robot_iat srv;
                                                        srv.request.audiopath = filename;
                                                        dictation_client.call(srv);
                                                        return srv.response.text;
                                                    }
                                                    string interaction::voice_tts(const char* text) {
                                                        ros::service::waitForService("voice_tts");
                                                        robot_audio::robot_tts srv;
                                                        srv.request.text = text;
                                                        tts_client.call(srv);
                                                        string cmd = "play " + srv.response.audiopath;
                                                        system(cmd.c_str());
                                                        sleep(1);
                                                        return srv.response.audiopath;
                                                    }
                                                                        void interaction::goto_nav(struct Point* point) { // 导航到目标
                                                                            ac = new AC("move_base", true);
                                                                            ROS_INFO("Waiting for action server to start.");
                                                                            ac->waitForServer(); // 一直等待move_base Action服务开启
                                                                            ROS_INFO("Action server started, sending goal.");
                                                                            move_base_msgs::MoveBaseGoal goal;
                                                                            goal.target_pose.header.frame_id = "map";
                                                                            goal.target_pose.header.stamp = ros::Time::now(); // 设置时间戳
                                                                            goal.target_pose.pose.position.x = point->px;
                                                                            goal.target_pose.pose.position.y = point->py;
                                                                            goal.target_pose.pose.orientation.z = point->pz;
                                                                            goal.target_pose.pose.orientation.w = point->pw;
                                                                            ac->sendGoal(goal); // 发送前点导航目标
                                                                            ac->waitForResult(); // 等待导航结果
                                                                                if (ac->getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
                                                                                        ROS_INFO("Reached pre-point!");
                                                                                        goal.target_pose.header.stamp = ros::Time::now(); // 更新时间戳
                                                                                        goal.target_pose.pose.position.x = point->x;
                                                                                        goal.target_pose.pose.position.y = point->y;
                                                                                        goal.target_pose.pose.orientation.z = point->z;
                                                                                        goal.target_pose.pose.orientation.w = point->w;
                                                                                        ac->sendGoal(goal); // 发送目标导航目标
                                                                                        ac->waitForResult(); // 等待导航结果
                                                                            if (ac->getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
                                                                                ROS_INFO("Goal succeeded!");
                                                     }
                                                                                            int main(int argc, char **argv) {
                                                                                            ros::init(argc, argv, "interaction");
                                                                                            interaction audio; // 创建一个交互实例
                                                                                            string dir, text, path; // 创建两个字符串变量
                                                                                                while (ros::ok()) {
                                                                                                      dir = audio.voice_collect(); // 采集语音
                                                                                                       text = audio.voice_dictation(dir.c_str()).c_str(); // 语音听写
                                                                                                    if(text.find("元宝元宝") != string::npos){
                                                                                                             audio.voice_tts("唉，什么事呀？");
                                                                                                             dir = audio.voice_collect(); // 采集语音
                                                                                                             text = audio.voice_dictation(dir.c_str()).c_str(); // 语音听写
                                                                                                    if (text.find("参观") != string::npos) {
                                                                                                             audio.voice_tts("好的，这就带您参观"); // 合成应答语音
                                                                                                        // }深圳 0 上海 1 北京2 广州3  吉林4  
                                                                                                            audio.goto_nav(&m_point[1]); // 导航到匹配的导航点
                                                                                                            audio.voice_tts(m_point[1].present.c_str()); // 介绍导航语
                                                                                                            audio.goto_nav(&n_point[1]); 
                                                                                                            audio.goto_nav(&m_point[0]); // 导航到匹配的导航点
                                                                                                            audio.voice_tts(m_point[0].present.c_str()); // 介绍导航语
                                                                                                            audio.goto_nav(&n_point[0]); 
                                                                                                            audio.goto_nav(&m_point[4]); // 导航到匹配的导航点
                                                                                                            audio.voice_tts(m_point[4].present.c_str()); // 介绍导航语
                                                                                                            audio.goto_nav(&n_point[4]); 
                                                                                                            audio.goto_nav(&m_point[3]); // 导航到匹配的导航点
                                                                                                            audio.voice_tts(m_point[3].present.c_str()); // 介绍导航语
                                                                                                            audio.goto_nav(&n_point[3]); 
                                                                                                            audio.goto_nav(&m_point[2]); // 导航到匹配的导航点
                                                                                                            audio.voice_tts(m_point[2].present.c_str()); // 介绍导航语
                                                                                                            audio.goto_nav(&n_point[2]);
                                                                                                           
                                                                                                            audio.goto_nav(&m_point[5]); //qiandian
                                                                                                            
                                                                                                    }

                                                                                             }
                                                                                          }
                                                                                 return 0;
                                                                      }