#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"

class SimpleNavigation {
private:
    ros::NodeHandle n_;
    ros::Publisher vel_pub_;
    ros::Subscriber pose_sub_;
    float goal_x_;
    float goal_y_;
    bool goal_reached_;

    void odom_callback(const nav_msgs::Odometry::ConstPtr& msg) {
        double x = msg->pose.pose.position.x;
        double y = msg->pose.pose.position.y;
        ROS_INFO("Robot Pose - x: [%f], y: [%f]", x, y);
        move_towards_goal(x, y);
    }

void move_towards_goal(double robot_x, double robot_y) {
    geometry_msgs::Twist vel_msg;
    if (goal_reached_) {
        vel_msg.linear.x = 0;
        vel_msg.angular.z = 0;
    } else {
        double angle_to_goal = std::atan2(goal_y_ - robot_y, goal_x_ - robot_x);
        double distance_to_goal = std::hypot(goal_x_ - robot_x, goal_y_ - robot_y);
        double angle_difference = angle_to_goal; 
        vel_msg.angular.z = 1 * angle_difference;

        if (std::fabs(angle_difference) < 0.1) { 
            vel_msg.linear.x = std::min(distance_to_goal, 0.7);
        } else {
            vel_msg.linear.x = 0; 
        }

        if (distance_to_goal < 0.1) { 
            vel_msg.linear.x = 0;
            vel_msg.angular.z = 0;
            goal_reached_ = true;
            ROS_INFO("Goal reached!");
        }
    }

    vel_pub_.publish(vel_msg);
}
public:
    SimpleNavigation(float goal_x, float goal_y)
        : n_(),
          goal_x_(goal_x),
          goal_y_(goal_y),
          goal_reached_(false) {
        vel_pub_ = n_.advertise<geometry_msgs::Twist>("/husky_velocity_controller/cmd_vel", 10);
        pose_sub_ = n_.subscribe("/husky_velocity_controller/odom", 10, &SimpleNavigation::odom_callback, this);
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
    ros::init(argc, argv, "simple_navigation");

    // Set your desired goal position here
    float goal_x = 5.8;
    float goal_y = 9.3;
    SimpleNavigation navigator(goal_x, goal_y);
    navigator.run();

    return 0;
}
