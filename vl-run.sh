#!/bin/bash
WORKING=6
{
	make VERBOSE=1 force;
	make VERBOSE=1 WORKING=$WORKING veli_loadreg-x86 veli_loadreg \
	&& rm -f tmp_veli* \
	&& ./veli_loadreg-x86 -a >& vl-a.log \
	&& ./veli_loadreg-x86 -J >& vl-j.log \
	&& ls -l tmp_veli* \
	&& ./veli_loadreg -R \
	&& echo YAY \
	&& echo 'tmp_veli_loadreg_big.S' \
	&& cat tmp_veli_loadreg_big.S \
	&& echo 'tmp_veli_loadreg_big.dis' \
	&& cat tmp_veli_loadreg_big.dis \
	&& echo SUCCESS;
} >& vl-$WORKING.log
