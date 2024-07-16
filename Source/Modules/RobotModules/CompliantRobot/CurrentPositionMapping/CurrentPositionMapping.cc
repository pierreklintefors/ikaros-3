#include "ikaros.h"
#include <random> // Include the header file that defines the 'Random' function


using namespace ikaros;

class CurrentPositionMapping: public Module
{
  
    matrix present_position;
    matrix present_current;
    matrix position_transitions;
    matrix previous_position;
    matrix start_position;
    
    matrix goal_current;
    matrix goal_positions_out;
    matrix current_controlled_servos;
    matrix max_present_current;
    matrix min_moving_current;

     

    parameter number_num_transitions;
    parameter min_limits;
    parameter max_limits;
    parameter robotType;


    std::random_device rd;
    
    bool goalReached;
    int position_margin = 4;
    int transition = 0;
    int current_increment = 2;
    int starting_current = 10;
    int current_limit = 1200;
    

    double time_prev_position;
    double time_interval_sampling_position = 0.1;

 
    matrix SetCurrent(matrix present_current,  int increment, int limit){
        matrix current_output(present_current.size());
        Notify(msg_debug, "Inside SetCurrent()");
        if(present_current.size() != current_output.size()){
            std::cout << present_current.size() << std::endl;
            std::cout << current_output.size() << std::endl;
            Notify(msg_fatal_error, "Present current and Goal current must be the same size");
            return -1;
        }
        for (int i = 0; i < current_controlled_servos.size(); i++) {
            if (abs(present_current(current_controlled_servos(i))) + increment < goal_current(current_controlled_servos(i))&& goal_current(current_controlled_servos(i)) < limit){
                Notify(msg_print, "Increasing current");
                current_output(current_controlled_servos(i)) = goal_current(current_controlled_servos(i))+ increment;
                if(abs(present_current(current_controlled_servos(i))) > max_present_current(current_controlled_servos(i))){
                    max_present_current(current_controlled_servos(i)) = abs(present_current(current_controlled_servos(i)));
                }
                
            }
            else
            {
                current_output(i) = goal_current(i);
            }
        }    
        return current_output;

    }

    matrix RandomisePositions(matrix present_position, int num_transitions, matrix min_limits, matrix max_limits, std::string robotType){
        // Initialize the random number generator with the seed
        std::mt19937 gen(rd());
        int num_columns = (robotType == "Torso") ? 2 : 12;
        
        matrix goal_positions(num_transitions, present_position.size());
        goal_positions.set(0);
        if (num_columns != min_limits.size() && num_columns != max_limits.size()){
    
            Notify(msg_fatal_error, "Min and Max limits must have the same number of columns as the current controlled servos in the robot type (2 for Torso, 12 for full body)");
            return -1;
        }
        if (robotType=="Torso"){
            for (int i = 0; i < num_transitions; i++) {
                for (int j = 0; j < num_columns; j++) {
                    std::uniform_real_distribution<double> distr(min_limits(j), max_limits(j));
                    goal_positions(i, j) = int(distr(gen));
                }
            }
        }
        
        
    
        return goal_positions;
    }

    bool ReachedGoal(matrix present_position, matrix goal_positions, int margin){
        for (int i = 0; i < current_controlled_servos.size(); i++) {
            if (abs(present_position(current_controlled_servos(i)) - goal_positions(current_controlled_servos(i))) > margin){
                std::cout << "Not reached goal" << std::endl;
                Notify(msg_debug, "Not reached goal");
                return false;
            }
        }
        std::cout << "Reached goal" << std::endl;
        
        return true;
    }

    bool Moving(matrix present_position, matrix previous_positionm, int margin){
        for (int i =0; i < present_position.size_x(); i++){
            if (abs(present_position(i)) - previous_position(i) > margin){
                Notify(msg_debug, "Moving");
                return true;
            }
            else{
                Notify(msg_debug, "Not moving");
                return false;
            }
        }
        return false;
    }   
    //saving starting position, goal position and current in json file
    void SavePositionsJson(matrix goal_positions, matrix start_position, matrix max_current, matrix min_current, std::string robotType, int transition){
        std::ofstream file;
        std::string scriptPath = __FILE__;
        std::string scriptDirectory = scriptPath.substr(0, scriptPath.find_last_of("/\\"));
        std::string filePath = scriptDirectory + "/CurrentPositionMapping" + robotType + ".json";
        
        // Check if file exists
        std::ifstream checkFile(filePath);
        bool fileExists = checkFile.good();
        checkFile.close();
        
        file.open(filePath, std::ios::app);
    
        if (transition==1 && !fileExists){
            file << "{\n";
            file << "\"Mapping\": [\n";
        }
        file << "{\"StartingPosition\": [";
        for (int i = 0; i < current_controlled_servos.size(); i++){
            file << start_position(current_controlled_servos(i));
            if (i < current_controlled_servos.size()-1){
                file << ", ";
            }     
        }
        file << "],";
        file << "\"GoalPosition\": [";
        for (int i = 0; i < current_controlled_servos.size(); i++){
            file << goal_positions(current_controlled_servos(i));
            if (i < current_controlled_servos.size()-1){
                file << ", ";
            }
        }
        file << "],";
        file << "\"MaxCurrent\": ["; 
        for (int i = 0; i < current_controlled_servos.size(); i++){
            file << max_current(current_controlled_servos(i));
            if (i < current_controlled_servos.size()-1){
                file << ", ";
            }
        }
        
        file << "],";
        file << "\"MinMovingCurrent\": ["; 
        for (int i = 0; i < current_controlled_servos.size(); i++){
            file << min_current(current_controlled_servos(i));
            if (i < current_controlled_servos.size()-1){
                file << ", ";
            }
        }
        //if transiiton is not the last one, add a comma
        if (transition < number_num_transitions){
            file << "]},";
        }
        else{
            file << "]}\n]\n}";
        }

        file << std::endl;
    
        file.close();
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

        previous_position.set_name("PreviousPosition");
        previous_position.copy(present_position);
        position_transitions.print();
        goal_current.set(starting_current);
        start_position.copy(present_position);
        max_present_current.set_name("MaxPresentCurrent");
        max_present_current.copy(present_current);
        min_moving_current.set_name("MinMovingCurrent");
        min_moving_current.copy(present_current);
     
        time_prev_position= std::time(nullptr);

        std::string robot = robotType;

        if(robot == "Torso"){
            current_controlled_servos.set_name("CurrentControlledServosTorso");
            current_controlled_servos = {0, 1};
        }
        else{
            current_controlled_servos.set_name("CurrentControlledServosFullBody");
            current_controlled_servos = {0, 1, 2, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 18};
        }
        
        current_controlled_servos.print(); 
    }


    void Tick()
    {   
        if (present_current.connected() && present_position.connected()){
            goal_current.copy(SetCurrent(present_current, current_increment, current_limit));
            goal_current.print();
            
            if (!Moving(present_position, previous_position, position_margin)){
            
                if (ReachedGoal(present_position, goal_positions_out, position_margin)){
                    transition++;
                    std::cout << "Transition: " << transition << std::endl;
                    //save starting position, goal position and current in json file
                    SavePositionsJson(goal_positions_out,start_position, max_present_current, min_moving_current, robotType, transition);

                    Sleep(1);
                    if (transition < number_num_transitions){
                        goal_positions_out.copy(position_transitions[transition]);
                    }
                    else{
                        Notify(msg_end_of_file, "All transitions completed");
                        std::exit(1); //terminate the program
                    }
                    start_position.copy(present_position);
                    goal_current.set(starting_current);
                    max_present_current.set(starting_current);
                    min_moving_current.set(starting_current);
                }
            }
            else{
                for (int i = 0; i < current_controlled_servos.size(); i++){
                    if(min_moving_current(current_controlled_servos(i)) > abs(present_current(current_controlled_servos(i)))){
                        min_moving_current(current_controlled_servos(i)) = abs(present_current(current_controlled_servos(i)));
                    }
                }
            }



            goal_positions_out.print();
            previous_position.print();
            present_position.print();
            
            if(abs(time_prev_position - std::time(nullptr)) > time_interval_sampling_position){
                time_prev_position = std::time(nullptr);
                previous_position.copy(present_position);
            
             }
         
        
        }
    }

    

};





INSTALL_CLASS(CurrentPositionMapping)

