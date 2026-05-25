#include<ros/ros.h>
#include<face_rec/recognition_results.h>
#include <ros/ros.h>
#include <robot_audio/robot_iat.h>
#include <robot_audio/Collect.h>
#include <robot_audio/robot_tts.h>
#include <actionlib/client/simple_action_client.h>
#include "move_base_msgs/MoveBaseAction.h"
#include <iostream>
#include <string>
using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> AC;//定义用到的action客户端的别名

class interaction{
    public:
        interaction();
        string voice_collect(); //语音采集
        string voice_dictation(const char* filename); //语音听写
        string voice_tts(const char* text); //语音合成
        void goto_nav(struct Point* point); //导航到目标位置
    private:
        ros::NodeHandle n; //创建一个节点句柄
        actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction>* ac; //创建action客户端对象指针
        ros::ServiceClient collect_client,dictation_client,tts_client; //创建客户端
};
interaction::interaction(){
    collect_client = n.serviceClient<robot_audio::Collect>("voice_collect"); //定义语音采集客户端
    dictation_client = n.serviceClient<robot_audio::robot_iat>("voice_iat"); //定义语音听写客户端
    tts_client = n.serviceClient<robot_audio::robot_tts>("voice_tts"); //定义语音合成客户端
}
string interaction::voice_collect(){
    //请求"voice_collect"服务，返回音频保存位置
    ros::service::waitForService("voice_collect");
    robot_audio::Collect srv;
    srv.request.collect_flag = 1;
    collect_client.call(srv);
    return srv.response.voice_filename;
}
string interaction::voice_dictation(const char* filename){
    //请求"voice_dictation"服务，返回听写出的文本
    ros::service::waitForService("voice_iat");
    robot_audio::robot_iat srv;
    srv.request.audiopath = filename;
    dictation_client.call(srv);
    return srv.response.text;
}
string interaction::voice_tts(const char* text){
    //请求"voice_tts"服务，返回合成的文件目录
    ros::service::waitForService("voice_tts");
    robot_audio::robot_tts srv;
    srv.request.text = text;
    tts_client.call(srv);
    string cmd= "play "+srv.response.audiopath;
    system(cmd.c_str());
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
  interaction audio; //创建一个交互实例
  string dir,text,path; //创建两个字符串变量
  while(ros::ok())
  {
     face_rec_data.request.mode = 1;
     face_rec_client.call(face_rec_data);
     if(face_rec_data.response.success)
        {
           int face_num = face_rec_data.response.result.num;
           cout<<"我看到了"<<face_num<<"个人脸,他们分别是："<<endl;
           for(int i=0;i<face_num;i++)
           {
            string rec_name = face_rec_data.response.result.face_data[i].name;
            cout<<rec_name<<" ";
           }
	   dir = audio.voice_collect(); //采集语音
           text = audio.voice_dictation(dir.c_str()).c_str(); //语音听写
           if(text.find("我是谁") != string::npos)
		{
			audio.voice_tts("好好好的，让我看看"); //合成应答语音
			if(face_rec_data.response.result.face_data[0].name=="龙正")
			{
				audio.voice_tts("龙正"); //合成应答语音
			}
			
		}
           cout<<endl;
        }
  }
  return true;
}

