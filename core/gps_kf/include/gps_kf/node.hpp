/*
 * node.hpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of gps_kf.
 *
 *	Created on: 03/08/2014
 *		  Author: gareth
 */

#ifndef GPS_KF_NODE_HPP
#define GPS_KF_NODE_CPP

#include <ros/ros.h>
#include <ros/node_handle.h>

#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/static_transform_broadcaster.h>

#include <sensor_msgs/Imu.h>
#include <nav_msgs/Odometry.h>

#include <gps_kf/error_state_kf.hpp>

#include <rviz_helper/rviz_helper.h>

namespace gps_kf {

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
  
  bool initialized_{ false };

  ros::NodeHandle nh_;
  ros::Publisher pubOdometry_;
  ros::Publisher pubAccelBias_;
  ros::Publisher pubGyroBias_;
  ros::Publisher pubPose_;
  ros::Subscriber subImu_;
  ros::Subscriber subOdometry_;
  ros::Time predictTime_;

  std::string worldFrameId_;
  tf2_ros::TransformBroadcaster broadcaster_;
  rviz_helper::TrajectoryVisualizer trajViz_;
  rviz_helper::CovarianceVisualizer covViz_;
};

} //  namespace gps_kf

#endif //  GPS_KF_NODE_HPP
