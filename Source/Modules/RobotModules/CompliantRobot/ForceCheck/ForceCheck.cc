#include "ikaros.h"
//#include "dynamixel_sdk.h"

using namespace ikaros;

class ForceCheck: public Module
{
    matrix present_current;
    matrix current_limit;
    matrix current_output;
    matrix present_position;// assumes degrees
    matrix goal_position_in;
    matrix goal_position_out;
    
    parameter gain_constant;
    parameter smooth_factor;
    parameter error_threshold;

    int tickCount;
    int current_increment;
    int current_value;
    int position_margin;
    int current_margin;
    int minimum_current;
   
    bool firstTick = true;
    bool obstacle = false;

    double Tapering(double error, double threshold)
    {
        if (abs(error) < threshold)
            return abs(error)/threshold;
        else
            return 1;
    }
    
    matrix SetCurrent(matrix currents, matrix limits, matrix positions, matrix goal_position_in, int gain, int error_threshold, double smooth_factor)
    {
        for (int i = 0; i < currents.size(); i++) {
            int position = positions[i];
            if (position > 0) 
            {
                int current_value = abs(currents[i]);
                int limit_value = limits[i];
                int position = positions[i];
                int goal = goal_position_in[i];
                double error = goal - position;
                double error_tapered = Tapering(error, error_threshold);
                std::cout << "error " << error << std::endl;
                std::cout << "eroor_tapered" << error_tapered << std::endl;
                int suggested_current = 5+  current_value + (gain* error * error_tapered);
                current_output[i] = std::min(suggested_current , limit_value); // Cap at limit_value 
                
            }
                         
            
             
        }
        
        return current_output;
}

    //Could also be done by using dynamixel_sdk
    void ObstacleCheck(matrix positions,  matrix current_limits, matrix goal_position_in, matrix goal_position_out )
    {   
        for (int i = 0; i < positions.size(); i++) {
            float current_position = positions(i);
            float goal = goal_position_in[i];
            float current_value = current_output[i];
            float limit_value = current_limit[i];
            if (current_position >0){

                if (abs(goal - current_position) > position_margin && abs(limit_value -current_value) < current_margin)
                {   
                    current_output[i]=0; 
                    //std::cout << "Obstacle detected, lowering current for servo " << i+1 << " to " << current_output(i) << std::endl;
                    goal_position_out[i] = current_position;
                    //std::cout << "Goal position changed to " << current_position << std::endl;
                }
                
            }
        }
    }

    void Init()
    {
        Bind(present_current, "PresentCurrent");
        Bind(current_limit, "CurrentLimit");
        Bind(current_output, "CurrentOutput");
        Bind(present_position, "PresentPosition");
        Bind(goal_position_in, "GoalPositionIn");
        Bind(goal_position_out, "GoalPositionOut");

        Bind(gain_constant, "GainConstant");
        Bind(smooth_factor, "SmoothFactor");
        Bind(error_threshold, "ErrorThreshold");
      
    
    
        current_margin = 10;
        position_margin = 5;
        gain_constant = 0.5;
        smooth_factor = 1;
        error_threshold = 5;
        
    

    }


    void Tick()
    {   
        if (firstTick){
            goal_position_out.copy(goal_position_in);
        }

        if (present_position.connected() && present_current.connected() && goal_position_in.connected()) {
            ObstacleCheck(present_position, current_limit, goal_position_in, goal_position_out);
            current_output = SetCurrent(present_current, current_limit, present_position, goal_position_in, gain_constant, error_threshold, smooth_factor);
        }
        
        current_output.print();
       
        firstTick=false;
            
    }
};


INSTALL_CLASS(ForceCheck)

