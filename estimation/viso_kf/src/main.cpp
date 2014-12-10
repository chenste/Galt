/*
 * main.cpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of gps_odom.
 *
 *	Created on: 03/08/2014
 *		  Author: gareth
 */

#include <ros/ros.h>
#include <viso_kf/node.hpp>

int main(int argc, char** argv) {
  ros::init(argc, argv, "viso_kf");
  viso_kf::Node node;
  node.initialize();
  ros::spin();
}
