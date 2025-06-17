//
// Created by biao on 24-11-1.
//

#ifndef QNXTOROS_H
#define QNXTOROS_H

#include <netinet/in.h>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_with_covariance_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/float64.hpp"

#pragma pack(4)
struct RobotState
{
    int robot_basic_state; ///< Basic motion state of robot
    int robot_gait_state; ///< Robot gait information
    double rpy[3]; ///< IMU angular
    double rpy_vel[3]; ///< IMU angular velocity
    double xyz_acc[3]; ///< IMU acceleration
    double pos_world[3]; ///< Position of robot in world coordinate system
    double vel_world[3]; ///< The speed of robot in the world coordinate system
    double vel_body[3]; ///< Speed of robot in body coordinate system
    unsigned touch_down_and_stair_trot;
    ///< This function has not been activated for the time being. This data is only used to occupy the position
    bool is_charging; ///< Not opened temporarily
    unsigned error_state; ///< Not opened temporarily
    int robot_motion_state; ///< Robot action status
    double battery_level; ///< Battery Percentage
    int task_state; ///< Not opened temporarily
    bool is_robot_need_move; ///< When the robot is standing in place, whether it is pushed to switch into motion mode
    bool zero_position_flag; ///< Zero return flag bit
    double ultrasound[2];
};

struct RobotStateReceived
{
    int code; ///< Command code
    int size; ///< Command value
    int cons_code; ///< Command type
    RobotState data;
};

struct JointState
{
    double LF_Joint;
    double LF_Joint_1;
    double LF_Joint_2;
    double RF_Joint;
    double RF_Joint_1;
    double RF_Joint_2;
    double LB_Joint;
    double LB_Joint_1;
    double LB_Joint_2;
    double RB_Joint;
    double RB_Joint_1;
    double RB_Joint_2;
};

struct JointStateReceived
{
    int code;
    int size;
    int cons_code;
    JointState data;
};

struct handleState
{
    double left_axis_forward; ///< Left rocker y-axis,        range: - 1~1
    double left_axis_side; ///< Left rocker x-axis,         range: - 1~1
    double right_axis_yaw; ///< right rocker y-axis,        range: - 1~1
    double goal_vel_forward; ///< Target linear speed in x direction
    double goal_vel_side; ///< Target linear speed in y direction
    double goal_vel_yaw; ///< Target Yaw angular velocity
};

struct handleStateReceived
{
    int code;
    int size;
    int cons_code;
    handleState data;
};


class QnxToRos final : public rclcpp::Node
{
public:
    QnxToRos();

    ~QnxToRos() override
    {
        close(sock_fd);
    }

private:
    void initUdp();
    void timer_callback();

    int serv_port_ = 43897;

    int sock_fd, len;
    long recv_num = -1;
    char recv_buf[500];
    sockaddr_in addr_serv{};

    rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr body_pose_pub_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr leg_odom_pub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr handle_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr ultrasound_pub_;

    std::unique_ptr<tf2_ros::TransformBroadcaster> odom_broadcaster_;

    rclcpp::TimerBase::SharedPtr timer_;
};


#endif //QNXTOROS_H
