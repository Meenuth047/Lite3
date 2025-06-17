#include <geometry_msgs/Twist.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Float64MultiArray.h>
#include <ros/ros.h>
#include <stdlib.h>
#include <ctime>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <chrono>
#include <iomanip>
#include <iostream>
using namespace std;

#pragma pack(4)
struct DataSend{
  int32_t code;
  int32_t size;
  int32_t cons_code;
  double cmd_data;
};

class ros2qnx
{
public:
  bool isdebug;
  DataSend data;    ///< cml_vel data
  int sock_fd_out = socket(AF_INET, SOCK_DGRAM,0);
  struct sockaddr_in addr_client;
  ros2qnx()
  {
    addr_client.sin_family = AF_INET;
    addr_client.sin_port = htons(43893);
    addr_client.sin_addr.s_addr = inet_addr("192.168.1.120");    //mannual set
  }
  int nbytes;
  void sendCallback(geometry_msgs::TwistConstPtr msg){
    //normal
    if(!isdebug)
    {
      data.code = 320;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->linear.x;                  ///< linear velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));

      data.code =325;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->linear.y;                 ///< Lateral velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));

      data.code = 321;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->angular.z;               ///< angular velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));
    }
    //fake vel
    else
    {
      data.code = 330;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->linear.x;                  ///< linear velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));

      data.code =335;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->linear.y;                 ///< Lateral velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));

      data.code = 331;
      data.size = 8;
      data.cons_code = 1;
      data.cmd_data = msg->angular.z;               ///< angular velocity
      nbytes = sendto(sock_fd_out, &data, sizeof(data), 0,
          (struct sockaddr *)&addr_client, sizeof(addr_client));
    }
  }
};

int main(int argc, char** argv) {
  ros::init(argc, argv, "ros2qnx");
  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");

  ros2qnx ros2qnx;
  bool lidar_param_ok = private_nh.param<bool>("/ros2qnx/isdebug", ros2qnx.isdebug, false);
  if(!lidar_param_ok)   {ROS_WARN("get param /ros2qnx/isdebug failed");}

  ROS_INFO("-----   ros2qnx node up   -----");
  if(ros2qnx.isdebug)  ROS_INFO("ros2qnx working in debug mode");
  else                ROS_INFO("ros2qnx working in normal mode");
  
  ros::Subscriber vel_sub = nh.subscribe("cmd_vel", 1, &ros2qnx::sendCallback, &ros2qnx);
  ros::Subscriber vel_sub2 = nh.subscribe("cmd_vel_corrected", 1, &ros2qnx::sendCallback, &ros2qnx);
    
  ros::spin();

  return 0;
}
