//
// Created by biao on 24-11-1.
//

#ifndef ROSTOQNX_H
#define ROSTOQNX_H

#include <netinet/in.h>
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>

#pragma pack(4)
struct DataSend{
    int32_t code;
    int32_t size;
    int32_t cons_code;
    double cmd_data;
};


class RosToQnx final : public rclcpp::Node{
public:
    RosToQnx();

    ~RosToQnx() override
    {
        close(sock_fd_);
    }
private:
    void velCallBack(geometry_msgs::msg::Twist::SharedPtr msg);
    int sock_fd_ = socket(AF_INET, SOCK_DGRAM,0);
    sockaddr_in addr_client;

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr vel_sub_;
};



#endif //ROSTOQNX_H
