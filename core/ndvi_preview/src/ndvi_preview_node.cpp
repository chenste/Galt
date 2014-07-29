/*
 * ndvi_preview_node.cpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of galt.
 *
 *  Created on: 29/7/2014
 *      Author: gareth
 */

#include <iostream>
#include <string>
#include <algorithm>

#include <ros/ros.h>

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

#define QUEUE_SIZE  (10)

int save_number = 0;

void image_callback(const sensor_msgs::ImageConstPtr &irImage,
                    const sensor_msgs::CameraInfoConstPtr &irInfo,
                    const sensor_msgs::ImageConstPtr &redImage,
                    const sensor_msgs::CameraInfoConstPtr &redInfo) {
  
  cv_bridge::CvImageConstPtr bridgedIrPtr =
      cv_bridge::toCvShare(irImage, "mono8");
  cv_bridge::CvImageConstPtr bridgedRedPtr = 
      cv_bridge::toCvShare(redImage, "mono8");
  
  if (!bridgedIrPtr || !bridgedRedPtr) {
    ROS_ERROR("Failed to convert image to mono8 with cv_bridge");
    return;
  }
  
  cv::Mat outputIr(bridgedIrPtr->image.size(), CV_8UC1);
  
  //  try to correct the IR image
  for (int y=0; y < bridgedIrPtr->image.rows; y++) {
    for (int x=0; x < bridgedIrPtr->image.cols; x++) {
      double delx = x - 464;
      double dely = y - 667;
      
      double rad = std::sqrt(delx*delx + dely*dely);
      double mul = 1.0 + 1.25 * (rad / 500.0);
      
      uchar s = bridgedIrPtr->image.at<uchar>(y,x);
      s = static_cast<uchar>( std::min(255.0, s * mul) );
      
      outputIr.at<uchar>(y,x) = s;
    }
  }
  
  cv::imshow("IR", outputIr);
  cv::imshow("Red", bridgedRedPtr->image);
  if (cv::waitKey(30) == static_cast<int>('s')) {
    //  save images
    cv::imwrite("output_ir_" + std::to_string(save_number) + ".png", bridgedIrPtr->image);
    cv::imwrite("output_red_" + std::to_string(save_number) + ".png", bridgedRedPtr->image);    
    
    save_number++;
  }
}

int main(int argc, char ** argv) {
  ros::init(argc, argv, "ndvi_preview_node");
  
  ros::NodeHandle nh("~");
  image_transport::ImageTransport imgTransport(nh);
  
  image_transport::SubscriberFilter subIRImage;
  image_transport::SubscriberFilter subRedImage;
  
  message_filters::Subscriber<sensor_msgs::CameraInfo> subIRInfo;
  message_filters::Subscriber<sensor_msgs::CameraInfo> subRedInfo;
  
  subIRImage.subscribe(imgTransport, "infrared/image_raw", QUEUE_SIZE);
  subRedImage.subscribe(imgTransport, "red/image_raw", QUEUE_SIZE);
  
  subIRInfo.subscribe(nh, "infrared/camera_info", QUEUE_SIZE);
  subRedInfo.subscribe(nh, "red/camera_info", QUEUE_SIZE);
  
  //  time sync policy
  typedef message_filters::sync_policies::ApproximateTime<
      sensor_msgs::Image, sensor_msgs::CameraInfo, sensor_msgs::Image, sensor_msgs::CameraInfo> TimeSyncPolicy;
  
  message_filters::Synchronizer<TimeSyncPolicy> sync(TimeSyncPolicy(QUEUE_SIZE),
                                                     subIRImage,subIRInfo,subRedImage,subRedInfo);
  sync.registerCallback( boost::bind(&image_callback,_1,_2,_3,_4) );
  
  cv::namedWindow("IR", cv::WINDOW_NORMAL);
  cv::namedWindow("Red", cv::WINDOW_NORMAL);
  
  ros::spin();
  return 0;
}
