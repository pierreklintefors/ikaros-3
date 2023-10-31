#include "matrix.h"

using namespace ikaros;

int
main()
{
    matrix m(2,2);
    m.print();

    matrix x = {{{1, 2}, {3, 4}}, {{69,70}, {9, 10}}};

    std::cout << x.json() << std::endl;

    
}