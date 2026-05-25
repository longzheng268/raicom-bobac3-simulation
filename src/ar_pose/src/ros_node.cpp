#include "PoseAdjust.h"

int main(int argc, char * argv[])
{
  ros::init(argc, argv, "ar_pose");
  rei_ar_pose::ArPoseAdjust arPoseAdjust;
  ros::NodeHandle nh;
  arPoseAdjust.Init(nh, true);
  ros::MultiThreadedSpinner s(2);
  s.spin();
  ros::shutdown();
  return 0;
}
