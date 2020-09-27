#include <ros/ros.h>
#include <tf/transform_broadcaster.h>
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <thread> 
#include <algorithm>        

void callback();
ros::Publisher pub;
ros::Subscriber sub;
bool shouldDodge = false;
bool driveToWall = true;
int turned = 0;

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

   // Hindernisse im 40° Winkel rechts und links vom Roboter erkennen
    for(int i = 1; i <= 40; i++)
    {
        if (msg->ranges[i] <= 0.15 && msg->ranges[i] >= 0.0001)
        {
            driveToWall = false;
            shouldDodge = true;
        }
    }

    for(int i = 320; i <= 359; i++)
    {
        if (msg->ranges[i] <= 0.15 && msg->ranges[i] >= 0.0001)
        {
            driveToWall = false;
            shouldDodge = true;
        }
    }

    if (msg->ranges[0] <= 0.2)
    {
        driveToWall = false;
        shouldDodge = true;
    }

    // An der Wand orientieren; der Roboter soll sich so drehen, dass die Wand rechts neben ihm in Reichweite ist
    if (!driveToWall)
    {
        for(int i = 250; i <= 290; i++)
        {
            if (msg->ranges[i] >= 0.35)
            {
                 //std::cout << "1" << std::endl;
                shouldDodge = true;
            }
        }
    }
    

    // Wenn der Roboter sich zu sehr im Raum und zu weit entfernt von der Wand befindet, soll dieser zur nächst naheliegenden Wand fahren
    int distance = 10000;
    int degree = 10000;

    if (turned >= 40)
    {
        for (int i = 0; i <= 359; i++)
        {
            if(distance > msg->ranges[i])
            {
                distance = msg->ranges[i];
                degree = i;
            }
        }
        
        std::cout << degree << std::endl;

        if ((degree <= 20 && degree >= 0) || (degree <= 359 && degree >= 340))
        {
            driveToWall = true;
        }
    }


    // Es wird abgefragt, ob der Roboter sich drehen oder geradeaus fahren soll
    if (shouldDodge && !driveToWall)
    {
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 1.0;
        turned++;
        std::cout << "Turned: " << turned << std::endl;
    }

    else
    {
        twist.linear.x = 0.5; twist.linear.y = 0.0; twist.linear.z = 0.0;
        turned = 0;
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
