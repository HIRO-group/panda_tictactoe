#include <string>
#include <ros/ros.h>
#include <ros/console.h>
#include <ros/assert.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ttt/tictactoe_utils.h"


namespace ttt
{


class HsvRangeFinder
{
private:

    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;

    int lowerH;
    int lowerS;
    int lowerV;
    int upperH;
    int upperS;
    int upperV;

    std::string window;

public:
    HsvRangeFinder() : it_(nh_), window("HSV Range Finder")
    {
        image_sub_ = it_.subscribe("image_in", 1, &HsvRangeFinder::imageCb, this);

        cv::namedWindow(window);

        lowerH=0;
        lowerS=0;
        lowerV=0;
        upperH=180;
        upperS=256;
        upperV=256;

        cv::createTrackbar("LowerH", window, &lowerH, 180, NULL);
        cv::createTrackbar("UpperH", window, &upperH, 180, NULL);
        cv::createTrackbar("LowerS", window, &lowerS, 256, NULL);
        cv::createTrackbar("UpperS", window, &upperS, 256, NULL);
        cv::createTrackbar("LowerV", window, &lowerV, 256, NULL);
        cv::createTrackbar("UpperV", window, &upperV, 256, NULL);
    }

    ~HsvRangeFinder()
    {
        cv::destroyWindow(window);
    }

    void imageCb(const sensor_msgs::ImageConstPtr& msg)
    {
        //converting ROS image format to opencv image format
        cv_bridge::CvImageConstPtr cv_ptr;
        try
        {
            cv_ptr = cv_bridge::toCvShare(msg, enc::BGR8);
        }
        catch (cv_bridge::Exception& e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
            return;
        }

        cv::Mat img_hsv(cv_ptr->image.rows, cv_ptr->image.cols,CV_8UC3);
        cv::cvtColor(cv_ptr->image, img_hsv, CV_BGR2HSV); //Change the color format from BGR to HSV

        cv::Mat img_thresh = GetThresholdedImage(img_hsv, cv::Scalar(lowerH,lowerS,lowerV), cv::Scalar(upperH,upperS,upperV));

        cv::imshow(window, img_thresh);

        int c = cv::waitKey(3);
        if( (c & 255) == 27 ) // ESC key pressed
        {
            ROS_INFO_STREAM("H=[" << lowerH << ".." << upperH << "] S=[" << lowerS << ".." << upperS << "] V=[" << lowerV << ".." << upperV << "]");
            ros::shutdown();
        }        
    }

    //This function threshold the HSV image and create a binary image
    static cv::Mat GetThresholdedImage(const cv::Mat& img_hsv, cv::Scalar lower, cv::Scalar upper)
    {

     cv::Mat img_thresh(img_hsv.rows,img_hsv.cols,CV_8U);
     cv::inRange(img_hsv, lower, upper, img_thresh);

     return img_thresh;

    }
};

}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "hsv_range_finder");
    ttt::HsvRangeFinder cd;
    ros::spin();
    return 0;
}
