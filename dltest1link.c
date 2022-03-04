/* Copyright (c) 2019 by NEC Corporation
 * This file is part of ve-jit */
#include <stdio.h>
extern int myLuckyNumber();
int main(int argc,char**argv){
	int lucky = myLuckyNumber();
	printf(" myLuckyNumber is %d\nGoodbye\n",lucky);
}
