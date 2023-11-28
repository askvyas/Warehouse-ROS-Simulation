// https://learnopencv.com/object-detection-using-yolov5-and-opencv-dnn-in-c-and-python/


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

// date time related
#include <chrono>
#include <iomanip>



class ObjDetector {
private:
    ros::NodeHandle n_;
    image_transport::Subscriber image_sub_;
        cv::VideoWriter video_writer_;
        cv::VideoCapture video_capture_;
        bool is_video_initialized_ = false;
        int frame_width_;
        int frame_height_;
        double frame_rate_;
        std::string video_path_;
    //check if dir exists
        void check_directory(const std::string& dir) {
        struct stat buffer;
        if(stat(dir.c_str(), &buffer) != 0) { 
            ROS_INFO("Creating video directory: %s", dir.c_str());
            mkdir(dir.c_str(), 0777); 
        }
    }

    void detect_objects():
    


    // image callback opens video_writer and saves using opencv
    void image_callback(const sensor_msgs::ImageConstPtr& msg) {
    try {
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);

        cv::imshow("Watcher", cv_ptr->image);
        cv::waitKey(30);

        if (!is_video_initialized_) {
            frame_width_ = cv_ptr->image.cols;
            frame_height_ = cv_ptr->image.rows;
            video_writer_.open(video_path_, cv::VideoWriter::fourcc('M','J','P','G'), frame_rate_, cv::Size(frame_width_, frame_height_), true);

            if (!video_writer_.isOpened()) {
                ROS_ERROR("Failed to open video ");
                return;
            }
            is_video_initialized_ = true;
        }

        video_writer_.write(cv_ptr->image);
    } catch (cv_bridge::Exception& e) {
        ROS_ERROR("Exception in image_callback: Could not convert from '%s' to 'bgr8'. Error: %s", msg->encoding.c_str(), e.what());
    }
}

public:
// code modification to update video file name everytime its ran
     Watcher()
        : n_(), frame_rate_(30.0){
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
        std::string time_str = ss.str();

        video_path_ = "/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder/robot_video_" + time_str + ".avi";


        check_directory("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder");


        image_transport::ImageTransport it(n_);
        image_sub_ = it.subscribe("/image_raw", 1, &Watcher::image_callback, this);
        cv::namedWindow("Watcher", cv::WINDOW_AUTOSIZE); 
    }

    ~Watcher() {
        if (video_writer_.isOpened()) {
            video_writer_.release();
        }
        cv::destroyWindow("Watcher"); 
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
    ros::init(argc, argv, "watcher");

    Watcher watcher;
    watcher.run();

    return 0;
}
