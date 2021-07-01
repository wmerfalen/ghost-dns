#!/bin/bash
#
export LD_PRELOAD=$PWD/build/ghost.so 
$@
