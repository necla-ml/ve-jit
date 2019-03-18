/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */

#include "spill2-impl.hpp"
#include <iostream>

//=========================================== template instantiation
namespace ve{

    using namespace std;
void spill_msg_destroy(int bottom)
{
    cout<<" ~Spill(bottom="<<bottom<<") ";
    cout.flush();
}

//template class Spill<SymbStates<>>;

}//ve::
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
