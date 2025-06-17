import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription

from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time', default='false')
    package_path = get_package_share_directory('lite3_description')

    config_dir = os.path.join(package_path, 'config', 'slam_toolbox', 'slam.yaml')
    rviz_config_dir = os.path.join(package_path, 'config', 'slam_toolbox', 'slam_toolbox.rviz')

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'),
        Node(package='slam_toolbox',
             executable='async_slam_toolbox_node',
             name='slam_toolbox',
             output='screen',
             parameters=[config_dir, {'use_sim_time': use_sim_time}], ),
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            arguments=['-d', rviz_config_dir],
            output='screen'),
    ])
