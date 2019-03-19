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
$NODEDIR/node 0 1 "message from 0" 50 &> $LOGDIR/0.txt &
$NODEDIR/node 1 1 &> $LOGDIR/1.txt &
$NODEDIR/node 2 2 &> $LOGDIR/2.txt &
$NODEDIR/node 3 2 "message from 3" 90 &> $LOGDIR/3.txt &
$NODEDIR/node 4 4 &> $LOGDIR/4.txt &
#$NODEDIR/node 5 5 &> $LOGDIR/5.txt &
$NODEDIR/node 6 6 &> $LOGDIR/6.txt &
$NODEDIR/node 7 7 &> $LOGDIR/7.txt &
$NODEDIR/node 8 8 &> $LOGDIR/8.txt &
$NODEDIR/node 9 2 "message from 9" 25 &> $LOGDIR/9.txt &
$CONTRDIR/controller &> $LOGDIR/c.txt &

