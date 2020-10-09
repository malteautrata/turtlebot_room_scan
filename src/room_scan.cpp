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
#include <math.h>       
#define pi 3.1415

void callback();
void callbackMap();
ros::Publisher pub;
ros::Subscriber subScan;
ros::Subscriber subMap;

bool shouldDodge = false;
bool driveToWall = true;
int turned = 0;
double Ngray = 0;

bool randomBool()
{
    return rand() % 2;
}

void cleanShutdown()
{
    geometry_msgs::Twist twist;
    twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
    twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0;
    pub.publish(twist);
}

void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    geometry_msgs::Twist twist;
    shouldDodge = false;
    float abstand = 10000000;
   // Hindernisse im 40° Winkel rechts und links vom Roboter erkennen
    for(int i = 1; i <= 40; i++)
    {
        if (abs(sin(i*pi/180) * msg->ranges[i]) < abstand && abs(cos(i*pi/180) * msg->ranges[i]) < 0.2 && msg->ranges[i] > 0)
        {
            abstand = sin(i*pi/180) * msg->ranges[i];
        }
    }
    std::cout << "Abstand bis 40°: " << abs(abstand) << std::endl;

    for(int i = 320; i <= 359; i++)
    {
        if (abs(sin(i*pi/180) * msg->ranges[i]) < abstand && abs(cos(i*pi/180) * msg->ranges[i]) < 0.2 && msg->ranges[i] > 0)
        {
            abstand = sin(i*pi/180) * msg->ranges[i];
        }
    }
    std::cout << "Abstand bis 360: " << abs(abstand) << std::endl;

    if (abs(abstand) <= 0.15)
    {
        driveToWall = false;
        shouldDodge = true;
    }
    


    if (msg->ranges[0] <= 0.2)
    {
        driveToWall = false;
        shouldDodge = true;
    }

    // An der Wand orientieren; der Roboter soll sich so drehen, dass die Wand rechts neben ihm in Reichweite ist
    if (!driveToWall)
    {
        for(int i = 270; i <= 290; i++)
        {
            if (msg->ranges[i] >= 0.4)
            {
                 //std::cout << "1" << std::endl;
                shouldDodge = true;
            }
        }
    }
    

    // Wenn der Roboter sich zu sehr im Raum und zu weit entfernt von der Wand befindet, soll dieser zur nächst naheliegenden Wand fahren
    int distance = 10000;
    int degree = 10000;

    if (turned >= 30)
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

        if ((degree <= 5 && degree >= 0) || (degree <= 359 && degree >= 350))
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
        //std::cout << "Turned: " << turned << std::endl;
    }
    else
    {
        if (driveToWall)
        {
            distance = 10000;
            degree = 10000;


            for (int i = 0; i <= 359; i++)
            {
                if(distance > msg->ranges[i])
                {   
                    distance = msg->ranges[i];
                    degree = i;
                }
            }
        
            if ((degree <= 5 && degree >= 0) || (degree <= 359 && degree >= 350))
            {
                twist.linear.x = 0.3; twist.linear.y = 0.0; twist.linear.z = 0.0;
                turned = 0;

            }
            else
            {
                twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
                twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 1.0;
            }
        }
        else 
        {
            twist.linear.x = 0.3; twist.linear.y = 0.0; twist.linear.z = 0.0;
            turned = 0;
        }

        
    }
    pub.publish(twist);
}

void callbackMap(const nav_msgs::OccupancyGrid::ConstPtr& msg)
{
    nav_msgs::MapMetaData info = msg->info;
    int mistakes = 0;
    for(int i=0; i<info.height*info.width; i++)
    {
      //Checken ob der Rahmen geschlossen ist, dadurch, das abgefragt wird, ob neben Pixeln die als grau gekennzeichnet sind(0) Pixel sind, die unbekannt sind(-1). 
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Rechts
            //std::cout<<"Karte nicht vollständig!"<<std::endl;

            mistakes++;
        }
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Links
            //std::cout<<"Karte nicht vollständig!"<<std::endl;
            mistakes++;
        }
         if(i>info.width)
        {
        if(double((msg->data[i])==0)&&(double(msg->data[i+info.width])==-1))
            {
                //Oben
                //std::cout<<"Karte nicht vollständig!"<<std::endl;
                mistakes++;
            }
        }
        if(i<info.height*info.width-info.width)
        {
            if(double((msg->data[i])==0)&&(double(msg->data[i-info.width])==-1))
            {
                //Unten
                //std::cout<<"Karte nicht vollständig!"<<std::endl;
                
                mistakes++;
            }
        }
        
    }
    std::cout<< "Map Fehler:" << mistakes << std::endl;

    if (mistakes < 30)
    {
        std::cout<<"Karte vollständig"<<std::endl;
        system("rosrun map_server map_saver -f ~/map");
        cleanShutdown();
        exit(0);

        std::cout<<Ngray<<std::endl;
        Ngray=0; 
    }  
}

void launchSlam()
{
    system("roslaunch turtlebot3_slam turtlebot3_slam.launch slam_methods:=gmapping");
}


int main(int argc, char** argv){
    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);
    ros::init(argc, argv, "room_scan");
    ros::NodeHandle n;

    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    subScan = n.subscribe("/scan", 10, callback);
    subMap = n.subscribe("/map", 1, callbackMap);

    std::thread slamThread (launchSlam); 

    while(ros::ok())
    {
        ros::spin();
    }
 }
