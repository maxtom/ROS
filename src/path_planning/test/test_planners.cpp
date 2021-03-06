/*
 * Copyright (c) 2015, The Regents of the University of California (Regents).
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
 * Authors: David Fridovich-Keil   ( dfk@eecs.berkeley.edu )
 */

#include <path_planning/geometry/trajectory_2d.h>
#include <path_planning/geometry/point_2d.h>
#include <path_planning/planning/rrt_planner_2d.h>
#include <path_planning/robot/robot_2d_circular.h>
#include <utils/math/random_generator.h>
#include <path_planning/scene/scene_2d_continuous.h>
#include <path_planning/scene/obstacle_2d.h>
#include <utils/image/image.h>
#include <utils/types/types.h>

#include <vector>
#include <cmath>
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <iostream>

#define VISUALIZE true

// Test that we can plan a RRT path in 2D.
TEST(RRTPlanner2D, TestRRTPlanner2D) {
  math::RandomGenerator rng(math::RandomGenerator::Seed());

  // Create a bunch of obstacles.
  std::vector<Obstacle2D::Ptr> obstacles;
  for (size_t ii = 0; ii < 200; ii++) {
    float x = rng.Double();
    float y = rng.Double();
    float radius = static_cast<float>(rng.DoubleUniform(0.01, 0.02));

    Obstacle2D::Ptr obstacle = Obstacle2D::Create(x, y, radius);
    obstacles.push_back(obstacle);
  }

  // Create a 2D continous scene.
  Scene2DContinuous scene(0.0, 1.0, 0.0, 1.0, obstacles);

  // Create a robot.
  Robot2DCircular robot(scene, 0.01);

  // Choose origin/goal.
  Point2D::Ptr origin, goal;
  while (!origin) {
    float x = static_cast<float>(rng.Double());
    float y = static_cast<float>(rng.Double());
    Point2D::Ptr point = Point2D::Create(x, y);

    if (robot.IsFeasible(point))
      origin = point;
  }
  while (!goal) {
    float x = static_cast<float>(rng.Double());
    float y = static_cast<float>(rng.Double());
    Point2D::Ptr point = Point2D::Create(x, y);

    if (robot.IsFeasible(point) &&
        Point2D::DistancePointToPoint(origin, point) > 0.4)
      goal = point;
  }

  // Plan a route.
  RRTPlanner2D planner(robot, scene, origin, goal, 0.05);
  Trajectory2D::Ptr route = planner.PlanTrajectory();

  // If visualize flag is set, query a grid and show the cost map.
  if (VISUALIZE) {
    scene.Visualize("RRT route", route);
  }
}

// Test that we can construct and destroy a scene.
TEST(RRTPlanner2D, TestOptimizePath2D) {
  math::RandomGenerator rng(math::RandomGenerator::Seed());

  // Create a bunch of obstacles.
  std::vector<Obstacle2D::Ptr> obstacles;
  for (size_t ii = 0; ii < 100; ii++) {
    float x = rng.Double();
    float y = rng.Double();
    float sigma_xx = 0.005 * rng.DoubleUniform(0.25, 0.75);
    float sigma_yy = 0.005 * rng.DoubleUniform(0.25, 0.75);
    float sigma_xy = rng.Double() *
      std::sqrt(sigma_xx * sigma_yy);

    Obstacle2D::Ptr obstacle =
      Obstacle2D::Create(x, y, sigma_xx, sigma_yy, sigma_xy, 3.0);
    obstacles.push_back(obstacle);
  }

  // Create a 2D continous scene.
  Scene2DContinuous scene(0.0, 1.0, 0.0, 1.0, obstacles);

  // Create a robot.
  Robot2DCircular robot(scene, 0.005);

  // Choose origin/goal.
  Point2D::Ptr origin, goal;
  while (!origin) {
    float x = static_cast<float>(rng.Double());
    float y = static_cast<float>(rng.Double());
    Point2D::Ptr point = Point2D::Create(x, y);

    if (robot.IsFeasible(point))
      origin = point;
  }
  while (!goal) {
    float x = static_cast<float>(rng.Double());
    float y = static_cast<float>(rng.Double());
    Point2D::Ptr point = Point2D::Create(x, y);

    if (robot.IsFeasible(point) &&
        Point2D::DistancePointToPoint(origin, point) > 0.8)
      goal = point;
  }

  // Plan a route.
  RRTPlanner2D planner(robot, scene, origin, goal, 0.05);
  Trajectory2D::Ptr route = planner.PlanTrajectory();
  route->Upsample(4);

  // If visualize flag is set, query a grid and show the cost map.
  if (VISUALIZE) {
    scene.Visualize("RRT route", route);
  }

  // Optimize path.
  Trajectory2D::Ptr optimized_path =
    scene.OptimizeTrajectory(route, 1e-3, 1e6, 1e-3, 5e-4, 1000);

  // If visualize flag is set, query a grid and show the cost map.
  if (VISUALIZE) {
    scene.Visualize("Smoothed route", optimized_path);
  }
}

int main(int argc, char** argv) {
  std::string log_file = GENERATED_TEST_DATA_DIR + std::string("/out.log");
  google::SetLogDestination(0, log_file.c_str());
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 1;
  google::InitGoogleLogging(argv[0]);
  ::testing::InitGoogleTest(&argc, argv);
  LOG(INFO) << "Running all tests.";
  return RUN_ALL_TESTS();
}
