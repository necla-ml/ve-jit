/** \file
 * print aurora register names array */
#include "reg-aurora.hpp"
#include <iostream>
#include <array>
#include <cstring>
using namespace std;
int main(int argc, char**argv){
    cout<<"static char const* aurora_regnames =";

    // some registers get special names
    constexpr int maxexc = 7;
    constexpr array<int,maxexc>         altreg =
    {  8,    9,    10,   11,   14,   15,    16 };       // increasing RegIds
    constexpr array<char const*,maxexc> altnam =
    { "sl", "fp", "lr", "sp", "tp", "got", "plt" };     // their alt names

    int exc = 0;
    for(int i=IDscalar; i<=IDscalar_last; ++i){
        cout<<(i%5==0?"\n    ":" ");
		if( exc < maxexc && i == altreg[exc] ){
			cout<<" \"%"<<altnam[exc];
			int n = 5 - strlen(altnam[exc]);
			for(int z=0; z<n; ++z) cout<<"\\0";
                        cout<<"\"";
			++exc;
		}else{
            cout<<" \"%s" << i
				<<(i<10?"\\0\\0\\0\"":"\\0\\0\"");
		}
    }
    cout<<"\n   ";
    for(int i=IDvector; i<=IDvector_last; ++i){
        int const n = i - IDvector;
        cout<<(n%5==0?"\n    ":" ")
            <<" \"%v" << n
            <<(n<10?"\\0\\0\\0\"":"\\0\\0\"");
    }
    cout<<"\n   ";
    for(int i=IDvmask; i<=IDvmask_last; ++i){
        int const n = i - IDvmask;
        cout<<(n%5==0?"\n    ":" ")
            <<" \"%vm" << n
            <<(n<10?"\\0\\0\"":"\\0\"");
    }
    cout<<"\n    ;"<<endl;
}
// vim: ts=4 sw=4 et cindent cino=^=l0,\:.5s,=-.5s,N-s,g.5s,h.5s,b1 cinkeys=0{,0},0),\:,0#,!^F,o,O,e,0=break
