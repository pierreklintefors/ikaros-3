#include "ikaros.h"

using namespace ikaros;

class KernelTestModule: public Module
{
    parameter a;
    parameter b;
    parameter c;
    parameter d;
    parameter e;
    parameter f;
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
        std::cout << GetValue("f") << std::endl;
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
        Bind(f, "f");
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

        f.print("f");
        std::cout << "f: " << f << std::endl;
        std::cout << "f: " << f.json() << std::endl;
        f= 17;
        std::cout << "f: " << f << std::endl;   


        g.print("g");
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
    }


    void Tick()
    {
        output.copy(input);
    }
};

INSTALL_CLASS(KernelTestModule)

