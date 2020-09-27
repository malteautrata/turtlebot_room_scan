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
void callback();
ros::Subscriber sub;

double Ngray = 0;

void callback(const nav_msgs::OccupancyGrid::ConstPtr& msg)
{
     nav_msgs::MapMetaData info = msg->info;
 for(int i=0; i<info.height*info.width; i++)
    {
      //Checken ob der Rahmen geschlossen ist, dadurch, das abgefragt wird, ob neben Pixeln die als grau gekennzeichnet sind(0) Pixel sind, die unbekannt sind(-1). 
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Rechts
            std::cout<<"Karte nicht vollständig!"<<std::endl;

            return;
        }
        if(double((msg->data[i])==0)&&(double(msg->data[i+1])==-1))
        {
            //Links
            std::cout<<"Karte nicht vollständig!"<<std::endl;
            return;
        }
         if(i>info.width)
        {
        if(double((msg->data[i])==0)&&(double(msg->data[i+info.width])==-1))
            {
                //Oben
                std::cout<<"Karte nicht vollständig!"<<std::endl;
                return;
            }
        }
        if(i<info.height*info.width-info.width)
        {
            if(double((msg->data[i])==0)&&(double(msg->data[i-info.width])==-1))
            {
                //Unten
                std::cout<<"Karte nicht vollständig!"<<std::endl;
                
                return;
            }
        }
        
    } 
    std::cout<<"Karte vollständig"<<std::endl;
    system("rosrun map_server map_saver -f ~/map");
    exit(0);


  std::cout<<Ngray<<std::endl;
  Ngray=0;
  
  
  
  
  
  
  
  
  
  
  
  
  
    
}

int main(int argc, char** argv){
    
    ros::init(argc, argv, "end");
    ros::NodeHandle n;
    sub = n.subscribe("/map", 1, callback);

    

    while(ros::ok())
    {
        ros::spin();
    }
 }
 
