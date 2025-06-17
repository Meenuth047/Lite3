#!/usr/bin/python3
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import LifecycleNode
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import ThisLaunchFileDir,LaunchConfiguration
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition


def generate_launch_description():
    driver_dir = os.path.join(get_package_share_directory('lite3_description'), 'config', 'lidar', 'lslidar_cx.yaml')
    rviz_dir = os.path.join(get_package_share_directory('lite3_description'), 'config', 'lidar', 'lslidar_cx.rviz')
    rviz_use = LaunchConfiguration('rviz')

    driver_node = LifecycleNode(package='lslidar_driver',
                                namespace='cx',
                                executable='lslidar_driver_node',
                                name='lslidar_driver_node',
                                output='screen',
                                emulate_tty=True,
                                parameters=[driver_dir],
                                )
    rviz_node = Node(
        package='rviz2',
        namespace='cx',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_dir],
        condition=IfCondition(rviz_use),
        output='screen')

    return LaunchDescription([
        DeclareLaunchArgument(
            'rviz',
            default_value='true',
            description='Whether to start rviz'
        ),
        driver_node,
        rviz_node,
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([ThisLaunchFileDir(), '/visualize.launch.py']),
            launch_arguments={'rviz': 'false'}.items(),
        ),
    ])
