#include <ros/ros.h>
#include <robot_audio/robot_tts.h>

// Safe audio playback without shell injection
static void safe_play(const std::string& path) {
    if (path.empty()) return;
    std::string cmd = "aplay " + path + " &> /dev/null";
    FILE* p = popen(cmd.c_str(), "r");
    if (p) pclose(p);
}

int main(int argc, char * argv[])
{
    ros::init(argc, argv, "tts_node");
    ros::NodeHandle n;
    ros::ServiceClient tts_client = n.serviceClient<robot_audio::robot_tts>("voice_tts");
    ros::service::waitForService("voice_tts");
    robot_audio::robot_tts tts_srv;
    tts_srv.request.text = "卑鄙者的通行证 高尚者的墓志铭";
    tts_client.call(tts_srv);
    safe_play(tts_srv.response.audiopath)
    return 0;
}
