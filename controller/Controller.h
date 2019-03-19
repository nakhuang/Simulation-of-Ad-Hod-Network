#ifndef ACN_PROJECT_CONTROLLER_H
#define ACN_PROJECT_CONTROLLER_H

#define MAX_NUM_NODES 10

#include <fstream>
#include <string>
#include <vector>

class Controller {
public:
    Controller();

    // Parse topology.txt.
    void ParseTopology();

    // Start processing messages from files.
    void StartProcess();

    // A useful function: converting int to string.
    static std::string itos(int i);

#ifdef DEBUG
    // Print out entire vector topo
    void PrintTopology();
#endif

private:
    // an adjcency matrix to represnt the links between nodes
    bool adjacency_matrix_[MAX_NUM_NODES][MAX_NUM_NODES];
    
    // a data structure for the entry of topology.txt
    //   - time: the time when this actication/deactivation happens
    //   - head: node id of the link head 
    //   - tail: node id of the link tail (ex. a link from X to Y --> head: X, tail: Y)
    //   - activation: activation or deactivation of the link. true: activation; false: decativation.
    struct TopologyEntry {
        int time, head, tail;
        bool activation;
    };

    // used to store all entries of topology.txt
    std::vector<TopologyEntry> topology_;

    // the iterator which indicates the start entry for next time to check
    std::vector<TopologyEntry>::iterator topology_next_start_it_;

    // a class for parsing files
    class Parser {
        public:
            // Initialize with reference to outer class Controller
            Parser(Controller &c);

            // Open and parse the file whose name is filename.
            bool ParseFile(std::string filename);

        private:
            // reference to outer class Controller
            Controller &controller_;

            // input filestream of input file
            std::ifstream inputfile_;

            // implementation of parsing topology.txt
            void ParseTopology();

            // implementation of parsing fromX.txt, where X is the node id
            void ParseFromX(std::string filename);
    };

    // instance of class Parser
    // Each instance of Controller has a instance of Parser.
    Parser parser_;

    // a vector of stream positions, each node has one stream position
    std::vector<std::streampos> position_;

    // Put one entry of topology.txt into vector topo
    void PutTopologyEntry(int time, bool activation, int head, int tail);

    // Update topology information, i.e. update adjacency matrix of links of nodes.
    void UpdateTopology(int timer);

};

#endif
