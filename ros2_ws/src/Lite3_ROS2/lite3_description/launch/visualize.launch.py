import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, ThisLaunchFileDir
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import xacro

package_description = "lite3_description"


def process_xacro():
    pkg_path = os.path.join(get_package_share_directory(package_description))
    xacro_file = os.path.join(pkg_path, 'xacro', 'robot.xacro')
    robot_description_config = xacro.process_file(xacro_file)
    return robot_description_config.toxml()


def launch_setup(context, *args, **kwargs):
    rviz_config_file = os.path.join(get_package_share_directory(package_description), "config", "visualize_urdf.rviz")
    robot_description = process_xacro()

    nodes = [
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[
                {
                    'publish_frequency': 100.0,
                    'use_tf_static': True,
                    'robot_description': robot_description
                }
            ],
        )
    ]

    if LaunchConfiguration('rviz').perform(context) == 'true':
        nodes.append(Node(
            package='rviz2',
            executable='rviz2',
            name='rviz_ocs2',
            output='screen',
            arguments=["-d", rviz_config_file]
        ))

    if LaunchConfiguration('check_gui').perform(context) == 'true':
        nodes.append(Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher',
            output='screen'
        ))
    elif LaunchConfiguration('udp_bridge').perform(context) == 'true':
        nodes.append(IncludeLaunchDescription(
            PythonLaunchDescriptionSource([ThisLaunchFileDir(), '/bridge.launch.py']),
        ))

    return nodes


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'check_gui',
            default_value='false',
            description='Whether to start joint_state_publisher_gui'
        ),
        DeclareLaunchArgument(
            'rviz',
            default_value='true',
            description='Whether to start rviz'
        ),
        DeclareLaunchArgument(
            'udp_bridge',
            default_value='true',
            description='Whether to use udp_bridge'
        ),
        OpaqueFunction(function=launch_setup)
    ])
