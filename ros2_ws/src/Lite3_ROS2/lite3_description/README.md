# DeepRobotics Lite3 Description

This repository contains the urdf model of lite3.

Tested environment:

* Ubuntu 24.04
    * ROS2 Jazzy

## Build

```bash
cd ~/ros2_ws
colcon build --packages-up-to lite3_description --symlink-install
```

## Visualize the robot

* To visualize and check the configuration of the robot in rviz, simply launch:

  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description visualize.launch.py check_gui:=true
  ```

* To visualize the robot with the actual joint states data, launch:
  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description visualize.launch.py
  ```

* To visualize the robot with lidar data, launch:
  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description lslidar.launch.py
  ```

## SLAM
* Cartographer
  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description slam_cartographer.launch.py
  ```
* Fast-LIO
  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description slam_fast_lio.launch.py
  ```

## Navigation
* AMR Remote Control Toolkit
  ```bash
  source ~/ros2_ws/install/setup.bash
  ros2 launch lite3_description amr_rctk.launch.py
  ```