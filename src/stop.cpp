#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/OccupancyGrid.h"
#include "std_msgs/Header.h"
#include "nav_msgs/MapMetaData.h"
#include "std_msgs/Header.h"
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <thread>   
#include <algorithm> 

ros::Publisher pub;
ros::Subscriber sub;
void callback();

void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    geometry_msgs::Twist twist;
    twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
    twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0;
    pub.publish(twist);
    exit(0);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "stop");
    ros::NodeHandle n;
    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    sub = n.subscribe("/scan", 10, callback);

    while(ros::ok())
    {
        ros::spin();
    }

 }