import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import ThisLaunchFileDir
from launch_ros.actions import Node

home_dir = os.path.expanduser("~")
foldername = os.path.join(home_dir, "maps")
if not os.path.exists(foldername):
    os.makedirs(foldername)


def generate_launch_description():
    return LaunchDescription([

        Node(
            package='amr_rctk',
            executable='pose_publisher',
            name='pose_publisher',
            namespace='amr_rctk',
            parameters=[{'source_frame': 'base'}],
        ),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([ThisLaunchFileDir(), '/lslidar.launch.py']),
            launch_arguments={'rviz': 'false'}.items(),
        ),

        Node(
            package='amr_rctk',
            executable='mapping_node.py',
            name='mapping_node',
            output='screen',
            parameters=[
                {'foldername': foldername},
                {'navigation_command': 'lite3_description nav2.launch.py'},
                {'start_mapping_command': 'ros2 launch lite3_description slam_cartographer.launch.py'},
                {'save_map_command': 'ros2 run nav2_map_server map_saver_cli -f'}
            ]
        )
    ])
