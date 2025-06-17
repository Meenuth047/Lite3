//
// Created by biao on 24-11-1.
//

#include "QnxToRos.h"
#include <sys/socket.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>


#define PI 3.1415926

QnxToRos::QnxToRos(): Node("qnx_to_ros")
{
    declare_parameter("serv_port", serv_port_);
    get_parameter("serv_port", serv_port_);

    body_pose_pub_ = create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>("body_pose", 10);
    leg_odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("leg_odom", 10);
    joint_state_pub_ = create_publisher<sensor_msgs::msg::JointState>("joint_states", 10);
    imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("imu/data", 10);
    handle_pub_ = create_publisher<geometry_msgs::msg::Twist>("handle", 10);
    ultrasound_pub_ = create_publisher<std_msgs::msg::Float64>("ultrasound_distance", 10);

    odom_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    initUdp();

    timer_ = create_wall_timer(std::chrono::microseconds(100), std::bind(&QnxToRos::timer_callback, this));
}

void QnxToRos::initUdp()
{
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        exit(1);
    }
    memset(&addr_serv, 0, sizeof(sockaddr_in)); ///< initialize to zeros
    addr_serv.sin_family = AF_INET; ///< host byte order
    addr_serv.sin_port = htons(serv_port_); ///< port in network byte order
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY); ///< automatically fill in my IP
    len = sizeof(addr_serv);
    if (bind(sock_fd, reinterpret_cast<sockaddr*>(&addr_serv), sizeof(addr_serv)) < 0)
    {
        RCLCPP_ERROR(get_logger(), "bind error:");
        exit(1);
    }

    RCLCPP_INFO(get_logger(), "UDP server started, port: %d", serv_port_);
}

void QnxToRos::timer_callback()
{
    if ((recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, reinterpret_cast<sockaddr*>(&addr_serv),
                             reinterpret_cast<socklen_t*>(&len))) < 0)
    {
        RCLCPP_ERROR(get_logger(), "recvfrom error:");
        exit(1);
    }

    // RCLCPP_INFO(get_logger(), "recv_num: %ld", recv_num);
    // RCLCPP_INFO(get_logger(), "RobotStateReceived: %ld", sizeof(RobotStateReceived));

    if (recv_num == sizeof(RobotStateReceived))
    {
        auto* dr = reinterpret_cast<RobotStateReceived*>(recv_buf);
        RobotState* robot_state = &dr->data;

        if (dr->code == 2305)
        {
            rclcpp::Time current_time = now();
            geometry_msgs::msg::PoseWithCovarianceStamped leg_odom_data;
            leg_odom_data.header.frame_id = "odom";
            leg_odom_data.header.stamp = current_time;

            tf2::Quaternion quaternion;
            quaternion.setRPY(0, 0, robot_state->rpy[2] / 180 * PI);
            leg_odom_data.pose.pose.orientation =
                toMsg(quaternion);

            leg_odom_data.pose.pose.position.x = robot_state->pos_world[0];
            leg_odom_data.pose.pose.position.y = robot_state->pos_world[1];
            leg_odom_data.pose.pose.position.z = robot_state->pos_world[2];
            body_pose_pub_->publish(leg_odom_data);

            nav_msgs::msg::Odometry leg_odom_data2;
            leg_odom_data2.header = leg_odom_data.header;
            leg_odom_data2.child_frame_id = "base";
            leg_odom_data2.pose = leg_odom_data.pose;
            leg_odom_data2.twist.twist.linear.x = robot_state->vel_body[0];
            leg_odom_data2.twist.twist.linear.y = robot_state->vel_body[1];
            leg_odom_data2.twist.twist.angular.z = robot_state->rpy_vel[2];
            leg_odom_pub_->publish(leg_odom_data2);

            sensor_msgs::msg::Imu imu_msg;
            imu_msg.header.frame_id = "imu";
            imu_msg.header.stamp = current_time;

            quaternion.setRPY(robot_state->rpy[0] / 180 * PI,
                              robot_state->rpy[1] / 180 * PI,
                              robot_state->rpy[2] / 180 * PI);
            imu_msg.orientation = toMsg(quaternion);
            imu_msg.angular_velocity.x = robot_state->rpy_vel[0];
            imu_msg.angular_velocity.y = robot_state->rpy_vel[1];
            imu_msg.angular_velocity.z = robot_state->rpy_vel[2];
            imu_msg.linear_acceleration.x = robot_state->xyz_acc[0];
            imu_msg.linear_acceleration.y = robot_state->xyz_acc[1];
            imu_msg.linear_acceleration.z = robot_state->xyz_acc[2];
            imu_pub_->publish(imu_msg);

            std_msgs::msg::Float64 ultrasound_distance;
            ultrasound_distance.data = robot_state->ultrasound[1];
            ultrasound_pub_->publish(ultrasound_distance);

            geometry_msgs::msg::TransformStamped leg_odom_trans;
            leg_odom_trans.header.stamp = current_time;
            leg_odom_trans.header.frame_id = "odom";
            leg_odom_trans.child_frame_id = "base";
            leg_odom_trans.transform.translation.x = leg_odom_data.pose.pose.position.x;
            leg_odom_trans.transform.translation.y = leg_odom_data.pose.pose.position.y;
            leg_odom_trans.transform.translation.z = leg_odom_data.pose.pose.position.z;
            leg_odom_trans.transform.rotation = imu_msg.orientation;
            odom_broadcaster_->sendTransform(leg_odom_trans);
        }
    } else if (recv_num == sizeof(JointStateReceived))
    {
        auto* dr = reinterpret_cast<JointStateReceived*>(recv_buf);
        JointState* joint_state = &dr->data;
        if (dr->code == 2306)
        {
            sensor_msgs::msg::JointState joint_state_data;
            joint_state_data.header.stamp = now();
            joint_state_data.name.resize(12);
            joint_state_data.position.resize(12);

            joint_state_data.name[0] = "FL_HipX";
            joint_state_data.position[0] = joint_state->LF_Joint;
            joint_state_data.name[1] = "FL_HipY";
            joint_state_data.position[1] = joint_state->LF_Joint_1;
            joint_state_data.name[2] = "FL_Knee";
            joint_state_data.position[2] = joint_state->LF_Joint_2;

            joint_state_data.name[3] = "FR_HipX";
            joint_state_data.position[3] = joint_state->RF_Joint;
            joint_state_data.name[4] = "FR_HipY";
            joint_state_data.position[4] = joint_state->RF_Joint_1;
            joint_state_data.name[5] = "FR_Knee";
            joint_state_data.position[5] = joint_state->RF_Joint_2;

            joint_state_data.name[6] = "HL_HipX";
            joint_state_data.position[6] = joint_state->LB_Joint;
            joint_state_data.name[7] = "HL_HipY";
            joint_state_data.position[7] = joint_state->LB_Joint_1;
            joint_state_data.name[8] = "HL_Knee";
            joint_state_data.position[8] = joint_state->LB_Joint_2;

            joint_state_data.name[9] = "HR_HipX";
            joint_state_data.position[9] = joint_state->RB_Joint;
            joint_state_data.name[10] = "HR_HipY";
            joint_state_data.position[10] = joint_state->RB_Joint_1;
            joint_state_data.name[11] = "HR_Knee";
            joint_state_data.position[11] = joint_state->RB_Joint_2;
            joint_state_pub_->publish(joint_state_data);
        }
    } else if (recv_num == sizeof(handleStateReceived))
    {
        auto* dr = reinterpret_cast<handleStateReceived*>(recv_buf);
        handleState* handle_state = &dr->data;
        if (dr->code == 2309)
        {
            geometry_msgs::msg::Twist handle_state_msg;
            handle_state_msg.linear.x = handle_state->left_axis_forward;
            handle_state_msg.linear.y = handle_state->left_axis_side;
            handle_state_msg.angular.z = -handle_state->right_axis_yaw;
            handle_pub_->publish(handle_state_msg);
        }
    }
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<QnxToRos>();
    spin(node);
    rclcpp::shutdown();
    return 0;
}
