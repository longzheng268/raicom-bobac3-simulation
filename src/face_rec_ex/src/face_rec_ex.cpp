#include <ros/ros.h>
#include <face_rec/recognition_results.h>
#include <robot_audio/robot_iat.h>
#include <robot_audio/Collect.h>
#include <robot_audio/robot_tts.h>
#include <actionlib/client/simple_action_client.h>
#include "move_base_msgs/MoveBaseAction.h"
#include <iostream>
#include <string>

// Safe audio playback without shell injection
static void safe_play(const std::string& path) {
    if (path.empty()) return;
    std::string cmd = "aplay " + path + " &> /dev/null";
    FILE* p = popen(cmd.c_str(), "r");
    if (p) pclose(p);
}
using namespace std;

class interaction{
    public:
        interaction();
        string voice_collect();
        string voice_dictation(const char* filename);
        string voice_tts(const char* text);
    private:
        ros::NodeHandle n;
        ros::ServiceClient collect_client,dictation_client,tts_client;
};
interaction::interaction(){
    collect_client = n.serviceClient<robot_audio::Collect>("voice_collect");
    dictation_client = n.serviceClient<robot_audio::robot_iat>("voice_iat");
    tts_client = n.serviceClient<robot_audio::robot_tts>("voice_tts");
}
string interaction::voice_collect(){
    ros::service::waitForService("voice_collect");
    robot_audio::Collect srv;
    srv.request.collect_flag = 1;
    collect_client.call(srv);
    return srv.response.voice_filename;
}
string interaction::voice_dictation(const char* filename){
    ros::service::waitForService("voice_iat");
    robot_audio::robot_iat srv;
    srv.request.audiopath = filename;
    dictation_client.call(srv);
    return srv.response.text;
}
string interaction::voice_tts(const char* text){
    ros::service::waitForService("voice_tts");
    robot_audio::robot_tts srv;
    srv.request.text = text;
    tts_client.call(srv);
    safe_play(srv.response.audiopath);
    sleep(1);
    return srv.response.audiopath;
}

int main(int argc,char** argv)
{
    ros::init(argc,argv,"face_rec_ex");
    ros::NodeHandle n;
    ros::ServiceClient face_rec_client = n.serviceClient<face_rec::recognition_results>("face_recognition_results");
    face_rec::recognition_results face_rec_data;
    ros::service::waitForService("face_recognition_results");
    interaction audio;
    string dir, text;

    while(ros::ok())
    {
        face_rec_data.request.mode = 1;
        face_rec_client.call(face_rec_data);

        if(face_rec_data.response.success)
        {
            int face_num = face_rec_data.response.result.num;
            cout << "检测到 " << face_num << " 个人脸" << endl;

            if(face_num > 0)
            {
                // 语音播报所有人脸名称
                string name_list = "我看到了";
                for(int i = 0; i < face_num; i++)
                {
                    string name = face_rec_data.response.result.face_data[i].name;
                    cout << name << " ";
                    if(i > 0) name_list += "和";
                    name_list += name;
                }
                cout << endl;

                // 采集语音并听写
                dir = audio.voice_collect();
                text = audio.voice_dictation(dir.c_str());

                // 回答"我是谁"
                if(text.find("我是谁") != string::npos)
                {
                    audio.voice_tts("好好好的，让我看看");
                    for(int i = 0; i < face_num; i++)
                    {
                        string name = face_rec_data.response.result.face_data[i].name;
                        audio.voice_tts(name.c_str());
                    }
                }
            }
        }
    }
    return 0;
}
