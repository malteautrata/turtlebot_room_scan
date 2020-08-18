#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/Odometry.h"

void callback();
ros::Publisher speedPub;
ros::Subscriber scanSub;
ros::Subscriber odomSub;


void callbackScan(const sensor_msgs::LaserScan::ConstPtr& msg)
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

void callbackOdom(const nav_msgs::Odometry::ConstPtr& odom)
{

}

int main(int argc, char** argv){
    ros::init(argc, argv, "room_scan");
    ros::NodeHandle n;
    speedPub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    scanSub = n.subscribe("/scan", 10, callbackScan);
    odomSub = n.subscribe("/odom", 10, callbackOdom);
    ros::spin();
 }
