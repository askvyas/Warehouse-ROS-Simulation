#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Image.h"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/dnn.hpp>
#include <sstream>
#include <sys/stat.h>
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
    cv::dnn::Net net; // YOLO network

    void check_directory(const std::string& dir) {
        struct stat buffer;
        if(stat(dir.c_str(), &buffer) != 0) { 
            ROS_INFO("Creating video directory: %s", dir.c_str());
            mkdir(dir.c_str(), 0777); 
        }
    }

    void detect_objects(cv::Mat& frame) {
        // Preprocess the frame
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1/255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);

        // Set the input to the network
        net.setInput(blob);

        // Forward pass
        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());

        // Process outputs
        for (auto& output : outputs) {
            float* data = (float*)output.data;
            for (int i = 0; i < output.rows; i++, data += output.cols) {
                float confidence = data[2];
                if (confidence > 0.5) {
                    int left   = static_cast<int>(data[3] * frame.cols);
                    int top    = static_cast<int>(data[4] * frame.rows);
                    int right  = static_cast<int>(data[5] * frame.cols);
                    int bottom = static_cast<int>(data[6] * frame.rows);

                    cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 255, 0), 3);
                }
            }
        }
    }

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

            detect_objects(cv_ptr->image); // Call object detection on the frame
            video_writer_.write(cv_ptr->image);

        } catch (cv_bridge::Exception& e) {
            ROS_ERROR("Exception in image_callback: Could not convert from '%s' to 'bgr8'. Error: %s", msg->encoding.c_str(), e.what());
        }
    }

public:
    ObjDetector()
        : n_(), frame_rate_(30.0) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");
        std::string time_str = ss.str();

        video_path_ = "/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder/robot_video_" + time_str + ".avi";
        net = cv::dnn::readNetFromONNX("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/yolov4.onnx");

        check_directory("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder");

        image_transport::ImageTransport it(n_);
        image_sub_ = it.subscribe("/image_raw", 1, &ObjDetector::image_callback, this);
        cv::namedWindow("Watcher", cv::WINDOW_AUTOSIZE); 
    }

    ~ObjDetector() {
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
    ros::init(argc, argv, "obj_detection");

    ObjDetector watcher;
    watcher.run();

    return 0;
}
