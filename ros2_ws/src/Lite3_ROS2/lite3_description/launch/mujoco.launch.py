from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch.substitutions import PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.conditions import IfCondition

def generate_launch_description():

    # Launch arguments
    urdf_file_arg = DeclareLaunchArgument(
        name='urdf_file',
        default_value='robot.urdf',
        description='Path to the URDF file of the robot.'
    )

    use_gui_arg = DeclareLaunchArgument(
        name='use_gui',
        default_value='true',
        description='Whether to show the MuJoCo GUI.'
    )

    # Path to the URDF file
    urdf_path = LaunchConfiguration('urdf_file')
    use_gui = LaunchConfiguration('use_gui')

    # Node: Convert URDF → MJCF and run simulation
    mujoco_node = Node(
        package='mujoco_ros',
        executable='mujoco_node',
        name='mujoco_sim',
        parameters=[{
            'robot_description': PathJoinSubstitution([
                FindPackageShare('lite3_description'),
                'urdf',
                urdf_path
            ]),
            'use_gui': use_gui,
        }],
        output='screen'
    )

    return LaunchDescription([
        urdf_file_arg,
        use_gui_arg,
        mujoco_node
    ])
