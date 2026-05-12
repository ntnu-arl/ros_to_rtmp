#include "ros_to_rtmp/ros_image_to_rtmp.h"

RosImageToRtmp::RosImageToRtmp(ros::NodeHandle& nh, ros::NodeHandle& pnh)
{
    pnh.param<std::string>("image_topic", image_topic_, "/camera/image_raw");
    pnh.param<std::string>("rtmp_url", rtmp_url_, "rtmp://localhost/live/stream");
    pnh.param<int>("image_rate", image_rate_, 30);
    pnh.param<int>("width", width_, 640);
    pnh.param<int>("height", height_, 480);
    pnh.param<int>("bitrate", bitrate_, 2500);

    if (image_rate_ <= 0)
        image_rate_ = 30;

    openWriter();

    sub_ = nh.subscribe(image_topic_, 10, &RosImageToRtmp::imageCallback, this);

    ROS_INFO("[ros_to_rtmp] Streaming %s to %s at %dx%d @ %d FPS",
             image_topic_.c_str(),
             rtmp_url_.c_str(),
             width_,
             height_,
             image_rate_);
}

void RosImageToRtmp::openWriter()
{
    std::string pipeline =
        "appsrc is-live=true block=true format=time "
        "caps=video/x-raw,format=BGR,width=" + std::to_string(width_) +
        ",height=" + std::to_string(height_) +
        ",framerate=" + std::to_string(image_rate_) + "/1 "
        "! videoconvert "
        "! x264enc tune=zerolatency bitrate=" + std::to_string(bitrate_) +
        " speed-preset=superfast key-int-max=" + std::to_string(image_rate_) + " "
        "! video/x-h264,profile=baseline "
        "! flvmux streamable=true "
        "! rtmpsink location=\"" + rtmp_url_ + "\"";

    writer_.open(
        pipeline,
        cv::CAP_GSTREAMER,
        0,
        static_cast<double>(image_rate_),
        cv::Size(width_, height_),
        true
    );

    if (!writer_.isOpened())
    {
        ROS_ERROR("[ros_to_rtmp] Failed to open RTMP writer. Check OpenCV GStreamer support and RTMP URL.");
    }
}

void RosImageToRtmp::imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
    if (!writer_.isOpened())
        return;

    ros::Time now = ros::Time::now();
    const double min_dt = 1.0 / static_cast<double>(image_rate_);

    // Rate limiting
    ros::Duration dt = now - last_frame_time_;
    if (!last_frame_time_.isZero() &&
        (dt).toSec() < min_dt)
    {
        return;
    }
    last_frame_time_ = now;

    cv_bridge::CvImageConstPtr cv_ptr;

    try
    {
        cv_ptr = cv_bridge::toCvShare(msg, "bgr8");
    }
    catch (const cv_bridge::Exception& e)
    {
        ROS_WARN_THROTTLE(2.0, "[ros_to_rtmp] cv_bridge error: %s", e.what());
        return;
    }

    const cv::Mat& frame = cv_ptr->image;

    if (frame.empty())
        return;

    // Resize to requested output size
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(width_, height_));

    // Send frame to RTMP stream
    writer_.write(resized);
    ROS_INFO_THROTTLE(5.0, "[ros_to_rtmp] Streaming frame at %f FPS", 1.0 / (dt).toSec());
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "ros_image_to_rtmp");

    ros::NodeHandle nh;
    ros::NodeHandle pnh("~");

    RosImageToRtmp node(nh, pnh);

    ros::spin();
    return 0;
}