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

// ein Publisher um die Geschwindigkeit des Roboters zu setzen
ros::Publisher pub;

// ein Subscriber für das Topic /scan, um die Sensordaten des Roboters aufzurufen
ros::Subscriber subScan;

// ein Subscriber für das Topic /map, um auf die Karten Daten zuzugreifen
ros::Subscriber subMap;

// eine Boolean-Variable, ob der Roboter sich drehen muss
bool shouldDodge = false;

// eine Boolean-Variable, ob der Roboter erst wieder zur Wand fahren muss
bool driveToWall = true;

// ein Zählwert, wie oft der Roboter sich gedreht hat, um bei einer Überschreitung eines bestimmten Wertes die nächstliegende Wand anzufahren
int turned = 0;

double Ngray = 0;

// eine zufällige Boolean-Variable wird zurückgegeben
bool randomBool()
{
    return rand() % 2;
}

// alle Werte des Roboters auf 0 setzen, um ihn zum stehen zu bringen
void cleanShutdown()
{
    geometry_msgs::Twist twist;
    twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
    twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0;
    pub.publish(twist);
}

// die Methode wird aufgerufen, wenn Sensordaten geliefert werden, hier passiert das reagieren des Roboters auf Hindernisse
void callback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
    // ein Objekt der Klasse Twist wird instanziiert; dieses besitzt Variablen, welche die Geschwinigkeiten entlang der Achsen festlegen
    geometry_msgs::Twist twist;

    // es wird davon ausgegangen, dass sich der Roboter zunächst nicht dreht
    shouldDodge = false;

    // der Abstand, der rechts oder links tatsächlich zwischen Roboter und Hindernis ist (über den Sinus errechnet)
    float abstand = 10000000;

    // der geringste Abstand zum Hindernis von 1° bis 40°
    for(int i = 1; i <= 40; i++)
    {
        if (abs(sin(i*pi/180) * msg->ranges[i]) < abstand && abs(cos(i*pi/180) * msg->ranges[i]) < 0.2 && msg->ranges[i] > 0)
        {
            abstand = sin(i*pi/180) * msg->ranges[i];
        }
    }
    //std::cout << "Abstand bis 40°: " << abs(abstand) << std::endl;

    // der geringste Abstand zum Hindernis von 1° bis 40° und von 320° bis 359°
    for(int i = 320; i <= 359; i++)
    {
        if (abs(sin(i*pi/180) * msg->ranges[i]) < abstand && abs(cos(i*pi/180) * msg->ranges[i]) < 0.2 && msg->ranges[i] > 0)
        {
            abstand = sin(i*pi/180) * msg->ranges[i];
        }
    }
    //std::cout << "Abstand bis 360: " << abs(abstand) << std::endl;

    // wenn der Abstand kleiner als 15cm oder direkt vor dem Roboter kleiner als 20cm ist, befindet sich der Roboter bereits an einer Wand, allerdings muss er sich noch drehen um auszuweichen
    if (abs(abstand) <= 0.15 || (msg->ranges[0] <= 0.2 && msg->ranges[0] > 0))
    {
        driveToWall = false;
        shouldDodge = true;
    }
    


    // An der Wand orientieren; der Roboter soll sich solange drehen, bis die Wand rechts neben ihm in Reichweite ist
    if (!driveToWall)
    {
        for(int i = 270; i <= 290; i++)
        {
            if (msg->ranges[i] >= 0.4)
            {
                shouldDodge = true;
            }
        }
    }
    

    // wenn der Roboter sich zu sehr im Raum befindet und zu weit entfernt von der Wand befindet, soll dieser zur nächst naheliegenden Wand fahren
    // es wird die nächstliegende Wand und der dazugehörige Winkel gesucht
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
        
        // wenn der Roboter zur Wand ausgerichtet ist, soll er diese anfahren
        if ((degree <= 5 && degree >= 0) || (degree <= 359 && degree >= 350))
        {
            driveToWall = true;
        }
    }


    // Es wird abgefragt, ob der Roboter sich drehen oder geradeaus fahren soll
    if (shouldDodge && !driveToWall)
    {
        // der Roboter soll sich drehen
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0;
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 1.0;
        turned++;
        //std::cout << "Turned: " << turned << std::endl;
    }
    else
    {
        // wenn der Roboter zur nächstliegende Wand fahren soll, wird nochmals geprüft, ob er immer noch zur nächstliegenden Wand ausgerichtet ist; wenn nicht soll er sich weiter drehen
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

        // der Roboter fährt an der Wand entlang
        else 
        {
            twist.linear.x = 0.3; twist.linear.y = 0.0; twist.linear.z = 0.0;
            turned = 0;
        }   
    }

    // das Objekt twist wird in das Topic /cmd_vel veröffentlicht
    pub.publish(twist);
}

// eine Callback Methode, die aufgerufen wird, wenn sich die Karten Daten ändern
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

// die ROS-Node turtlebot3_slam soll gestartet werden, um die Karte des Roboters zu starten
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

    // Der Publisher und die Subscriber bekommen die jeweiligen Topics und Callback-Funktionen zugewiesen
    pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 10);
    subScan = n.subscribe("/scan", 10, callback);
    subMap = n.subscribe("/map", 1, callbackMap);

    // die Methode launchSlam wird auf einen neuen Thread geöffnet
    std::thread slamThread (launchSlam); 

    while(ros::ok())
    {
        ros::spin();
    }
 }
