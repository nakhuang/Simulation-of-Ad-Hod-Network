#!/bin/bash

#
# Setup directory of debugging log files
#
LOGDIR=log
rm -rf $LOGDIR
mkdir $LOGDIR

#
# Setup directory of text files
#
TXTDIR=text
rm -rf $TXTDIR
mkdir $TXTDIR

#
# Directory of controller execution file
#
CONTRDIR=controller

#
# Directory of node execution file
#
NODEDIR=node

#
# Sample executions in Scenario 2 given in Project_scenarios.pdf
#
$NODEDIR/node 0 1 "message from 0" 50 &
$NODEDIR/node 1 1 &
$NODEDIR/node 2 2 &
$NODEDIR/node 3 2 "message from 3" 90 &
$NODEDIR/node 4 4 &
#$NODEDIR/node 5 5 &
$NODEDIR/node 6 6 &
$NODEDIR/node 7 7 &
$NODEDIR/node 8 8 &
$NODEDIR/node 9 2 "message from 9" 25 &
$CONTRDIR/controller &

