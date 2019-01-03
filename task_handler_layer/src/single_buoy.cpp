#include <single_buoy.h>

singleBuoy::singleBuoy(): forwardPIDClient("forwardPID"), sidewardPIDClient("sidewardPID"), 
                        anglePIDClient("turnPID"), upwardPIDClient("upwardPID"), th(15)
{
    forward_sub_ = nh_.subscribe("/anahita/x_coordinate", 1, &singleBuoy::forwardCB, this);
}
singleBuoy::~singleBuoy() {}

void singleBuoy::setActive(bool status) {

    if (status) {
        ros::Duration(1).sleep();

        ROS_INFO("Waiting for sidewardPID server to start, task buoy.");
        sidewardPIDClient.waitForServer();

        ROS_INFO("sidewardPID server started, sending goal, task buoy.");
        sidewardPIDgoal.target_distance = 0;
        sidewardPIDClient.sendGoal(sidewardPIDgoal);

        ///////////////////////////////////////////////////

        ROS_INFO("Waiting for upwardPID server to start.");
        upwardPIDClient.waitForServer();

        ROS_INFO("upwardPID server started, sending goal.");
        upwardPIDgoal.target_depth = 0; // for gazebo
        // upwardPIDClient.sendGoal(upwardPIDgoal);

        ///////////////////////////////////////////////////

        ROS_INFO("Waiting for anglePID server to start.");
        anglePIDClient.waitForServer();

        ROS_INFO("anglePID server started, sending goal.");

        anglePIDGoal.target_angle = 0;
        anglePIDClient.sendGoal(anglePIDGoal);

        /////////////////////////////////////////////////////
        ROS_INFO("SETTING F0RWARD PWM TO 50");
        nh_.setParam("/pwm_surge", 50);

        while (!forwardGoalReceived && ros::ok()) {}

        ROS_INFO("forward distance received");

        while(forward_distance_ >= 60 && ros::ok()) {
        }

        ROS_INFO("forward distance less than 60");

        sidewardPIDClient.cancelGoal();
        // upwardPIDClient.cancelGoal();
        nh_.setParam("/pwm_sway", 0);

        nh_.setParam("/pwm_surge", 50);

        ros::Duration(6).sleep(); // 8 for gazebo and 6 for real world
        //////////////////////////////////////////////////////
        nh_.setParam("/pwm_surge", -50);

        ROS_INFO("Buoy Task, moving backward");
        
        ros::Duration(10).sleep(); // 10 for gazebo and 8 for real world
        
        nh_.setParam("/pwm_surge", 0);

        ROS_INFO("moving backward finished");

        ROS_INFO("SidewardPID Client sending goal again, task buoy.");
        sidewardPIDgoal.target_distance = 0;
        sidewardPIDClient.sendGoal(sidewardPIDgoal);
        
        ROS_INFO("UpwardPID Client sending goal again, task buoy.");
        upwardPIDgoal.target_depth = 0;
        // upwardPIDClient.sendGoal(upwardPIDgoal);

        //////////////////////////////////////////////////////

        // ROS_INFO("ForwardPID Client sending goal again, task buoy.");        
        // forwardPIDgoal.target_distance = 50;
        // forwardPIDClient.sendGoal(forwardPIDgoal);

        // while(forward_distance_ <= 50 && ros::ok()) {
        //     continue;
        // }

        // th.isAchieved(50, 15, "forward");

        // forwardPIDClient.cancelGoal();
        
        ROS_INFO("Killing the thrusters");
	    nh_.setParam("/kill_signal", true);

        anglePIDClient.cancelGoal();
    }
    else {
        // upwardPIDClient.cancelGoal();
        sidewardPIDClient.cancelGoal();
        ROS_INFO("Closing Single Buoy");
    }
}

void singleBuoy::forwardCB(const std_msgs::Float32ConstPtr &_msg) {
    forward_distance_ = _msg->data;
    forwardGoalReceived = true;
}
