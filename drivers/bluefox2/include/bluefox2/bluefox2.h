#ifndef BLUEFOX2_CAMERA_H_
#define BLUEFOX2_CAMERA_H_

#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <mvIMPACT_CPP/mvIMPACT_acquire.h>

#define LABEL ("\033[0;34m[BLFOX] \033[0m")
#define btoa(x) ((x) ? "true" : "false")
#define PRESS_A_KEY getchar();
#define TIMEOUT_MS (300)
#define DISP(name, val) (std::cout << LABEL << name << val << std::endl)
using std::string;
using std::vector;
using namespace mvIMPACT::acquire;

namespace bluefox2 {
// Represents a set of camera parameters
typedef struct mv_params {
  bool color;
  bool hdr;
  bool inverted;
  bool binning;
  bool auto_expose;
  int expose_us;
  int height;
  int width;
  int fps;
  double gain;
  string mode;
  string white_balance;
} mv_params_t;

// Represents image data
typedef struct mv_image {
  int height;
  int width;
  int channel;
  int step;
  vector<unsigned char> data;
} mv_image_t;

// Camera class
class Camera {
 public:
  /**
   * @brief Constructor
   *
   * @param serial Serial number of camera
   * @param mv_params A struct collects all user settings
   */
  Camera(string serial, mv_params_t mv_params);

  /**
   * @brief Destructor
   */
  ~Camera();

  /**
   * @brief Initialize camera
   *
   * @param verbose True for printing detailed settings
   */
  void init(bool verbose);
  void printSettings() const;
  void printStats() const;
  void printDetails() const;

  /**
   * @brief Grab single image from device
   *
   * @param image A mv image
   * @return True for successful
   */
  bool grabImage(mv_image_t &image);

  bool ok() const { return ok_; }
  int device_count() const { return device_count_; };
  int fps() const { return params_.fps; };

 private:
  int device_count_;
  int id_;
  bool ok_;
  const string serial_;
  const mv_params_t params_;

  DeviceManager device_manager_;
  Device *device_;
  FunctionInterface *func_interface_;
  Statistics *stats_;
  Request *request_;
  SettingsBlueFOX *bf_settings_;
  SystemSettings *sys_settings_;

  int findDeviceId() const;
  bool open();
  void close();
  void applySettings();
  void setAoi(const int &width, const int &height);
  void printMvErrorMsg(const ImpactAcquireException &e,
                       const std::string header) const;
};  // class Camera

}   // namespace bluefox2

#endif  // BLUEFOX2_CAMERA_H_
