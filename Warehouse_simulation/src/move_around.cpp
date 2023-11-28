#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include <sensor_msgs/LaserScan.h>
#include <unordered_map>
#include <numeric>

class Drive
{
private:
    ros::NodeHandle n_;
    ros::Publisher vel_pub_;
    ros::Subscriber pose_sub_;
    ros::Subscriber scan_sub_;
    float safe_thresh = 4.5;

    std::vector<float> selectArea(const std::vector<float> &arr, int s, int id)
    {
        std::vector<float> points;
        int start_angle = 0;
        int end_angle = 0;
        if (id == 0)
        {
            start_angle = 0;
            end_angle = 180;
        }
        else if (id == 1)
        {
            start_angle = 180;
            end_angle = 360;
        }
        else if (id == 2)
        {
            start_angle = 360;
            end_angle = 540;
        }
        else if (id == 3)
        {
            start_angle = 540;
            end_angle = 719;
        }

        for (int i=start_angle; i < end_angle; i++)
        {
            points.push_back(arr[i]);
        }
        return points;
    }
    float average(std::vector<float> const &v)
    {
        if (v.empty())
        {
            return 0;
        }

        float sum = std::accumulate(v.begin(), v.end(), 0.0);
        return sum / v.size();
    }

    void scan_callback(const sensor_msgs::LaserScan::ConstPtr &scan_msg)

    {

        std::unordered_map<std::string, float> umap;
        std::vector<float> ll = selectArea(scan_msg->ranges, 720, 0);
        std::vector<float> lc = selectArea(scan_msg->ranges, 720, 1);
        std::vector<float> rc = selectArea(scan_msg->ranges, 720, 2);
        std::vector<float> rr = selectArea(scan_msg->ranges, 720, 3);
        umap["ll"] = average(ll);
        umap["lc"] = average(lc);
        umap["rc"] = average(rc);
        umap["rr"] = average(rr);
        std::string dir = "";

        float max_r = std::max({umap["ll"], umap["lc"], umap["rc"], umap["rr"]});
        float min_r = std::min({umap["ll"], umap["lc"], umap["rc"], umap["rr"]});
        // 3.5 saftery threshold : robot stopping quite early before the obstacle is detected
        float saftey_t=1.25;
        bool foundMax = false;
        for (const auto &x : umap)
        {
            if (x.second == max_r && !foundMax)
            {
                dir = x.first;
                foundMax = true;
            }
        }

        ROS_INFO("Max range: %f", max_r);
        ROS_INFO("Direction: %s", dir.c_str());


        if(min_r<saftey_t)
        {
            move(-1);

        }
        else
        {

        if (dir == "ll")
        {
            move(0);
        }
        else if (dir == "lc")
        {
            move(1);
        }
        else if (dir == "rc")
        {
            move(2);
        }
        else if (dir == "rr")
        {
            move(3);
        }
        else
        {
            move(4);
        }
        }
    }

    void move(int d)
    {
        geometry_msgs::Twist vel_msg;

        vel_msg.linear.x = 0.7;
        if (d == -1)
        {
            vel_msg.linear.x = -0.7;
            vel_msg.angular.z = 0.25;
        }

        else if (d == 0)
        {
            vel_msg.angular.z = -0.5;
        }
        else if (d == 1)
        {
            vel_msg.angular.z = -0.25;
        }
        else if (d == 2)
        {
            vel_msg.angular.z = 0.5;
        }
        else if (d == 3)
        {
            vel_msg.angular.z = 0.25;
        }
        else
        {
            vel_msg.linear.x = 0.7;
        }

        vel_pub_.publish(vel_msg);
    }

public:
    Drive()
        : n_()
    {

        vel_pub_ = n_.advertise<geometry_msgs::Twist>("/husky_velocity_controller/cmd_vel", 10);
        // pose_sub_ = n_.subscribe("/husky_velocity_controller/odom", 10, &Drive::odom_callback, this);
        scan_sub_ = n_.subscribe("/scan", 10, &Drive::scan_callback, this);
    }

    void run()
    {
        ros::Rate loop_rate(10);
        while (ros::ok())
        {
            ros::spinOnce();
            loop_rate.sleep();
        }
    }
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "drive");

    Drive navigator;
    navigator.run();

    return 0;
}
