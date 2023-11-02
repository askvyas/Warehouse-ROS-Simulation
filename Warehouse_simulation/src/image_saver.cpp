#include "ros/ros.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <sensor_msgs/image_encodings.h>


/*Image callback */
void imageCallback(const sensor_msgs::ImageConstPtr& msg) {
    try {
        cv_bridge::CvImagePtr cv_ptr;
        cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        
        static int image_count = 0;
        std::stringstream ss;
        /*Save the images in the image_folder and name it with the count */
        ss << "/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/image_folder/image_" << image_count++ << ".jpg";
        cv::imwrite(ss.str(), cv_ptr->image);
        ROS_INFO("Image saved: %s", ss.str().c_str());

    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
    }
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "image_saver");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_transport::Subscriber sub = it.subscribe("/camera/image_raw", 1, imageCallback);
    ros::spin();
    return 0;
}
