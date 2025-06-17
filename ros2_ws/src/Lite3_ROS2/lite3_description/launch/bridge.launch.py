from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='lite3_udp_bridge',
            executable='qnx2ros',
            name='qnx2ros',
            parameters=[
                {'SERV_PORT': 43897},
            ]
        ),
        Node(
            package='lite3_udp_bridge',
            executable='ros2qnx',
            name='ros2qnx',
        ),
        Node(
            package='rosbridge_server',
            executable='rosbridge_websocket',
            name='rosbridge_websocket'
        ),
    ])