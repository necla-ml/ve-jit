#!/bin/awk -f
#
# output performance tests from benchdnn in increasing avg_Gops order
# assumes gawk 4.x
#
BEGIN {FS=","}
/^perf/ { perfline1[++n]=$0; best_Gops[n]=$6; avg_Gops[n]=$8; }
/^ prim/ { perfline2[n]=$0; fullspec[n]=$6}
END{
	PROCINFO["sorted_in"]="@val_num_asc";
	for(i in avg_Gops) {
		printf("\n%-8.3f avg Gops in perf test %-4u    %s\n",avg_Gops[i],i,fullspec[i])
		print(perfline1[i]);
		print(perfline2[i]);
	}
}
