#include "ros/ros.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "move_base_msgs/MoveBaseAction.h"
#include "actionlib/client/simple_action_client.h"


// This node uses ACML, Move base packages
// ACML --> estimates  robots position



typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

class WarehouseNavigator {
private:
    ros::NodeHandle nh_;
    ros::Publisher vel_pub_;
    ros::Subscriber laser_sub_;
    MoveBaseClient ac_;
    bool obstacle_detected_;

    void laserCallback(const sensor_msgs::LaserScan::ConstPtr& scan) {
        obstacle_detected_ = false;
        for (const auto& range : scan->ranges) {
            // Trial 1: 1meter
            if (range < 1.0) { 
                obstacle_detected_ = true;
                break;
            }
        }
    }

    void moveToGoal(float x, float y) {
        move_base_msgs::MoveBaseGoal goal;
        goal.target_pose.header.frame_id = "base_link";
        goal.target_pose.header.stamp = ros::Time::now();
        goal.target_pose.pose.position.x = x;
        goal.target_pose.pose.position.y = y;
        goal.target_pose.pose.orientation.w = 1.0;

        ROS_INFO("Sending goal");
        ac_.sendGoal(goal);
        ac_.waitForResult();

        if(ac_.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
            ROS_INFO("Reached goal");
        else
            ROS_INFO("Not Yet");
    }

public:
    WarehouseNavigator() : ac_("move_base", true), vel_pub_(), laser_sub_(), obstacle_detected_(false) {
        vel_pub_ = nh_.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
        laser_sub_ = nh_.subscribe("/scan", 10, &WarehouseNavigator::laserCallback, this);

        while(!ac_.waitForServer(ros::Duration(5.0))){
            ROS_INFO("Waiting for node to communicate to move_base");
        }
    }

    void start() {
        ros::Rate rate(10); 
        while (ros::ok()) {
            if (obstacle_detected_) {
                geometry_msgs::Twist stop_msg;
                stop_msg.linear.x = 0;
                stop_msg.angular.z = 0;
                vel_pub_.publish(stop_msg);
            } else {
                moveToGoal(15, 0); 
            }
            ros::spinOnce();
            rate.sleep();
        }
    }
};

int main(int argc, char **argv) {
    ros::init(argc, argv, "warehouse_navigator");

    WarehouseNavigator navigator;
    navigator.start();

    ros::spin();
    return 0;
}
