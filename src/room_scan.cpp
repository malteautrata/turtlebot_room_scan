#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"

void callback();
ros::Publisher pub;
ros::Subscriber sub;


void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    std::cout << msg->ranges[0];

    geometry_msgs::Twist twist;

    if(msg->ranges[0] <= 0.3)
    {
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 90.0;
    } else {
        twist.linear.x = 0.5; twist.linear.y = 0.0; twist.linear.z = 0.0;
    }
    pub.publish(twist);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "drive_random");
    ros::NodeHandle n;
    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    sub = n.subscribe("/scan", 10, callback);
    ros::spin();
 }
