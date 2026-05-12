#pragma once

#include <string>

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

class RosImageToRtmp
{
public:
    RosImageToRtmp(ros::NodeHandle& nh, ros::NodeHandle& pnh);

private:
    void openWriter();
    void imageCallback(const sensor_msgs::ImageConstPtr& msg);

    // ROS
    ros::Subscriber sub_;
    ros::Time last_frame_time_;

    // OpenCV writer
    cv::VideoWriter writer_;

    // Parameters
    std::string image_topic_;
    std::string rtmp_url_;

    int image_rate_;
    int width_;
    int height_;
    int bitrate_;
};
