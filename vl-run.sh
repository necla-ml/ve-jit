#!/bin/bash
{
	make force;
make veli_loadreg-x86 veli_loadreg \
	&& rm -f tmp_veli* \
	&& ./veli_loadreg-x86 -a >& vl-a.log \
	&& ./veli_loadreg-x86 -J >& vl-j.log \
	&& ls -l tmp_veli* \
	&& ./veli_loadreg -R \
	&& echo YAY \
	&& cat tmp_veli_loadreg_big.dis \
	&& echo SUCCESS;
} >& vl-06.log
