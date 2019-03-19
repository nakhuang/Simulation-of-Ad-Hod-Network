# CS 6390 Summer 2017 Programming Project

NOTE: there are always typos/omissions/inconsistencies, etc., and probably lots
of them, so please help me find them. Also, please read it ASAP and start
working on the code.  It may take longer than you think.

NOTE: This is INDIVIDUAL work. People sharing code will be sent to the Dean of
Students for disciplinary action.

NOTE: YOU CANNOT BORROW CODE FROM THE WEB. Any code has to be originally yours. 

## System/Language requirements

Your program can be written in any language that you wish, the only restriction
is that it must run in the Unix machines here on campus (no, not on your
laptop). Most, of course, prefer Java ad C++, but I have had projects turned in
Perl and in Pascal (no kidding).

When you turn in your project, you can zip together all the source files. In
addition, you should include a "README" file that contains:

1. The name of the Unix machine you used to run your project
2. Compilation instructions for your project (PRECISE compilation instructions,
   including the compiler name and any flags as arguments to the compiler)
3. Obviously your name and ID :)

We will compile your source code and run it against our test cases.

The due date is to be determined, but it will be around one week before the
final grades are due.

## Overview

### Processes and arguments

We will simulate a very simple network by having a process correspond to a node
in the network, and files correspond to channels in the network.

We will have at most 10 nodes in the network, nodes 0, 1, 2, . . . , 9

Each process is going to be given the following arguments

1. id of this node (i.e., a number from 0 to 9)
2. the destination id of a process to which the node should send data
3. a string of arbitrary text that the node will send to the destination
4. the time at which it should transfer the string to the destination 

We will have a __single program__ node.c (or whatever extension you want to
use, node.cc, node.java, whatever) which has the code for a node. Since we have
multiple nodes, we will run the same program multiple times, in parallel. The
only difference between each of the copies of the program running in
__parallel__ is the arguments to the program.

For example, assume I have two programs, A and B, and I want to run them at the
same time. At the Unix prompt >, I would type the following

    > A &

    > B &

By typing the & we are putting the program in the "background", i.e., the
program runs in the background and you can keep typing things at the terminal.
Therefore, A and B are running in parallel at the same time.

Again, let "node" be your compiled program that represents a node. The
arguments of the program are as follows

    node 3 5 "this is a message" 25 &

The following would execute the program node, and the first argument is the id
of the node (3) the second is the destination for this node (5) the third is
the message string "this is a message", and the fourth is the time at which the
node begins attempting to transmit the string to the destination (i.e. after 25
seconds of execution).

For example, assume I have a network with three nodes, 0 , 1, 2, and I want
node 0 to send a string "this is a message from 0"to node 2 (starting at time
10), and node 1 to send a message"this is a message from 1" to node 2 (starting
at time 20). Then I would execute the following commands at the Unix prompt >
(your prompt may be different)

    > node 0 2 "this is a message from 0" 10 &

    > node 1 2 "this is a message from 1" 20 &

    > node 2 2 &

This will run three copies of node in the background, the only difference
between them are the arguments each one has.

For node 2, since the "destination" is 2 itself, this means 2 should not send a
string to anyone.

There will be a single additional process, called __controller__. The
controller is needed because of the way that we handle I/O, as will be
discussed below.

All processes must be started at the same time. Since the above takes a while
to type, you can write a small shell script to quickly spawn all the above
processes. A sample shell script will be provided.

### Communication Model and Files

We will model a wireless network (well, it is of course a far cry from a
wireless network but nonetheless . . . ). I will assume that when a node sends
a message all of its neighbors are able to hear the message (since it is a
broadcast medium).

Each node with ID X will open two text files: toX.txt and fromX.txt. When X
sends a message it ___appends___ it to fromX.txt (it does not overwrite any
previous content of the file). Hence, at the end of the execution, fromX.txt
contains all the messages sent by X.

Messages are separated by a newline (to make viewing/parsing easier)

A node is not aware of who its neighbors are. The “controller” is the process
that is aware of the entire topology of the system. In real life the controller
would not exist, but since we are using files rather than a real wireless
network, the controller will act like the wireless network.

If the controller knows there is a link from X to Y (i.e., X and Y are
neighbors, WARNING, links can be one-directional) then when X appends a message
to fromX.txt, the controller will read fromX.txt, notice the new message, and
copy the message to the file toY.txt.

Thus, node X receives in the file toX.txt a copy of every message sent by any
of its neighbors.

How does the controller know who is a neighbor of whom? Before the execution,
you will create by hand a file called topology.txt. This file contains multiple
lines, each line is of the following form.

    time status node1 node2

Each line corresponds to the "activation" or "deactivation" of the
one-directional link (node1 --> node2). For example, if 10 seconds after the
system begins to execute node X and Y become close to each other (and become
bidirectional neighbors) then the topology.txt file will have the following
lines (among others)

    10 UP X Y 
    10 UP Y X

Where X and Y are numbers in the range 0 .. 9 (i.e. node ID’s)

If at time 20 X and Y become far away from each other then topology.txt will
have the following lines (again, among others)

    20 DOWN X Y
    20 DOWN Y X

You can assume the entries in topology.txt are sorted by time. Also, it is
possible that the link from X to Y remains up while the link from Y to X goes
down.

__ONCE EVERY SECOND__,  the controller will read __ALL__ fromX.txt files (for
every X 0 .. 9) to check if there are new messages sent, and if so the messages
are forwarded to the appropriate toX.txt files.

__ONCE EVERY SECOND__ each node X will read the toX.txt file to see if there
are any new messages to process.

# Routing

Routing of messages from one node to another will be performed using the OLSR
protocol. I will not include all details here since you know how OLSR behaves,
but I will provide some of the highlights.

There will be three types of messages:

- `HELLO` - These messages are the hello messages exchanged between neighbors

- `TC` - These are the topology control messages that are flooded throughout
  the network

- `DATA` - These are data messages that are  sent from a source to a destination.

When a message is written into the file fromX.txt by node X, they are preceded
by the node id of the next-hop node that should receive this message.

E.g., consider DATA messages, they are of the form

    <nxthop>  <fromnbr> DATA <srcnode> <dstnode> <string>

One example of a data message would be

    2 1 DATA 0 3 This is a message

This message is being sent from node 1 to node 2 (i.e. along the link 1 -- 2),
and it is a data message which originated at node 0 and whose ultimate
destination is node 3. The data in the message is “This is a message”.

The other two type of messages, HELLO and TC, are always sent to all neighbors,
rather than a specific neighbor. In this case, we will use `*` to denote all
neighbors, as indicated later below.

## Creating and Routing Data Messages.

### Creating data messages.

After the appropriate number of seconds (i.e., the last argument of the
program) have elapsed since the beginning of the execution, then the node
attempts to send the string to the destination.
 
Each node maintains a routing table and a topology table as described in the
slides. At all times, the routing table is maintained current with respect to
the topology table. I.e., whenever the topology table changes the node
recalculates all the entries in its routing table.

If the node has an entry in its routing table for the destination, then it
creates the data message and sends it to the next hop to the destination, as
indicated in its routing table. (i.e. it puts the string in the data message
and sends it to the next hop).
 
If an entry is not found in the routing table, the node waits for 30 seconds,
and then it tries again. It keeps trying every 30 seconds until it finds an
entry in the routing table. Once the data message is successfully created and
sent, it is not created again, i.e., we will only send the string of text once
to the destination.

If node X receives a data message that has been addressed to it (i.e., X is the
destination) then X appends the string in the data message to the file
Xreceived.txt. I.e., this file will have all the data strings that X received
during its execution.
 
### Routing of data messages

Nothing interesting here. When a node receives a data message, and it is not
the destination, it has to forward it according to its routing table. If it
does have an entry in its routing table for the destination it forwards the
message accordingly. If  it does not have an entry in its routing table for the
destination,  it just drops the message.

## Hello Message Creation and Processing

A `HELLO` message has the following format

    *  <node> HELLO UNIDIR <neighbor> ... <neighbor> BIDIR <neighbor> ... <neighbor> MPR <neighbor> ... <neighbor>

`<node>` is the id of the node that is sending the hello message.

`*` indicates that this message will be sent to all neighbors of `<node>`,
i.e., the controller will forward a copy of this message to all neighbors of
`<node>`. 

`UNIDIR` is a list of neighbors that the node thinks (so far) there is only a
one-directional link to the neighbor

`BIDIR` is a list of neighbors that the node has confirmed that it has a
bidirectional link with each of the neighbors.

`MPR` is a list of neighbors that the node has chosen as multipoint relays.

Each node sends a hello message every 5 seconds.

The MPRs are selected using the basic OLSR greedy strategy: select first the
neighbor that can reach the largest number of two hop neighbors, then the next
neighbor that can select the most remaining two-hop neighbors, etc, as
described in class. 

If a node does not receive a hello message from one of its neighbors in 15
seconds, it assume the neighbor is dead or has moved away, so it removes it
from its list of neighbors.

## TC  Message Creation and Processing

The format of a TC message is as follows.

    * <fromnbr> TC <srcnode> <seqno> MS <msnode> ... <msnode> 

Recall that TC messages are flooded throughout the entire network. Thus,
`<fromnbr>` is the node transmitting this particular copy of the message, which
will be received by all neighbors of this node. The originator of the TC
message is the  node `<srcnode>`. The `<seqno>` is the sequence number of the
srcnode of the TC message  (recall that each node when it creates a TC message
it increases its local sequence number and places it in the TC message it
creates). MS is a sequence of MPR selectors of the node `<srcnode>`

Each node maintains a topology table and a routing table as described in the
slides.

TC messages are flooded as described in the slides. I.e., when a neighbor x of
`<fromnbr>` receives the above message, it will forward it to all its neighbors
if x is an MPR of `<fromnbr>`. When x retransmits the message, it of course
updates `<fromnbr>` to be itself.

The sequence number determines whether the information is old or new. I.e., if
you have information from `<srcnode>` whose sequence number is smaller than
that of this message, then you throw the old information away (if different)
and keep the new one with larger sequence number. If the information in the
message has a smaller sequence number than what you have seen before from
`<srcndoe>` you just throw the message away. If you receive the same
information but with a larger sequence number, you refresh the timeout period
of the information (see below)

Every node with a non-empty MS sets creates and floods a TC message every 10
seconds.

If you do not receive a TC message from a node during a period of 45 seconds
you remove all TC information that you received from that node.

# Program Skeleton

You can write your “node” program with an overall general structure as follows
(of course details have to be added by you, and I don't claim that the below
structure is complete). You can figure out the controller on your own.

    main()
        initialize variables
        open fromX.txt for appending, toX.txt for reading,
            and Xreceived for appending.
        i = 0
        while (i < 120)
            read toX.txt
            process any new received messages  (i.e.  DATA, HELLO, TC)
            if it is time to send the data string
                if there is a routing table entry for the destination
                    send the data message
            if i is a multiple of 5 send a hello message
            if i is a multiple of 10 send a TC message
            remove old entries of the neighbor table if necessary
            remove old entries from the TC table if necessary
            recalculate the routing table if necessary
            i = i + 1;
            sleep for 1 second.
        end while
        close files
        end program


# General Remarks

__DO NOT RUN YOUR PROGRAM WITHOUT THE SLEEP COMMAND.__ Otherwise you would use
too much CPU time and administrators are going to get upset with you and with
me!

___Notice that your process will finish within 120 seconds (or about) after you
started it.___

Note that you have to run multiple processes in the background. The minimum are
two processes that are neighbors of each other, of course.

After each "run", you will have to delete the channel and output files by hand
(otherwise their contents would be used in the next run, which of course is
incorrect).

Also, after each run, __you should always check that you did not leave any
unwanted processes running, especially after you log out !!!__ To find out
which processes you have running, type

    ps -ef | grep userid

where `userid` is your Unix login id. (mine is jcobb). That will give you a
list of processes with the process identifier (the process id is the large
number after your user id in the listing)

To kill a process type the following

    kill -9 processid

I will give you soon a little writeup on Unix (how to compile, etc) and account
information. However, you should have enough info by now to start working on
the design of the code

Good luck.

