# ACN Final Project

It is an academic project for CS6390 Advanced Computer Networks taught by Jorge A. Cobb (Summer 2017). The goal is to simulate a simple Ad Hoc network and implement OLSR protocol. See [requirements.md](https://github.com/nakhuang/Simulation-of-Ad-Hod-Network/blob/master/requirements.md), [Project_scenarios.pdf](https://github.com/nakhuang/Simulation-of-Ad-Hod-Network/blob/master/Project_scenarios.pdf) and [SampleScenario.txt](https://github.com/nakhuang/Simulation-of-Ad-Hod-Network/blob/master/SampleScenario.txt)for further details.

## UNIX Machine Uesd to Run the Project 

This project is run on one of campus VMs (csgrads1.utdallas.edu) with platform configuration: Linux – CentOS 7.4 x86_64 Dell PowerEdge R720 w/2 eight-core 2.2GHz Intel Xeon, 256GB RAM.

## Breif Intruduction

- The codes are written in C++ and compiled with g++ compiler.
- There are two execution files: "controller" and "node".
- Use shell script to launch one "controller" and several "node"s in background.
- The config file "topology.txt" is treated as a necessary input.
- There are two modes to compile and execute: normal mode and debugging mode.
-- Normal mode: Run programs without debugging messages.
-- Debugging mode: Run programs with debugging messages.

## Directories and Files Introduction

``` text
. [Root_of_Project]: the root directory of whole project
|-- controller: a directory with all source codes and the execution file of controller
    |-- main.cpp: the source code of main program of controller
    |-- Controller.h: the header file of Controller class
    |-- Controller.cpp: the implementation details of Controller class
    |-- Makefile: Makefile (default in normal mode)
    |-- Makefile_debug_version: Makefile in debugging mode
|-- node: a directory with all source codes and the execution file of node
    |-- main.cpp: the source code of main program of node
    |-- Node.h: the header file of Node class
    |-- Node.cpp: the implementation details of Node class
    |-- Makefile: Makefile (default in normal mode)
    |-- Makefile_debug_version: Makefile in debugging mode
|-- text: a directory with all fromX.txt, toX.txt and Xreceived.txt files (This will be created after running programs)
|-- log: a directory only for debugging (This will be created after running programs in debugging mode)
|-- "topology.txt": a text file with topology configuration
|-- "launcher.sh": an execution script to run "controller" and "node"s in background
|-- "launcher_debug_version.sh": an execution script to run "controller" and "node"s in background with debugging mode
```

## Compilation Instructions

### Step 1. Choose Mode

- To compile in normal mode:
-- No extra step for this.
- To compile in debugging mode:
-- Replace "Makefile" with "Makefile_debugging_version". Do it for both node and controller.

### Step 2. Compile Controller
 
- Enter the directory of controller by following command:
    ``` sh
    cd [Root_of_Project]/controller
    ```
- Compile with make command:
    ``` sh
    make
    ```
- As a result, there should be an execution file "controller" created under the directory of controller.

### Step 3. Compile Node

- Enter the directory of node by following command:
    ``` sh
    cd [Root_of_Project]/node
    ```
- Compile with make command:
    ``` sh
    make
    ```
- As a result, there should be an execution file "node" created under the directory of node.

## Execution Instructions

### Step 1

- Modify "topology.txt" and "launcher.sh" (or "launcher_debugging_version.sh") into the way which fits the scenario.
- More details about "launcher.sh", please see the attached sample "launcher.sh".
- Note that if the compilation is done in debugging mode, please use "launcher_debugging_version.sh".

### Step 2 

- Place files into right directories. 
- Make sure "topology.txt" and "launcher.sh" is under [Root_of_Project].
- Make sure execution file "controller" is under [Root_of_Project]/controller.
- Make sure execution file "node" is under [Root_of_Project]/node.
- See more details at "Directories and Files Introduction" part.
 
### Step 3

- Enter [Root_of_Project] by following command:
  ``` sh
  cd [Root_of_Project]
  ```
- Make sure "launcher.sh" is executable by command:
  ``` sh
  chmod +x launcher.sh
  ```

### Step 4

- Run "launcher.sh" by following command:
  ``` sh
  ./launcher.sh
  ```
- Then there should be some processes running in background.

### Step 5
- After about 120 seconds, all processes finish running.
- There should be all fromX.txt, toX.txt and Xreceived.txt created under [Root_of_Project]/text.

## More details about HELLO message

- A neighbor node who is in MPR set means it is also a bidiretional neighbor.
- For example, a HELLO message like: 
    ``` text
    * 0 HELLO UNIDIR 1 2 BIDIR 3 4 MPR 5 6
    ```
- Note that node 5 and 6 only appear in MPR (not in BIDIR).
- From this message, we know that node 0 has bidirectional neighbors 3, 4, 5, and 6.
