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


class Watcher()
{




    public:
    Watcher():
        n_(), frame_rate_(30.0), video_path_("/home/vyas/catkin_ws/src/Warehouse-ROS-Simulation/Warehouse_simulation/video_folder/robot_video.avi")
        {
            
        }
};
