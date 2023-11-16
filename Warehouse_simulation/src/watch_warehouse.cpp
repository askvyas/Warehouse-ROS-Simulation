#include "ros/ros.h"
#include "sensor_msgs/Image.h"
#include "cv_bridge/cv_bridge.h"
#include "image_transport/image_transport.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class Watcher {
private:
    ros::NodeHandle n_;
    image_transport::Subscriber image_sub_;

    void image_callback(const sensor_msgs::ImageConstPtr& msg) {
        try {
            cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
            cv::imshow("Frame", cv_ptr->image);
            cv::waitKey(30); // Wait for a brief period to display the frame
        } catch (cv_bridge::Exception& e) {
            ROS_ERROR("Exception in image_callback: %s", e.what());
        }
    }

public:
    Watcher()
        : n_() {
        image_transport::ImageTransport it(n_);
        image_sub_ = it.subscribe("/camera/image_raw", 1, &Watcher::image_callback, this);
        cv::namedWindow("Frame", cv::WINDOW_AUTOSIZE); // Create a window for display.
    }

    ~Watcher() {
        cv::destroyWindow("Frame"); // Destroy the OpenCV window
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
