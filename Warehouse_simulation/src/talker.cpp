#include "ros/ros.h"
#include "std_msgs/String.h"
#include "nav_msgs/Odometry.h"
#include <sstream>

void odom_callback(const nav_msgs::Odometry::ConstPtr& msg)
{
  double x=msg->pose.pose.position.x;
  double y=msg->pose.pose.position.y;
  double z=msg->pose.pose.position.z;
  ROS_INFO("Husky Pose - x: [%f], y: [%f], z: [%f]", x, y, z);

}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "talker");
  ros::NodeHandle n;
  ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);

  /*Subscriber to get odometry data for husky in Ware house*/
  ros::Subscriber pose_sub=n.subscribe("/husky_velocity_controller/odom",100,odom_callback);
  ros::Rate loop_rate(10);
  int count = 0;
  while (ros::ok())
  {
    std_msgs::String msg;
    std::stringstream ss;
    ros::spinOnce();
    loop_rate.sleep();
    ++count;
  }


  return 0;
}
