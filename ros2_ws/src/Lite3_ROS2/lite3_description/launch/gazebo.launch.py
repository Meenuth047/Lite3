from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os
import xacro

def generate_launch_description():
    pkg_name = 'lite3_description'
    pkg_path = get_package_share_directory(pkg_name)

    # Paths
    world_path = os.path.join(pkg_path, 'worlds', 'empty.world')
    xacro_file = os.path.join(pkg_path, 'xacro', 'robot.xacro')

    # Process XACRO to URDF
    robot_description_config = xacro.process_file(xacro_file)
    robot_description = robot_description_config.toxml()

    return LaunchDescription([
        # Launch Gazebo with custom world
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py')
            ),
            launch_arguments={'world': world_path}.items()
        ),

        # Publish robot state
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{'robot_description': robot_description}],
            output='screen'
        ),

        # Spawn robot into Gazebo
        Node(
            package='gazebo_ros',
            executable='spawn_entity.py',
            arguments=[
                '-entity', 'lite3_robot',
                '-topic', 'robot_description',
                '-x', '0', '-y', '0', '-z', '0.5'
            ],
            output='screen'
        ),
    ])




