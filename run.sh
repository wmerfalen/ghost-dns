#!/bin/bash
#
export LD_PRELOAD=$PWD/ghost.so 
$@
