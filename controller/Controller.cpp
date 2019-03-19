#include "Controller.h"
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

// Directory with fromX.txt and toX.txt files
#define FILE_DIR "text/"

// Maximum number of nodes
#define MAX_NUM_NODES 10

using namespace std;

// When constructing memeber ps, a Parser instance, pass the reference to itself.
Controller::Controller() : topology_next_start_it_(NULL), parser_(*this) {
    for (int i=0; i<MAX_NUM_NODES; i++) {
        position_.push_back(0);
        
        for (int j=0; j<MAX_NUM_NODES; j++)
            adjacency_matrix_[i][j] = false;
    }
    
    topology_.clear();    
}

// Parse topology.txt through Parser.
void Controller::ParseTopology() {
    string filename("topology.txt");
    parser_.ParseFile(filename);
}

// Start processing messages from files.
void Controller::StartProcess() {
    // Timer ticks every second in real time.
    int timer = 0;

    // The process lasts for 120 seconds.
    // Run one round per second by using sleep().
    while (timer < 120) {
        // Update adjacency metrix of the links between nodes.
        UpdateTopology(timer);

#ifdef DEBUG
        cout << "[time: " << timer << "]" << endl << "Topology: \n";
        for(int i=0; i<MAX_NUM_NODES; i++){
            for(int j=0; j<MAX_NUM_NODES; j++)
                cout << adjacency_matrix_[i][j] << " ";
            cout << endl;
        }
#endif

        // Each round, the controller check all fromX.txt.
        // Assumption: 
        //   - There are at most 10 nodes.
        //   - The IDs of nodes are 0 to 9.
        // If there are new messages, process them properly.
        for (int i=0; i<10; i++) {
            string file = "from" + itos(i) + ".txt";
            char name[100];
            strcpy(name, FILE_DIR);
            strcat(name, file.c_str());
            string filename(name);

            parser_.ParseFile(filename);
        }

        sleep(1);
        ++timer;
    }

}

// Put one entry of topology.txt into vector topo.
void Controller::PutTopologyEntry(int time, bool activation, int head, int tail) {
    TopologyEntry data;
    data.time = time;
    data.activation = activation;
    data.head = head;
    data.tail = tail;
    topology_.push_back(data);
}

// Update topology, i.e. update adjacency matrix of links of nodes.
void Controller::UpdateTopology(int timer) {
    
    // Set iterator to the first entry that hasn't been processed.
    vector<TopologyEntry>::iterator it;
    it = topology_next_start_it_;

    // Check if there are entries need to be processed.
    // If so, process it, i.e. update adjacency matrix.
    while (it != topology_.end() && it->time <= timer) {
        adjacency_matrix_[ it->head ][ it->tail ] = it->activation;
        ++it;
    }

    topology_next_start_it_ = it;

#ifdef DEBUG    
    cout << "! topology_next_start_it_: " ;
    if(topology_next_start_it_ != topology_.end())
        cout << topology_next_start_it_->time << " "
         << topology_next_start_it_->activation << " "
         << topology_next_start_it_->head << " -> "
         << topology_next_start_it_->tail << endl;
    else
        cout << "topology_.end()" << endl;
#endif

}

// Convert int to string
string Controller::itos(int i) {
    string ret;
    stringstream str_stream;
    str_stream << i;
    ret = str_stream.str();
    return ret;
}

#ifdef DEBUG
// Print out entire vector topology_.
void Controller::PrintTopology() {
    vector<TopologyEntry>::iterator it;
    for (it = topology_.begin(); it != topology_.end(); ++it) {
        cout << it->time << " " << it->activation << " " << it->head << " -> " << it->tail << endl;
    }
}
#endif

// Construct Parser with reference to outer class Controller.
Controller::Parser::Parser(Controller &c) : controller_(c) {}

// Open and parse file whose name is filename.
// Return false if it fails to open the file; otherwise, return true;
bool Controller::Parser::ParseFile(string filename) { 

    inputfile_.open(filename.c_str(), ios::in);
    if(!inputfile_.is_open()){
#ifdef DEBUG
        cerr << "Failed: Open file \"" << filename << "\"" << endl;
#endif
        return false;
    }

    // Identify the input file.
    if(filename == "topology.txt"){
        ParseTopology();
    }
    else{
        ParseFromX(filename);
    }

    inputfile_.close();
    return true;
}

// Parse topology.txt, store topology information into vector topology_.
void Controller::Parser::ParseTopology() {
    
    string line;
    
    // Parse the file line by line and put entries into topology_.
    while (getline(inputfile_, line)) {
        stringstream strstream(line);
        
        int time, head, tail;
        bool activation;
        string activation_str;

        strstream >> time >> activation_str >> head >> tail;
        activation = (activation_str == "UP") ? true : false;
        controller_.PutTopologyEntry(time, activation, head, tail);
    }

    controller_.topology_next_start_it_ = controller_.topology_.begin();
}

// Parse fromX.txt
void Controller::Parser::ParseFromX(string filename) {
    // Get node id from filename.
    char dir[100];
    strcpy(dir, FILE_DIR);
    filename.erase(0, strlen(dir));  // remove DIR
    filename.erase(0, 4);           // remove "from"
    filename.erase(filename.end()-4, filename.end());    // remove ".txt"
    int id = stoi(filename);

#ifdef DEBUG
    cout << "* Processing \"from" << id << ".txt\": " << endl;
#endif

    // Parse the file from where hasn't been parsed 
    // and keep record of the last byte that has been parsed.
    string line;
    streampos last_byte_parsed;
    inputfile_.seekg(controller_.position_[id]);
            
    while (getline(inputfile_, line)) {
#ifdef DEBUG
        cout << "- Processing msg: " << line << endl;
#endif
        last_byte_parsed = inputfile_.tellg();
        
        // Dealing with new messages sent from node id.
        // Controller only copy messages from "fromX.txt" to "toX.txt" properly.
        // For each message ("line" here), controller do the following steps: 
        //  1. Find out where to sent.
        //      - to all neighbors
        //      - to a specific neighbor
        //  2. Copy the message and paste it to proper "toX.txt".

        // Send to all neighbors of node id.
        if (line[0] == '*'){
            for(int i=0; i<MAX_NUM_NODES; i++){
                if(i == id || !controller_.adjacency_matrix_[id][i]) continue;
                
                string name = "to" + itos(i) + ".txt";
                char filename[100];
                strcpy(filename, FILE_DIR);
                strcat(filename, name.c_str());
                
                ofstream outputfile;
                outputfile.open(filename, ios::app);
                if(!outputfile.is_open()){
#ifdef DEBUG
                    cerr << "Failed: Open file \"" << filename << "\"" << endl;
#endif
                    return;
                }

                outputfile << line << endl;
                outputfile.close();
            }
        }
        // Send to a specific node.
        else {
            stringstream str_stream(line);
            int sent_to;
            str_stream >> sent_to;
            
            if(sent_to != id && controller_.adjacency_matrix_[id][sent_to]) {   
                string name = "to" + itos(sent_to) + ".txt";
                char filename[100];
                strcpy(filename, FILE_DIR);
                strcat(filename, name.c_str());
                
                ofstream outputfile;
                outputfile.open(filename, ios::app);
                if(!outputfile.is_open()){
#ifdef DEBUG
                    cerr << "Failed: Open file \"" << filename << "\"" << endl;
#endif
                    return;
                }

                outputfile << line << endl;
                outputfile.close();
            }
        }
    }

    controller_.position_[id] = max(last_byte_parsed, controller_.position_[id]);
#ifdef DEBUG
    cout << "! position_[" << id << "]: " << controller_.position_[id] << endl;
#endif
}
