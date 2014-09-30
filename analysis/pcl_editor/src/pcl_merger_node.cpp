#include "pcl_editor/pcl_merger_node.hpp"
#include <eigen3/Eigen/Geometry>
#include <pcl/common/transforms.h>
#include <tf2/LinearMath/Quaternion.h>

namespace pcl_editor {

using namespace pcl;

PclMergerNode::PclMergerNode(const ros::NodeHandle& nh)
    : nh_(nh),
      cloud1_(new MyPointCloud),
      cloud2_(new MyPointCloud),
      viewer_(new visualization::PCLVisualizer("3D Viewer")) {
  std::string pcd1;
  std::string pcd2;
  nh_.param<std::string>("pcd1", pcd1, std::string());
  nh_.param<std::string>("pcd2", pcd2, std::string());
  // Load two point cloud
  if (!(LoadPcdFile(pcd1, cloud1_) && LoadPcdFile(pcd2, cloud2_))) {
    throw std::runtime_error("Failed to load file");
  }
  viewer_->setBackgroundColor(0, 0, 0);
  viewer_->addPointCloud<PointWithViewpoint>(cloud1_, "cloud1");
  viewer_->setPointCloudRenderingProperties(
      visualization::PCL_VISUALIZER_POINT_SIZE, 1, "cloud1");
  viewer_->addCoordinateSystem(1.0);
  viewer_->initCameraParameters();
  cfg_server_.setCallback(boost::bind(&PclMergerNode::ConfigCb, this, _1, _2));
}

void PclMergerNode::ConfigCb(MergerDynConfig& config, int level) {
  // Initialize
  if (level < 0) {
    ROS_INFO("Initialize reconfigure server for %s",
             nh_.getNamespace().c_str());
    // Add cloud 2
    visualization::PointCloudColorHandlerCustom<MyPoint> single_color(
        cloud2_, 0, 255, 0);
    viewer_->addPointCloud<MyPoint>(cloud2_, single_color, "cloud2");
    viewer_->setPointCloudRenderingProperties(
        visualization::PCL_VISUALIZER_POINT_SIZE, 1, "cloud2");
    config.save = false;
  }

  // Transform cloud
  if (level < 1) {
    cloud_transformed_.reset(new MyPointCloud);
    Eigen::Vector3f t(config.x, config.y, config.z);
    tf2::Quaternion tf_q(config.yaw, config.pitch, config.roll);
    Eigen::Quaternionf q(tf_q.getW(), tf_q.getX(), tf_q.getY(), tf_q.getZ());

    transformPointCloud(*cloud2_, *cloud_transformed_, t, q);

    // Visualize the transformed cloud
    visualization::PointCloudColorHandlerCustom<MyPoint> single_color(
        cloud_transformed_, 0, 255, 0);
    viewer_->updatePointCloud<MyPoint>(cloud_transformed_, single_color,
                                       "cloud2");
  }

  // Save merged cloud
  if (level == 1) {
    if (config.save) {
      if (cloud_transformed_->empty()) return;
      // Merge two clouds
      MyPointCloud cloud_merged;
      cloud_merged = *cloud1_;
      cloud_merged += *cloud_transformed_;
      ROS_INFO("Merging cloud, %zu + %zu = %zu", cloud1_->size(),
               cloud_transformed_->size(), cloud_merged.size());
      try {
        pcl::io::savePCDFile(config.pcd_merged, cloud_merged);
        ROS_INFO("Saved merged cloud to: %s", config.pcd_merged.c_str());
      }
      catch (const std::exception& e) {
        ROS_ERROR("%s: %s", nh_.getNamespace().c_str(), e.what());
      }
      config.save = false;
    }
  }
}

}  // namespace pcl_editor
