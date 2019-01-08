#include <cstdlib>
#include <iostream>
using namespace std;
int main(int,char**){
    int i=std::rand();
    if( i == 0 ) goto izero;
    cout<<" i nonzero";
    goto done;
izero:
    cout<<" i zero";
done:
    cout<<endl;
}

