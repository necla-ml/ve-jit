#!/bin/bash
# Example:
#   ./cf5.sh 8 0:2:8:10 jj PRINT 2
# VL --------^
# loop split --^ the first and last two loop over ii,
# kernel PRINT ------------^
# unroll by ---------------------^
# 
echo "cf5.sh args: $*"
./cf.sh cf5 $*
