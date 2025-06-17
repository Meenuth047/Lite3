#include <errno.h>
#include <geometry_msgs/Quaternion.h>
#include <geometry_msgs/TwistStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <netinet/in.h>
#include <ros/duration.h>
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/JointState.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include "std_msgs/Int32.h"
#include "std_msgs/Bool.h"
#include "std_msgs/Float64.h"
#include <geometry_msgs/Point.h>
#include "tf/transform_datatypes.h"
#include <tf/transform_broadcaster.h>
#include <fstream>

using namespace std;

struct FunctionState
{
  bool obstacle_avoidance = false;
  //TODO:
  bool people_tracking = false;
  bool slam = false;
  bool navigation = false;
};

struct AppCommand
{
  int code;   ///< Instruction Code
  int size;   ///< Instruction Value
  int cons_code;  ///< Instruction type
};

class SensorLogger
{
public:
  bool isdebug;

  bool imu_isalive = false;
  void imu_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    imu_isalive = msg->data;
    if(isdebug)   {ROS_INFO("imu stat is [%i]",imu_isalive);}
  }

  bool odom_isalive = false;
  void odom_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    odom_isalive = msg->data;
    if(isdebug)   {ROS_INFO("odom stat is [%i]",odom_isalive);}
  }

  bool odom2_isalive = false;
  void odom2_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    odom2_isalive = msg->data;
    if(isdebug)   {ROS_INFO("odom2 stat is [%i]",odom2_isalive);}
  }

  bool joint_isalive = false;
  void joint_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    joint_isalive = msg->data;
    if(isdebug)   {ROS_INFO("joint stat is [%i]",joint_isalive);}
  }

  bool realsense_isalive = false;
  void realsense_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    realsense_isalive = msg->data;
    if(isdebug)   {ROS_INFO("realsense stat is [%i]",realsense_isalive);}
  }

  bool lidar_isalive = false;
  void lidar_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    lidar_isalive = msg->data;
    if(isdebug)   {ROS_INFO("lidar stat is [%i]",lidar_isalive);}
  }

  bool ultrasound_isalive = false;
  void ultrasound_stat_set(const std_msgs::Bool::ConstPtr& msg)
  {
    ultrasound_isalive = msg->data;
    if(isdebug)   {ROS_INFO("ultrasound stat is [%i]",ultrasound_isalive);}
  }
};

int main(int argc, char **argv) {
  //program init
  ros::init(argc, argv, "nx2app");
  ros::NodeHandle nh;
  ros::NodeHandle private_nh("~");
  int server_port;       
  int default_port = 43899;       
  private_nh.param<int>("server_port", server_port, default_port);
  ROS_INFO("nx2app node start");
  ros::Rate loop_rate(100);

  //sensorLogger init
  SensorLogger sensorLogger;
  ros::Subscriber imu_sub = nh.subscribe("/sensor_status/imu_isalive", 10, &SensorLogger::imu_stat_set, &sensorLogger);
  ros::Subscriber odom_sub = nh.subscribe("/sensor_status/odom_isalive", 10, &SensorLogger::odom_stat_set, &sensorLogger);
  ros::Subscriber odom2_sub = nh.subscribe("/sensor_status/odom2_isalive", 10, &SensorLogger::odom2_stat_set, &sensorLogger);
  ros::Subscriber joint_sub = nh.subscribe("/sensor_status/joint_isalive", 10, &SensorLogger::joint_stat_set, &sensorLogger);
  ros::Subscriber realsense_sub = nh.subscribe("/sensor_status/realsense_isalive", 10, &SensorLogger::realsense_stat_set, &sensorLogger);
  ros::Subscriber lidar_sub = nh.subscribe("/sensor_status/lidar_isalive", 10, &SensorLogger::lidar_stat_set, &sensorLogger);
  ros::Subscriber ultrasound_sub = nh.subscribe("/sensor_status/ultrasound_isalive", 10, &SensorLogger::ultrasound_stat_set, &sensorLogger);
  bool isdebug_param_ok = nh.param<bool>("/nx2app/isdebug", sensorLogger.isdebug, false);
  if(!isdebug_param_ok)    {ROS_WARN("get param /nx2app/isdebug failed");}

  //FunctionState init
  FunctionState functionState;

  //socket_listen init    
  //listen to any host who sends to 43899 port
  int socket_listen = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in listen_addr;
  memset(&listen_addr, 0, sizeof(struct sockaddr_in));
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(server_port);
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(socket_listen, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0;

  //socket_send init
  //send to 43897 port of the listened host
  int socket_send = socket(AF_INET, SOCK_DGRAM,0);
  struct sockaddr_in send_addr;
  send_addr.sin_family = AF_INET;
  send_addr.sin_port = htons(43897);

  //log init
  ofstream outfile;
  outfile.open("/home/ysc/message_transformer_ws/nx2app_log.txt");
  outfile << "start logging" << endl;
  
  int recv_num = -1;
  char recv_buf[500];
  int system_return;
  char *fgets_return;
  while(ros::ok()){
    //recvfrom will block if no udp package received.
    int len = sizeof(listen_addr);
    if((recv_num = recvfrom(socket_listen, recv_buf, sizeof(recv_buf), 0,(struct sockaddr *)&listen_addr,(socklen_t *)&len)) < 0) 
    {
      perror("recvfrom error:");
      exit(1);
    }

    //immediately update information(such as sensor status), when udp package received.
    ros::spinOnce();

    //only logs when the sensor is down
    outfile << "-----new round-----" << endl;
    if(!sensorLogger.imu_isalive)         outfile << "imu is down"        << endl;
    if(!sensorLogger.odom_isalive)        outfile << "odom is down"       << endl;
    if(!sensorLogger.odom2_isalive)       outfile << "odom2 is down"      << endl;
    if(!sensorLogger.joint_isalive)       outfile << "joint is down"      << endl;
    if(!sensorLogger.realsense_isalive)   outfile << "realsense is down"  << endl;
    if(!sensorLogger.lidar_isalive)       outfile << "lidar is down"      << endl;
    if(!sensorLogger.ultrasound_isalive)  outfile << "ultrasound is down" << endl;

    if(recv_num == sizeof(AppCommand))
    {
      AppCommand *dr = (AppCommand *)(recv_buf);
      if (dr->code == 0x21012109)
      {
        //start obstacle avoidance
        if (dr->size==0x40 && functionState.obstacle_avoidance == false)   
        {   
          if(sensorLogger.imu_isalive)
          {
            outfile << "start obstacle avoidance" << endl;
            system_return = system("systemctl start voa.service &");
            functionState.obstacle_avoidance = true; 
          }
          else
          {
            outfile << "imu not ready, obstacle avoidance not started." << endl;
          }
        }
        //Turn off all ai functions
        if (dr->size==0x00)
        { 
          outfile << "kill all functions" << endl;
          system_return = system("systemctl stop voa.service");
          system_return = system("ps aux | grep -e realsen* | grep -v grep | awk '{print $2}' | xargs -i kill -9 {}");
          system_return = system("ps aux | grep -e height_map* | grep -v grep | awk '{print $2}' | xargs -i kill -9 {}");
          functionState.obstacle_avoidance = false;
        }
      }

      //Inquire obstacle avoidance state
      if (dr->code == 0x2101210D)
      {
        char outBuf[7]="";
        FILE *fp = popen("systemctl is-active voa.service", "r");
        if (fp)
        {
            fgets_return = fgets(outBuf, 7, fp);
            pclose(fp);
        }
        AppCommand message;
        if(memcmp(outBuf,"active",6)==0)  
        { 
          outfile << "voa active" << endl;

          message.code = 0x2101210D;
          message.size = 0x11;
          message.cons_code = 0;
          send_addr.sin_addr.s_addr = listen_addr.sin_addr.s_addr;
          ssize_t nbytes = sendto(socket_send, &message, sizeof(message), 0,
                (struct sockaddr *)&send_addr, sizeof(send_addr));
          outfile << "bytes send" << nbytes << endl;
        }
        else
        {
          outfile << "voa not active" << endl;

          message.code = 0x2101210D;
          message.size = 0x10;
          message.cons_code = 0;
          send_addr.sin_addr.s_addr = listen_addr.sin_addr.s_addr;
          ssize_t nbytes = sendto(socket_send, &message, sizeof(message), 0,
                (struct sockaddr *)&send_addr, sizeof(send_addr));
          outfile << "bytes send" << nbytes << endl;
        }
      }         
    }
    loop_rate.sleep();
  }
  close(socket_listen);
  outfile.close();
  return 0;
}
