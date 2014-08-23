/*
 * flir_gige.cpp
 *  _   _             _           _____         _
 * | \ | | ___  _   _| | ____ _  |_   _|__  ___| |__
 * |  \| |/ _ \| | | | |/ / _` |   | |/ _ \/ __| '_ \
 * | |\  | (_) | |_| |   < (_| |   | |  __/ (__| | | |
 * |_| \_|\___/ \__,_|_|\_\__,_|   |_|\___|\___|_| |_|
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of flir_gige.
 *
 *	Created on: 21/08/2014
 */

#include "flir_gige/flir_gige.h"

#include <memory>
#include <functional>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <std_msgs/Header.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/Temperature.h>
#include <dynamic_reconfigure/server.h>
#include <cv_bridge/cv_bridge.h>

#include <opencv2/core/core.hpp>

#include "flir_gige/gige_camera.h"

namespace flir_gige {

using sensor_msgs::CameraInfo;
using sensor_msgs::CameraInfoPtr;
using camera_info_manager::CameraInfoManager;

FlirGige::FlirGige(const ros::NodeHandle &nh, const ros::NodeHandle &pnh)
    : nh_{nh}, pnh_{pnh}, it_{pnh} {
  // Get ros parameteres
  double fps;
  pnh_.param<double>("fps", fps, 20.0);
  ROS_ASSERT_MSG(fps > 0, "FlirGige: fps must be greater than 0");
  pnh_.param<std::string>("frame_id", frame_id_, std::string("flir_a5"));
  rate_.reset(new ros::Rate(fps));

  // Create a camera
  std::string ip_address;
  pnh_.param<std::string>("ip_address", ip_address, std::string(""));
  gige_camera_.reset(new GigeCamera(ip_address));
  gige_camera_->use_image =
      std::bind(&FlirGige::PublishImage, this, std::placeholders::_1,
                std::placeholders::_2);
  gige_camera_->use_temperature =
      std::bind(&FlirGige::PublishTemperature, this, std::placeholders::_1);

  // Setup camera publisher and dynamic reconfigure callback
  std::string calib_url;
  pnh_.param<std::string>("calib_url", calib_url, "");
  cinfo_manager_.reset(new CameraInfoManager(pnh_, frame_id_, calib_url));
  if (!cinfo_manager_->isCalibrated()) {
    ROS_WARN_STREAM("FlirGige: " << frame_id_ << " not calibrated");
  }
  cinfo_ = CameraInfoPtr(new CameraInfo(cinfo_manager_->getCameraInfo()));

  pub_camera_ = it_.advertiseCamera("image_raw", 1);
  pub_temperature_ = pnh_.advertise<sensor_msgs::Temperature>("spot", 1);
  server_.setCallback(boost::bind(&FlirGige::ConfigCb, this, _1, _2));
}

void FlirGige::Run() {
  GigeConfig config;
  nh_.param<bool>("color", config.color, config.color);
  nh_.param<int>("bit", config.bit, config.bit);
  gige_camera_->Connect();
  gige_camera_->Configure(config);
  gige_camera_->Start();
}

void FlirGige::End() {
  gige_camera_->Stop();
  gige_camera_->Disconnect();
}

void FlirGige::PublishImage(const cv::Mat &image, const Planck &planck) {
  std_msgs::Header header;
  header.stamp = ros::Time::now();
  header.frame_id = frame_id_;
  // We put planck constants into CameraInfo::R. The orders are B F O R
  cinfo_->R[0] = planck.B;
  cinfo_->R[1] = planck.F;
  cinfo_->R[2] = planck.O;
  cinfo_->R[3] = planck.R;
  // Convert to ros image msg and publish camera
  cv_bridge::CvImage cv_image(header, GetImageEncoding(image), image);
  image_ = cv_image.toImageMsg();
  cinfo_->header = header;
  pub_camera_.publish(image_, cinfo_);
  rate_->sleep();
}

void FlirGige::PublishTemperature(const std::pair<double, double> &spot) {
  // Construct a temperature mesage
  sensor_msgs::Temperature temperature;
  temperature.header.stamp = ros::Time::now();
  temperature.header.frame_id = frame_id_;
  temperature.temperature = spot.first;
  temperature.variance = spot.second;
  pub_temperature_.publish(temperature);
}

std::string FlirGige::GetImageEncoding(const cv::Mat &image) const {
  std::string encoding;
  switch (image.type()) {
    case CV_8UC1:
      encoding = sensor_msgs::image_encodings::MONO8;
      break;
    case CV_8UC3:
      encoding = sensor_msgs::image_encodings::BGR8;
      break;
    case CV_16UC1:
      encoding = sensor_msgs::image_encodings::MONO16;
      break;
    default:
      encoding = sensor_msgs::image_encodings::MONO8;
  }
  return encoding;
}

void FlirGige::ConfigCb(DynConfig &config, int level) {
  // Do nothing when first starting
  if (level < 0) {
    return;
  }
  // Get config
  GigeConfig gige_config;
  // Color image only works with 8-bit output
  if (config.color) config.bit = 2;
  gige_config.color = config.color;
  gige_config.bit = config.bit;
  // Stop the camera if in acquisition
  if (gige_camera_->IsAcquire()) {
    // Stop the image thread if camera is running
    gige_camera_->Stop();
    gige_camera_->Disconnect();
  }
  gige_camera_->Connect();
  gige_camera_->Configure(gige_config);
  gige_camera_->Start();
}

}  // namespace flir_gige
