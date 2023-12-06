// matrix.cc

#include "matrix.h"


namespace ikaros {

float
matrix::sum()
{
    if(empty())
        return 0;
    if(is_scalar())
        return (*data_)[info_->offset_];
    else
    {
        float s = 0;
        for(int i=0; i<info_->shape_.front(); i++)
            s += (*this)[i].sum();
        return s;
    }
}


float
matrix::average()
{
    if(empty())
        return 0;
    else
        return sum()/size();
}

}

