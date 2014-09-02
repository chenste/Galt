/*
 * node.hpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of gps_odom.
 *
 *	Created on: 31/07/2014
 *		  Author: gareth
 */

#ifndef GPS_ODOM_NODE_HPP
#define GPS_ODOM_NODE_HPP

#include <ros/ros.h>
#include <ros/node_handle.h>

#include <sensor_msgs/Imu.h>
#include <sensor_msgs/FluidPressure.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/NavSatStatus.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <geometry_msgs/TwistWithCovarianceStamped.h>
#include <message_filters/subscriber.h>
#include <message_filters/sync_policies/exact_time.h>
#include <message_filters/sync_policies/approximate_time.h>

#include <pressure_altimeter/Height.h>

#include <GeographicLib/Geoid.hpp>
#include <GeographicLib/MagneticModel.hpp>
#include <GeographicLib/LocalCartesian.hpp>

#include <memory>

namespace gps_odom {

class Node
{
public:
  Node();
  
  void initialize();
  
private:
   
  static constexpr int kROSQueueSize = 200;
  
  ros::NodeHandle nh_;
  std::string pkgPath_;
  std::string childFrameId_;
  std::string frameId_;
  ros::Publisher pubOdometry_;
  ros::Publisher pubImu_;
  
  message_filters::Subscriber<sensor_msgs::Imu> subImu_;
  message_filters::Subscriber<sensor_msgs::FluidPressure> subPressure_;
  
  //  time sync policy for IMU data
  using TimeSyncIMU = message_filters::sync_policies::ExactTime<
    sensor_msgs::Imu, sensor_msgs::FluidPressure>;
  using SynchronizerIMU = message_filters::Synchronizer<TimeSyncIMU>;
  
  std::shared_ptr<SynchronizerIMU> syncImu_;
  
  void imuCallback(const sensor_msgs::ImuConstPtr&);
  
  message_filters::Subscriber<sensor_msgs::NavSatFix> subFix_;
  message_filters::Subscriber<geometry_msgs::TwistWithCovarianceStamped> subFixTwist_;
  
  //  time sync policy for GPS data
  using TimeSyncGPS = message_filters::sync_policies::ApproximateTime<
    sensor_msgs::NavSatFix, 
    geometry_msgs::TwistWithCovarianceStamped,
    sensor_msgs::Imu>;
  using SynchronizerGPS = message_filters::Synchronizer<TimeSyncGPS>;
  
  std::shared_ptr<SynchronizerGPS> syncGps_;
  
  void gpsCallback(const sensor_msgs::NavSatFixConstPtr&,
                   const geometry_msgs::TwistWithCovarianceStampedConstPtr &navSatTwist,
                   const sensor_msgs::ImuConstPtr&);
  
  //  geographic lib objects
  std::shared_ptr<GeographicLib::Geoid> geoid_;
  std::shared_ptr<GeographicLib::MagneticModel> magneticModel_;
  
  bool refSet_;
  GeographicLib::LocalCartesian refPoint_;
  double currentDeclination_;
};

} //  namespace_gps_odom

#endif // GPS_ODOM_NODE_HPP
