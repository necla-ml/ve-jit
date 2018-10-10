/** add 256 doubles <em>in groups of 16</em> */
void grpsum(double* d, double* s){
	for(int j=0; j<16; ++j){
		s[j] = 0.0;
	}
	for(int j=0; j<16; ++j){
		for(int i=0; i<16; ++i){
			s[j] += d[j*16+i];
		}
	}
}
void grpsum2(double* d, double* s){
	s[0] = d[0];
	for(int i=1; i<256; ++i){
		if(i%16){
			s[i] = s[i-1] + d[i];
		}else{
			s[i] = d[i];
		}
	}
}
void grpsum3(double* d, double* s){
	double tmp = 0.0;
	s[0] = tmp;
	for(int i=1; i<256; ++i){
		tmp = (i%16? s[i-1]: 0.0);
		s[i] = tmp + d[i];
	}
}
// NOT able to get compile-time mask register
int const msk0[256]={[0]=1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1};
int const msk1[256]={[16]=1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,1};
void grpsum4(double* d, double* s){
	double tmp =0.0;
	for(int i=0; i<256; ++i){
		if(msk0[i]) tmp += d[i];
	}
	s[0] += tmp;
	tmp =0.0;
	for(int i=0; i<256; ++i){
		if(msk1[i]) tmp += d[i];
	}
	s[1] += tmp;
}

// NOT able to get lvm
unsigned long grp16[4]=
{ 0x000000000000ffffUL,
	0x00000000ffff0000UL,
	0x0000ffff00000000UL,
	0xffff000000000000UL };
void grpsum5(double* d, double* s){
	long msk[4];
	for(int i=0; i<4; ++i){
		msk[i] = 0;
	}
	msk[0] = grp16[0];
	double tmp1= 0.0;
	for(int i=0; i<256; ++i){
		if(msk[i/64] & (1<<i%64)){
			tmp1 += d[i];
		}
	}
	s[0] = tmp1;
}

// vim: ts=2 sw=2 cindent
