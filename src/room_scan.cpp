#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <thread>         

void callback();
ros::Publisher pub;
ros::Subscriber sub;
bool shouldDodge = false;
bool isTurning;
bool turnLeft;
bool randomBool()
{
    return rand() % 2;
}

void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    geometry_msgs::Twist twist;

    // Herausfinden wieviele Daten im angle[] array gespeichert sind: von 0 bis 359
    //int num_readings = std::ceil( (msg->angle_max - msg->angle_min) / msg->angle_increment );
    //std::cout << num_readings << std::endl;
    //std::cout << "Range 350:" <<  msg->ranges[350] << std::endl;


    // Herausfinden welche bei der Ausweichung sinnvoll sind
    /*
    for (int i = 0; i <= 359; i++)
    {
        if (msg->ranges[i] <= 0.3 && msg->ranges[i] >= 0.00001)
        {
            std::cout << i << std::endl;
        } 
    }
    std::cout << "new" << std::endl;
    */

    shouldDodge = false;

    for(int i = 1; i <= 45; i++)
    {
        if (msg->ranges[i] <= 0.15 && msg->ranges[i] >= 0.0001)
        {
            shouldDodge = true;
        }
    }

    for(int i = 315; i <= 359; i++)
    {
        if (msg->ranges[i] <= 0.15 && msg->ranges[i] >= 0.0001)
        {
            shouldDodge = true;
        }
    }

    if (msg->ranges[0] <= 0.2)
    {
        shouldDodge = true;
    }

    if(shouldDodge)
    {
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        std::cout << randomBool() << std::endl;

        if (isTurning)
       {

       } else
       {
           turnLeft = randomBool();
       }
       
       isTurning = true;

        if(turnLeft)
        {
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 5.0;

        } else 
        {
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = -5.0;

        }
    } else 
    {
        twist.linear.x = 0.5; twist.linear.y = 0.0; twist.linear.z = 0.0;
        isTurning = false;
    }
    pub.publish(twist);
}

void launchSlam()
{
    system("roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping");
}

// wird nicht genutzt
void cleanShutdown()
{
    
}

int main(int argc, char** argv){
    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);
    ros::init(argc, argv, "drive_random");
    ros::NodeHandle n;
    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    sub = n.subscribe("/scan", 10, callback);

    std::thread slamThread (launchSlam); 

    while(ros::ok())
    {
        ros::spin();
    }
 }
