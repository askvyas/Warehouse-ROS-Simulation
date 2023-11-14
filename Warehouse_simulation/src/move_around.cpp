#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Image.h"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>
#include <sys/stat.h>

class Drive {
private:
    ros::NodeHandle n_;
    ros::Publisher vel_pub_;
    ros::Subscriber pose_sub_;
    image_transport::Subscriber image_sub_;

    cv::VideoWriter video_writer_;
    cv::VideoCapture video_capture_;
    bool is_video_initialized_ = false;
    int frame_width_;
    int frame_height_;
    double frame_rate_;
    std::string video_path_;

    void check_directory(const std::string& dir) {
        struct stat buffer;
        if(stat(dir.c_str(), &buffer) != 0) { 
            ROS_INFO("Creating video directory: %s", dir.c_str());
            mkdir(dir.c_str(), 0777); 
        }
    }

    void odom_callback(const nav_msgs::Odometry::ConstPtr& msg) {
        double x = msg->pose.pose.position.x;
        double y = msg->pose.pose.position.y;
        ROS_INFO("Robot Pose - x: [%f], y: [%f]", x, y);
        move(x, y);
    }

    void image_callback(const sensor_msgs::ImageConstPtr& msg) {
        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);

            if (!is_video_initialized_) {
                frame_width_ = cv_ptr->image.cols;
                frame_height_ = cv_ptr->image.rows;
                video_writer_.open(video_path_, cv::VideoWriter::fourcc('M','J','P','G'), frame_rate_, cv::Size(frame_width_, frame_height_), true);

                if (!video_writer_.isOpened()) {
                    ROS_ERROR("Failed to open video for writing");
                    return;
                }
                is_video_initialized_ = true;
            }

            video_writer_.write(cv_ptr->image);
        }
        catch (cv_bridge::Exception& e) {
            ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
        }
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
        : n_(), frame_rate_(30.0), video_path_("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder/robot_video.avi") {

        check_directory("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder");

        vel_pub_ = n_.advertise<geometry_msgs::Twist>("/husky_velocity_controller/cmd_vel", 10);
        pose_sub_ = n_.subscribe("/husky_velocity_controller/odom", 10, &Drive::odom_callback, this);

        image_transport::ImageTransport it(n_);
        image_sub_ = it.subscribe("/camera/image_raw", 1, &Drive::image_callback, this);
    }

    ~Drive() {
        if (video_writer_.isOpened()) {
            video_writer_.release();
        }
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
