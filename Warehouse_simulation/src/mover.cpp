#include "ros/ros.h"
#include "std_msgs/String.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"


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
  ros::init(argc, argv, "mover");

  
  ros::NodeHandle n;
  ros::Publisher vel_pub = n.advertise<geometry_msgs::Twist>("/husky_velocity_controller/cmd_vel", 10);
  ros::Publisher chatter_pub = n.advertise<std_msgs::String>("chatter", 1000);

  /*Subscriber to get odometry data for husky in Ware house*/
  ros::Subscriber pose_sub=n.subscribe("/husky_velocity_controller/odom",100,odom_callback);
  ros::Rate loop_rate(10);
  int count = 0;
  while (ros::ok())
  {
        geometry_msgs::Twist vel_msg;

        vel_msg.linear.x = 1.0; 
        vel_msg.linear.y = 0.0;
        vel_msg.linear.z = 0.0;
        vel_msg.angular.x = 0.0;
        vel_msg.angular.y = 0.0;
        vel_msg.angular.z = 0.0;
        vel_pub.publish(vel_msg);
        ros::spinOnce();
        loop_rate.sleep();

    ros::spinOnce();
    loop_rate.sleep();
    ++count;
  }


  return 0;
}

