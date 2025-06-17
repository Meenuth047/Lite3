//
// Created by biao on 24-11-1.
//

#include "RosToQnx.h"
#include <arpa/inet.h>

using std::placeholders::_1;

RosToQnx::RosToQnx() : Node("ros_to_qnx")
{
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(43893);
    addr_client.sin_addr.s_addr = inet_addr("192.168.1.120"); //mannual set

    vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 10, std::bind(&RosToQnx::velCallBack, this, _1));
}

void RosToQnx::velCallBack(geometry_msgs::msg::Twist::SharedPtr msg)
{
    DataSend data{};

    data.code = 320;
    data.size = 8;
    data.cons_code = 1;
    data.cmd_data = msg->linear.x; ///< linear velocity
    sendto(sock_fd_, &data, sizeof(data), 0,
           reinterpret_cast<sockaddr*>(&addr_client), sizeof(addr_client));

    data.code = 325;
    data.size = 8;
    data.cons_code = 1;
    data.cmd_data = msg->linear.y; ///< Lateral velocity
    sendto(sock_fd_, &data, sizeof(data), 0,
           reinterpret_cast<sockaddr*>(&addr_client), sizeof(addr_client));

    data.code = 321;
    data.size = 8;
    data.cons_code = 1;
    data.cmd_data = msg->angular.z; ///< angular velocity
    sendto(sock_fd_, &data, sizeof(data), 0,
           reinterpret_cast<sockaddr*>(&addr_client), sizeof(addr_client));
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RosToQnx>();
    spin(node);
    rclcpp::shutdown();
    return 0;
}
