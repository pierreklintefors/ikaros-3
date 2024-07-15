#include "ikaros.h"
#include <random> // Include the header file that defines the 'Random' function

using namespace ikaros;

class CurrentPositionMapping: public Module
{
  
    matrix present_position;
    matrix present_current;
    matrix position_transitions;
    
    matrix goal_current;
    matrix goal_positions_out;

     

    parameter number_num_transitions;
    parameter min_limits;
    parameter max_limits;
    parameter robotType;

    std::random_device rd;
    
    bool goalReached;
    int position_margin = 2;
    int transition = 0;
    int current_increment;
    matrix current_controlled_servos_torso;

 
    matrix SetCurrent(matrix present_current, matrix goal_current, int increment){
        matrix current_output;
        std::cout << "inside set current" << std::endl;
        if(present_current.size() != goal_current.size()){
            std::cout << "mismatch between present and goal current size" << std::endl;
            Notify(msg_fatal_error, "Present current and Goal current must be the same size");
            return -1;
        }
        for (int i = 0; i < present_current.size(); i++) {
            if (present_current(i) + increment < goal_current(i)&& goal_current(i) > 2000)
                current_output(i) += increment;  
        }    
        return current_output;

    }

    matrix RandomisePositions(matrix present_position, int num_transitions, matrix min_limits, matrix max_limits, std::string robotType){
        // Initialize the random number generator with the seed
        std::mt19937 gen(rd());
        int num_columns = (robotType == "Torso") ? 2 : 12;
        
        matrix goal_positions(num_transitions, num_columns);
        goal_positions.set(0);
        goal_positions.info();
        if (num_columns != min_limits.size() && num_columns != max_limits.size()){
    
            Notify(msg_fatal_error, "Min and Max limits must have the same number of columns as the current controlled servos in the robot type (2 for Torso, 12 for full body)");
            return -1;
        }
        std::cout << "Inside random, passed max min check: " << num_columns << std::endl;
        if (robotType=="Torso"){
            for (int i = 0; i < num_transitions; i++) {
                for (int j = 0; j < num_columns; j++) {
                    std::uniform_real_distribution<double> distr(min_limits(j), max_limits(j));
                    goal_positions(i, j) = int(distr(gen));
                }
            }
            return goal_positions;
        }
    
        return goal_positions;
    }

    bool ReachedGoal(matrix present_position, matrix goal_positions, int margin){
        for (int i = 0; i < present_position.size(); i++) {
            if (abs(present_position(i) - goal_positions(i)) > margin)
                return false;
        }
        return true;
    }
    void Init()
    {
    
        Bind(present_current, "PresentCurrent");
        Bind(present_position, "PresentPosition");
        Bind(goal_current, "GoalCurrent");
        Bind(number_num_transitions, "NumberTransitions");
        Bind(goal_positions_out, "GoalPosition");

        Bind(min_limits, "MinLimits");
        Bind(max_limits, "MaxLimits");
        Bind(robotType, "RobotType");



        position_transitions.set_name("PositionTransitions");
        position_transitions = RandomisePositions(present_position, number_num_transitions, min_limits, max_limits, robotType);
        goal_positions_out.copy(position_transitions[0]);
        current_increment = 5;
        
        
    }


    void Tick()
    {   
        if (present_current.connected() && goal_current.connected()){
            goal_current = SetCurrent(present_current, goal_current, current_increment);
            std::cout << "Present current: " << present_current << std::endl;
            goal_current.print();
            
            if (ReachedGoal(present_position, goal_positions_out, position_margin)){
                transition++;
                if (transition < number_num_transitions){
                    goal_positions_out.copy(position_transitions[transition]);
                }
                else{
                    Notify(msg_warning, "All transitions completed");
                }
            }
            
        }
        
        
        goal_positions_out.print();
    }

    

};





INSTALL_CLASS(CurrentPositionMapping)

