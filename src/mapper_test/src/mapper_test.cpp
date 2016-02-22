/*
 * Copyright (c) 2016, The Regents of the University of California (Regents).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *    3. Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Please contact the author(s) of this library if you have any questions.
 * Author: James Smith   ( james.smith@berkeley.edu )
 */

///////////////////////////////////////////////////////////////////////////////
//
// Start up a new MapperTest node.
//
///////////////////////////////////////////////////////////////////////////////
#include <mapper_test/mapper_test.h>
#include <iostream>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <opencv/cv.h>
#include <pcl/point_types.h>
#include <pcl_ros/point_cloud.h>

// Constructor/destructor.
MapperTest::MapperTest() {}
MapperTest::~MapperTest() {}

// Initialize.
bool MapperTest::Initialize(const ros::NodeHandle& n) {
  name_ = ros::names::append(n.getNamespace(), "mapper_test");

  if (!LoadParameters(n)) {
    ROS_ERROR("%s: Failed to load parameters.", name_.c_str());
    return false;
  }

  if (!RegisterCallbacks(n)) {
    ROS_ERROR("%s: Failed to register callbacks.", name_.c_str());
    return false;
  }

  std::cout << "Initialization complete." << std::endl;
  return true;
}

// Load parameters.
bool MapperTest::LoadParameters(const ros::NodeHandle& n) { return true; }

// Register callbacks.
bool MapperTest::RegisterCallbacks(const ros::NodeHandle& n) {
  ros::NodeHandle nl(n);

  // pub = nl.advertise<geometry_msgs::WHATEVER>("topic_name", "topic_hz",
  // false);
  depth_sub_ = nl.subscribe("/guidance/depth/images", 10, &MapperTest::DepthImageCallback, this);
  depth_sub_ = nl.subscribe("/guidance/depth/imu", 10, &MapperTest::IMUCallback, this);
  cloud_pub_ = nl.advertise<pcl::PointCloud<pcl::PointXYZ> >("/mapper/cloud", 1);

  return true;
}

void MapperTest::DepthImageCallback(const guidance::multi_image::ConstPtr& msg) {
  // This will contain all new points from this update
  pcl::PointCloud<pcl::PointXYZ> cloud;

  for (auto const& img : msg->images) {
    cv_bridge::CvImagePtr cv_ptr;
    try {
      sensor_msgs::Image im = img.image;
      cv_ptr = cv_bridge::toCvCopy(im, sensor_msgs::image_encodings::MONO16);
    } catch (cv_bridge::Exception& e) {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

    cv::Mat depth8(320, 240, CV_8UC1);
    cv_ptr->image.convertTo(depth8, CV_8UC1);
    
    // Project depth8 using a path::Mapper and add to cloud
  }

  cloud_pub_.publish(cloud.makeShared());
}

void MapperTest::IMUCallback(const geometry_msgs::TransformStamped::ConstPtr& msg) {
  // keep track of a master position and rotation. Use these values when creating a camera
  // up in the DepthImageCallback
}