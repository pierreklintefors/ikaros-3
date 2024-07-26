#include "ikaros.h"

using namespace ikaros;

class KernelTestModule: public Module
{
    parameter a;
    parameter b;
    parameter c;
    parameter d;
    parameter e;

    parameter f1; // number
    parameter f2; // string
    parameter f3; // bool

    parameter g;

    parameter data;
    parameter mdata;

    parameter x;
    parameter y;

    matrix input;
    matrix output;

    matrix output1;
    matrix output2;
    matrix output3;
    matrix output4;
    matrix output5;
    matrix output6;
    matrix output7;
    matrix output8;
    matrix output9;

    void Init()
    {
        Bind(b, "b");
        if(b)
            std::cout << "b: true" << std::endl;
        else
            std::cout << "b: false" << std::endl;

        std::cout << GetValue("p") << std::endl;  // => 333


        std::cout << GetValue("a") << std::endl;
        std::cout << GetValue("b") << std::endl;
        std::cout << GetValue("c") << std::endl;
        std::cout << GetValue("d") << std::endl;
        std::cout << GetValue("e") << std::endl;

        std::cout << GetValue("f1") << std::endl;
        std::cout << GetValue("f2") << std::endl;
        std::cout << GetValue("f3") << std::endl;

        std::cout << GetValue("g") << std::endl;
        std::cout << GetValue("data") << std::endl;

        std::cout << GetValue("z") << std::endl;
        std::cout << GetValue("Test_2.check_sum") << std::endl;
        std::cout << GetValue(".check_sum") << std::endl;
        std::cout << GetValue(".Const.data") << std::endl;

            std::cout << GetValue("Test_2.Const.Test_2.Test.Test_2.Test.g") << std::endl;
            std::cout << GetValue("Test_2.Const.Test_2.Test.Test_2.g") << std::endl;
            std::cout << GetValue("Test_2.Const.Test_2.Test.z") << std::endl;
    


        Bind(a, "a");
        Bind(b, "b");
        Bind(c, "c");
        Bind(d, "d");
        Bind(e, "e");

        Bind(f1, "f1");
        Bind(f2, "f2");
        Bind(f3, "f3");

        Bind(g, "g");
        Bind(data, "data");
        Bind(mdata, "mdata");
        Bind(x, "x");
        Bind(y, "y");


        a.print("a");
        b.print("b");
        c.print("c");
        d.print("d");
        e.print("e");

        f1.print("f1");
        f2.print("f2");
        f3.print("f3");

        std::cout << "f1: " << f1 << std::endl;
        std::cout << "f1: " << f1.json() << std::endl;
        f1= 17;
        std::cout << "f1: " << f1 << std::endl;   
        f1= 0;
        std::cout << "f1: " << f1 << std::endl; 

        g.print("g");
        double gg = g;
        std::cout << "g rate scaled: " << gg << std::endl; 


        data.print("data");
        mdata.print("mdata");
        x.print("x");
        y.print("t");

        Bind(input, "INPUT");
        Bind(output, "OUTPUT");

        Bind(output1, "OUTPUT1");
        Bind(output2, "OUTPUT2");
        Bind(output3, "OUTPUT3");
        Bind(output4, "OUTPUT4");
        Bind(output5, "OUTPUT5");
        Bind(output6, "OUTPUT6");
        Bind(output7, "OUTPUT7");
        Bind(output8, "OUTPUT8");
        Bind(output9, "OUTPUT9");
/*
        output1.info();
        output2.info();
        output3.info();
        output4.info();
        output5.info();
        output6.info();
        output7.info();
        output8.info();
        output9.info();
*/

    // TESTING PARAMETERS

        std::cout << "PARAMETER TEST" << std::endl; 

    parameter xx("number");
    parameter yy("string");
    parameter xxo("number", "AA,BB,CC");
    parameter yyo("string", "aa,bb,cc");

    xx.info();
    yy.info();
    xxo.info();
    yyo.info();

    xx.print();
    yy.print();
    xxo.print();
    yyo.print();


        std::cout << "DOUBLE ASSIGNMENT TEST" << std::endl; 

    xx = 1.2;
    yy = 1.2;
    xxo = 1.2;
    yyo = 1.2;

    xx.print();
    yy.print();
    xxo.print();
    yyo.print();
        std::cout << "STRING ASSIGNMENT TEST" << std::endl; 

    xx = "123";
    yy = "123";
    xxo = "BB";
    yyo = "bb";

    xx.print();
    yy.print();
    xxo.print();
    yyo.print();

        std::cout <<yyo*1 << std::endl; 

    }


    void Tick()
    {
        output.copy(input);
    }
};

INSTALL_CLASS(KernelTestModule)

