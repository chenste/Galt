/*
 * CameraCalibration.cpp
 *
 *  Copyright (c) 2014 Nouka Technologies. All rights reserved.
 *
 *  This file is part of galt.
 *
 *  Created on: 21/7/2014
 *      Author: gareth
 */

#include <spectral/CameraCalibration.hpp>
#include "yaml_utilities.hpp"

namespace galt {

CameraCalibration::CameraCalibration()
    : cameraSerial(), cameraExposure(0), calibrationDate(), slope(0),
      intercept(0), squaredError(0), spectrometerPose(), filterProfile() {}

CameraCalibration::CameraCalibration(const std::string &camSerial,
                                     int camExposure,
                                     const std::string &isoCalibDate,
                                     double slope, 
                                     double intercept, double squaredError,
                                     const galt::SpectrometerPose &specPose,
                                     const galt::FilterProfile &profile)
    : cameraSerial(camSerial), cameraExposure(camExposure),
      calibrationDate(isoCalibDate), slope(slope), intercept(intercept),
      squaredError(squaredError), spectrometerPose(specPose), 
      filterProfile(profile) {}
}

YAML::Node YAML::convert<galt::CameraCalibration>::encode(
    const galt::CameraCalibration &rhs) {
  YAML::Node node;
  node["camera_serial"] = rhs.cameraSerial;
  node["camera_exposure"] = rhs.cameraExposure;
  node["calibration_date"] = rhs.calibrationDate;
  node["slope"] = rhs.slope;
  node["intercept"] = rhs.intercept;
  node["squared_error"] = rhs.squaredError;
  node["spectrometer_pose"] = rhs.spectrometerPose;
  node["filter_profile"] = rhs.filterProfile;
  return node;
}

bool
YAML::convert<galt::CameraCalibration>::decode(const YAML::Node &node,
                                               galt::CameraCalibration &rhs) {

  const auto requiredFields = { "camera_serial",    "camera_exposure",
                                "calibration_date", "slope",
                                "intercept", "squared_error", "spectrometer_pose",
                                "filter_profile" };
  if (!node.IsMap() || !galt::hasFields(node, requiredFields)) {
    return false;
  }

  //  TODO: Update this to support integer camera serials?
  rhs = galt::CameraCalibration(
      node["camera_serial"].as<std::string>(),
      node["camera_exposure"].as<int>(),
      node["calibration_date"].as<std::string>(), node["slope"].as<double>(),
      node["intercept"].as<double>(),
      node["squared_error"].as<double>(),
      node["spectrometer_pose"].as<galt::SpectrometerPose>(),
      node["filter_profile"].as<galt::FilterProfile>());
  
  return true;
}


YAML::Emitter &operator<<(YAML::Emitter &out, 
                          const galt::CameraCalibration &calib) {
  out << YAML::Block;
  out << YAML::BeginMap;
  out << YAML::Key << "camera_serial" << YAML::Value << calib.cameraSerial;
  out << YAML::Key << "camera_exposure" << YAML::Value << calib.cameraExposure;
  out << YAML::Key << "calibration_date" << YAML::Value << calib.calibrationDate;
  out << YAML::Key << "slope" << YAML::Value << calib.slope;
  out << YAML::Key << "intercept" << YAML::Value << calib.intercept;
  out << YAML::Key << "squared_error" << YAML::Value << calib.squaredError;
  out << YAML::Key << "spectrometer_pose" << YAML::Value << calib.spectrometerPose;
  out << YAML::Key << "filter_profile" << YAML::Value << calib.filterProfile;
  out << YAML::EndMap;
  return out;
}
