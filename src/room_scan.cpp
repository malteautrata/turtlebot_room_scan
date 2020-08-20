#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/Imu.h"

struct startPosition
{
    double x;
    double y;
};

void callback();
ros::Publisher twistPub;
ros::Subscriber scanSub;
ros::Subscriber odomSub;
ros::Subscriber imuSub;

double xPos;
double yPos;

//Der Abstand, zu dem die Wand abgefahren werden soll
double distanceToWall = 1;

//Startposition merken
startPosition startPos;   

//evtl. nicht benötigt
bool startReached = false;

// herausfinden, wann der Raum koomplett gescannt wurde

void callbackScan(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    std::cout << msg->ranges[90]; //herausfinden was der Wert bei 90° entspricht
   
    geometry_msgs::Twist twist;

    if (xPos == startPos.x)
    {
        distanceToWall += 1;
    }

    //Turtlebot darf nicht mehr als 90° drehen, damit er nicht zurück fährt
    if(msg->ranges[90] <= distanceToWall && msg->ranges[0] >= 0.3)  
    {
        twist.linear.x = 0.5; twist.linear.y = 0.0; twist.linear.z = 0.0;
    } else {
        
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 10.0;
    }
    twistPub.publish(twist);
}

void callbackOdom(const nav_msgs::Odometry::ConstPtr& odom)
{
    std::cout << odom->pose.pose.position.x;
    std::cout << odom->pose.pose.position.y;

    xPos = odom->pose.pose.position.x;
    yPos = odom->pose.pose.position.y;

    if (yPos < startPos.y)
    {
        startPos.x = xPos;
        startPos.y = yPos;
    }
}

void callbackImu(const sensor_msgs::Imu::ConstPtr& imu)
{
    //Read out rotation
    std::cout << imu->angular_velocity;
}

int main(int argc, char** argv){
    ros::init(argc, argv, "room_scan");
    ros::NodeHandle n;
    twistPub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    scanSub = n.subscribe("/scan", 10, callbackScan);
    odomSub = n.subscribe("/odom", 10, callbackOdom);
    imuSub = n.subscribe("/imu", 10, callbackImu);
    while(ros::ok())
    {
        ros::spin();
    }
 }
