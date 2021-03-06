#include "spectral_meter/node.hpp"

#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <dynamic_reconfigure/Config.h>
#include <dynamic_reconfigure/Reconfigure.h>

#include <cmath>

namespace galt {
namespace spectral_meter {

Node::Node(const ros::NodeHandle& nh, const ros::NodeHandle& pnh)
    : nh_(nh), pnh_(pnh), it_(pnh), cfg_server_(pnh), offset_(0, 0) {
  cfg_server_.setCallback(boost::bind(&Node::configCallback, this, _1, _2));
  bool use_tracker;
  pnh_.param("use_tracker", use_tracker, false);
  if (use_tracker) {
    tracker_ = boost::make_shared<Tracker>(ros::NodeHandle(pnh_, "tracker"));
  }
}

void Node::configure() {
  image_transport::TransportHints hints("raw", ros::TransportHints(), pnh_);
  const auto resolved_image = pnh_.resolveName("image");
  ROS_WARN("Subscribing to: %s, transport: %s", resolved_image.c_str(),
           hints.getTransport().c_str());
  // We need a fully resolved name here to subscribe to transport correctly
  sub_image_ =
      it_.subscribe(resolved_image, 1, &Node::imageCallback, this, hints);
  ROS_WARN("Subscribed to: %s, transport: %s", sub_image_.getTopic().c_str(),
           sub_image_.getTransport().c_str());
  pnh_.param("ui_scale", ui_scale_, 0.5);

  //  retrieve current camera exposure
  camera_topic_name_ = nh_.getNamespace();
  expose_rosparam_name_ = nh_.resolveName("expose_us");
  if (!nh_.getParamCached(expose_rosparam_name_, expose_us_)) {
    std::string err = "Failed to retrieve param " + expose_rosparam_name_;
    throw std::runtime_error(err);
  }

  cv::namedWindow(camera_topic_name_, cv::WINDOW_AUTOSIZE);
  cv::setMouseCallback(camera_topic_name_, &Node::mouseCallbackStatic,
                       reinterpret_cast<void*>(this));

  ROS_INFO("Spectral Meter 9000 - The Ultimate in Exposure Calibration");
  ROS_INFO("\tConfiguring camera: %s", expose_rosparam_name_.c_str());
  ROS_INFO("\tReflectance: %f", config_.target_reflectance);
  ROS_INFO("\tSelection size: %i", config_.selection_size);
}

void Node::configCallback(Config& config, int level) {
  if (level < 0) {
    ROS_INFO("%s: Initializaing reconfigure server",
             pnh_.getNamespace().c_str());
  }
  config_ = config;
}

void Node::imageCallback(const sensor_msgs::ImageConstPtr& img) {
  cv_bridge::CvImageConstPtr bridged = cv_bridge::toCvShare(img, "mono8");
  if (!bridged || bridged->image.empty()) {
    ROS_ERROR_ONCE_NAMED("convert_error",
                         "Failed to convert image to mono8 w/ cv_bridge");
    return;
  }

  cv::Mat input_image = bridged->image;
  const cv::Size size(input_image.cols, input_image.rows);
  const cv::Size ui_size(size.width * ui_scale_, size.height * ui_scale_);

  //  create image for user interface (scale and conert to color)
  cv::Mat ui_image;
  cv::resize(input_image, ui_image, ui_size);
  cv::cvtColor(ui_image, ui_image, CV_GRAY2RGB);

  if (tracker_) {
    // Track features across image
    tracker_->step(ui_image);
    offset_ = tracker_->calcOffset();
  }

  click_position_ += offset_;

  //  draw target position
  {
    //  clamp to at least one pixel
    const double box_size = std::max(config_.selection_size * ui_scale_, 1.0);
    drawSelection(ui_image, click_position_, box_size);

    //  draw measured and target reflectance
    drawPercentage(ui_image, click_position_, measured_reflectance_,
                   config_.target_reflectance);

    // draw exposure value
    drawExposeUs(ui_image, {20, 50}, expose_us_);
  }

  //  adjust camera
  if (position_updated_ && (num_skip_frames_--) <= 0) {
    //  read the position from the image
    const cv::Size scaled_pos(click_position_.x / ui_scale_,
                              click_position_.y / ui_scale_);
    const cv::Rect sel_rect =
        createRectAround(scaled_pos, config_.selection_size, size);

    if (sel_rect.area() > 0) {
      //  crop out region selected by user
      const cv::Mat region = input_image(sel_rect);
      //  calculate min/max and average
      double min, max;
      cv::minMaxLoc(region, &min, &max);
      const cv::Scalar scal_mean = cv::mean(region);
      const double mean = scal_mean[0];

      //      ROS_INFO("Selection min/max/mean: %f/%f/%f", min, max, mean);
      //  scale units into 'reflectance'
      measured_reflectance_ = mean / 255.0;
      updateExposure(measured_reflectance_);
    }
    // position_updated_ = false;
  }

  //  draw output
  cv::imshow(camera_topic_name_, ui_image);

  //  refresh UI
  cv::waitKey(1);
}

void Node::callDynamicReconfigure(int expose_us) {
  //  send to dynamic reconfigure
  dynamic_reconfigure::ReconfigureRequest req;
  dynamic_reconfigure::ReconfigureResponse resp;
  dynamic_reconfigure::IntParameter int_param;
  dynamic_reconfigure::Config config;

  int_param.name = "expose_us";
  int_param.value = expose_us;
  config.ints.push_back(int_param);
  req.config = config;
  if (!ros::service::call(camera_topic_name_ + "/set_parameters", req, resp)) {
    ROS_WARN("ros::service::call failed while updating exposure");
  }
}

void Node::updateExposure(double measured_mean) {
  nh_.getParamCached(expose_rosparam_name_, expose_us_);

  const double err = config_.target_reflectance - measured_mean;
  // Clamp inc to some reasonable number to prevent overshoot
  int inc = config_.kp * err;
  int max_inc = 100;
  if (inc > max_inc) {
    inc = max_inc;
  } else if (inc < -max_inc) {
    inc = -max_inc;
  }
  // ROS_INFO("Increment: %i", inc);
  if (std::abs(inc) < 100) {
    //  stop adjusting
    inc = 0;
  }
  //  calculate commanded exposure
  const int expose_us = expose_us_ + inc;

  //  ROS_INFO("Desired: %f, Measured: %f", config_.target_reflectance,
  //           measured_mean);
  //  ROS_INFO("Current: %i, Commanding: %i", expose_us_, expose_us);
  if (expose_us_ != expose_us) {
    callDynamicReconfigure(expose_us);
  }
  num_skip_frames_ = config_.skip_frames;
}

void Node::mouseCallback(int event, int x, int y, int flags, void*) {
  if (event == cv::EVENT_LBUTTONDOWN) {
    click_position_ = cv::Point2i(x, y);
    position_updated_ = true;
  } else if (event == cv::EVENT_RBUTTONDOWN) {
    ROS_WARN("Camera %s's exposure is set to %d", camera_topic_name_.c_str(),
             expose_us_);
    ros::shutdown();
  } else if (event == cv::EVENT_MBUTTONDOWN) {
  } else if (event == cv::EVENT_MOUSEMOVE) {
  }
}

void Node::mouseCallbackStatic(int event, int x, int y, int flags,
                               void* userdata) {
  //  pass to instance
  Node* self = reinterpret_cast<Node*>(userdata);
  self->mouseCallback(event, x, y, flags, NULL);
}

/// ================
/// Helper functions
/// ================
std::pair<std::string, bool> generatePercentageString(double num, double den) {
  const auto num_percentage = static_cast<int>(std::round(num * 100));
  const auto den_percentage = static_cast<int>(std::round(den * 100));
  const auto num_percentage_str = std::to_string(num_percentage);
  const auto den_percentage_str = std::to_string(den_percentage);
  return {num_percentage_str + "/" + den_percentage_str,
          num_percentage == den_percentage};
}

void drawPercentage(cv::Mat& image, const cv::Point& point, double num,
                    double den) {
  const auto percentage_str = generatePercentageString(num, den);
  const auto text_color =
      percentage_str.second ? CV_RGB(0, 255, 0) : CV_RGB(0, 0, 255);
  cv::putText(image, percentage_str.first, point, cv::FONT_HERSHEY_SIMPLEX, 1,
              text_color, 2, CV_AA);
}

void drawExposeUs(cv::Mat& image, const cv::Point& point, int expose_us) {
  cv::putText(image, std::to_string(expose_us), point, cv::FONT_HERSHEY_SIMPLEX,
              1, CV_RGB(255, 0, 0), 2, CV_AA);
}

void drawSelection(cv::Mat& image, const cv::Point2f& point, double box_size) {
  const auto im_size = image.size();
  const auto draw_rect = createRectAround(point, box_size, im_size);
  const int min_ret_x = draw_rect.x;
  const int max_ret_x = draw_rect.x + draw_rect.width;
  const int min_ret_y = draw_rect.y;
  const int max_ret_y = draw_rect.y + draw_rect.height;

  const cv::Scalar line_color(138, 43, 226);
  // vertical lines
  cv::line(image, cv::Point2f(point.x, 0), cv::Point2f(point.x, min_ret_y),
           line_color, 2);
  cv::line(image, cv::Point2f(point.x, max_ret_y),
           cv::Point2f(point.x, im_size.height - 1), line_color, 2);

  // horizontal lines
  cv::line(image, cv::Point2f(0, point.y), cv::Point2f(min_ret_x, point.y),
           line_color, 2);
  cv::line(image, cv::Point2f(max_ret_x, point.y),
           cv::Point2f(im_size.width - 1, point.y), line_color, 2);
  // central box
  cv::Rect rect(min_ret_x, min_ret_y, max_ret_x - min_ret_x,
                max_ret_y - min_ret_y);
  cv::rectangle(image, rect, line_color, 2);
}

cv::Rect createRectAround(const cv::Point2i& center, double size,
                          const cv::Size& im_size) {
  const int min_x = std::floor(std::max(0.0, center.x - size / 2));
  const int max_x =
      std::ceil(std::min(im_size.width - 1., center.x + size / 2));
  const int min_y = std::floor(std::max(0.0, center.y - size / 2));
  const int max_y =
      std::ceil(std::min(im_size.height - 1., center.y + size / 2));
  return cv::Rect(min_x, min_y, max_x - min_x, max_y - min_y);
}

}  // namespace spectral_meter
}  // namespace galt
