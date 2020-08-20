#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/Odometry.h"

struct startPosition
{
    double x;
    double y;
};

void callback();
ros::Publisher speedPub;
ros::Subscriber scanSub;
ros::Subscriber odomSub;

double xPos;
double yPos;
startPosition startPos;
bool startReached = false;

void callbackScan(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    std::cout << msg->ranges[90]; //herausfinden was der Wert bei 90° entspricht
   
    geometry_msgs::Twist twist;

    if(msg->ranges[90] <= 0.3)  
    {
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 90.0;
    } else {
        if (yPos < startPos.y)
        {
            startPos.x = xPos;
            startPos.y = yPos;
        }
        twist.linear.x = 0.5; twist.linear.y = 0.0; twist.linear.z = 0.0;
    }
    speedPub.publish(twist);
}

void callbackOdom(const nav_msgs::Odometry::ConstPtr& odom)
{
    std::cout << odom->pose.pose.position.x;
    std::cout << odom->pose.pose.position.y;

    xPos = odom->pose.pose.position.x;
    yPos = odom->pose.pose.position.y;
}

int main(int argc, char** argv){
    ros::init(argc, argv, "room_scan");
    ros::NodeHandle n;
    speedPub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    scanSub = n.subscribe("/scan", 10, callbackScan);
    odomSub = n.subscribe("/odom", 10, callbackOdom);
    ros::spin();
 }
