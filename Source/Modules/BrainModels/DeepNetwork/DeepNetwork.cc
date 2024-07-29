#include "ikaros.h"
#include "network.h"
using namespace ikaros;

class DeepNetwork: public Module
{
    parameter spec_filename;
    parameter weights_filename;
    parameter do_load_weights;
    parameter do_save_weights;
    parameter learning_rate;
    matrix input;
    matrix t_input;
    matrix t_target;
    matrix loss;
    matrix output;

    NeuralNetwork *network;


    void Init()
    {
        Bind(spec_filename, "spec_filename");
        Bind(weights_filename, "weights_filename");
        Bind(do_load_weights, "load_weights");
        Bind(do_save_weights, "save_weights");
        Bind(learning_rate, "learning_rate");
        Bind(input, "INPUT");
        Bind(t_input, "T_INPUT");
        Bind(t_target, "T_TARGET");
        Bind(loss, "loss");

        Bind(output, "OUTPUT");

        // TODO make input to network dynamic 
        network = new NeuralNetwork(spec_filename);
        if(do_load_weights) network->load_weights(weights_filename);
    }


    void Tick()
    {
        network->forward(input);
        output.copy(network->get_output());
        if(!t_input.empty() && t_target.empty()){
            network->backward(t_input, t_target);
            network->update(learning_rate);
            loss[0] = network->compute_loss(t_target);
        }
    }
};

INSTALL_CLASS(DeepNetwork)

