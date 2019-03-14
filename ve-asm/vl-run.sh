#!/bin/bash
WORKING=6 # once, I had some errors so WORKING>=6 enables the full code
{
	make VERBOSE=1 -C.. force;
	make VERBOSE=1 WORKING=$WORKING veli_loadreg-x86 veli_loadreg-ve \
	&& rm -f tmp_veli* \
	&& ./veli_loadreg-x86 -a >& vl-a.log \
	&& echo "Next command, \"veli_loadreg-x86 -J\" will fail if you don't have ncc available" \
	&& ./veli_loadreg-x86 -J >& vl-j.log \
	&& ls -l tmp_veli* \
	&& ./veli_loadreg-ve -R \
	&& echo YAY \
	&& echo 'tmp_veli_loadreg_big.S' \
	&& cat tmp_veli_loadreg_big.S \
	&& echo 'tmp_veli_loadreg_big.dis' \
	&& cat tmp_veli_loadreg_big.dis \
	&& echo SUCCESS;
} >& vl-$WORKING.log
