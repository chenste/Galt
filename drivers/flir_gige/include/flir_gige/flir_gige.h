#ifndef FLIR_GIGE_FLIR_GIGE_H_
#define FLIR_GIGE_FLIR_GIGE_H_

#include <memory>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>
#include <dynamic_reconfigure/server.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/core/core.hpp>

#include "flir_gige/gige_camera.h"
#include "flir_gige/FlirConfig.h"

namespace flir_gige {

class FlirNode {
 private:
  // ROS related
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Publisher image_pub_;
  sensor_msgs::ImagePtr image_msg_;
  dynamic_reconfigure::Server<flir_gige::FlirConfig> server_;
  ros::Rate rate_;
  std::string frame_id_;
  unsigned seq = 0;
  // Flir Camera
  std::shared_ptr<flir_gige::GigeCamera> camera_;

 public:
  FlirNode(const ros::NodeHandle &nh, const double fps);

  FlirNode(const FlirNode &) = delete;             // No copy constructor
  FlirNode &operator=(const FlirNode &) = delete;  // No assignment operator

  void Init();
  void PublishImage(const cv::Mat &image);
  void ReconfigureCallback(flir_gige::FlirConfig &config, int level);

};  // class FlirNode

}  // namespace flir_gige

#endif  // FLIR_GIGE_FLIR_GIGE_H_
