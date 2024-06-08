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
        g.print("g");
        data.print("data");
        mdata.print("mdata");
        x.print("x");
        y.print("t");

/*
        Bind(output1, "OUTPUT1");
        Bind(output2, "OUTPUT2");
        Bind(output3, "OUTPUT3");
        Bind(output4, "OUTPUT4");
        Bind(output5, "OUTPUT5");
        Bind(output6, "OUTPUT6");
        Bind(output7, "OUTPUT7");
        Bind(output8, "OUTPUT8");
        Bind(output9, "OUTPUT9");

        output1.print();
        output2.print();
        output3.print();
        output4.print();
        output5.print();
        output6.print();
        output7.print();
        output8.print();
        output9.print();
        */
    }


    void Tick()
    {
        //output.copy(data);
    }
};

INSTALL_CLASS(KernelTestModule)

