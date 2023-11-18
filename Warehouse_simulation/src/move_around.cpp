#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include <sensor_msgs/LaserScan.h>


class Drive {
private:
    ros::NodeHandle n_;
    ros::Publisher vel_pub_;
    ros::Subscriber pose_sub_;
    ros::Subscriber scan_sub_;

   

    void odom_callback(const nav_msgs::Odometry::ConstPtr& msg) {
        double x = msg->pose.pose.position.x;
        double y = msg->pose.pose.position.y;
        ROS_INFO("Robot Pose - x: [%f], y: [%f]", x, y);
        move(x, y);
    }

    void scan_callback(const sensor_msgs::LaserScan::ConstPtr& scan_msg)

    {
    int center_index = scan_msg->ranges.size() / 2;
    float range = scan_msg->ranges[center_index];
    ROS_INFO("Range at center: %f", range);


    }
 

    void move(double robot_x, double robot_y) {
        geometry_msgs::Twist vel_msg;

        if(robot_x > 16.9) {
            vel_msg.angular.z = 0.3;
        }
        vel_msg.linear.x = 0.5;
        vel_pub_.publish(vel_msg);
    }

public:
    Drive()
        : n_() {


        vel_pub_ = n_.advertise<geometry_msgs::Twist>("/husky_velocity_controller/cmd_vel", 10);
        pose_sub_ = n_.subscribe("/husky_velocity_controller/odom", 10, &Drive::odom_callback, this);
        scan_sub_ = n_.subscribe("/scan",10,&Drive::scan_callback,this);

    }

    void run() {
        ros::Rate loop_rate(10);
        while (ros::ok()) {
            ros::spinOnce();
            loop_rate.sleep();
        }
    }
};

int main(int argc, char **argv) {
    ros::init(argc, argv, "drive");

    Drive navigator;
    navigator.run();

    return 0;
}
