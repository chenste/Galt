/*
 * node.hpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of viso_kf.
 *
 *	Created on: 03/08/2014
 *		  Author: gareth
 */

#ifndef VISO_KF_NODE_HPP_
#define VISO_KF_NODE_HPP_

#include <ros/ros.h>
#include <ros/node_handle.h>

#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>
#include <tf2/buffer_core.h>
#include <tf2_ros/transform_listener.h>

#include "viso_kf/error_state_kf.hpp"

#include "rviz_helper/marker_visualizer.hpp"
#include "rviz_helper/tf_publisher.hpp"

namespace viso_kf {

class Node {
 public:
  static constexpr double kOneG = 9.80665;

  /**
   * @brief Construct node and load parameters.
   */
  Node();

  /**
   * @brief Subscribe to topics and advertise outputs.
   */
  void initialize();

 private:
  void imuCallback(const sensor_msgs::ImuConstPtr &imu);

  void odoCallback(const nav_msgs::OdometryConstPtr &odometry);

  ErrorStateKF<double> positionKF_;
  double gyroBiasDriftStd_;
  double accelBiasDriftStd_;
  double initDeadTime_;

  bool initialized_{false};

  ros::NodeHandle nh_;
  ros::Publisher pubOdometry_;
  ros::Publisher pubAccelBias_;
  ros::Publisher pubGyroBias_;
  ros::Publisher pubPose_;
  ros::Subscriber subImu_;
  ros::Subscriber subOdometry_;
  ros::Time predictTime_;
  tf2::BufferCore tfCore_;
  tf2_ros::TransformListener tfListener_;

  std::string worldFrameId_;
  kr::viz::TfPublisher tfPub_;
  kr::viz::TrajectoryVisualizer trajViz_;
  kr::viz::CovarianceVisualizer covViz_;
};

}  //  namespace viso_kf

#endif  //  VISO_KF_NODE_HPP_
