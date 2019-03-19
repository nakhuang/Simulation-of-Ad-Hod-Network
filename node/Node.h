#ifndef ACN_PROJECT_NODE
#define ACN_PROJECT_NODE

#include <set>
#include <string>
#include <map>
#include <fstream>

#define MAX_NUM_NODES 10

class Node {
public:
    Node(int id, int dst_id, std::string str_msg, int time);

    Node(int id, int dst_id);

    Node();

    // Start running process for 120 seconds.
    void StartProcess();

    // Convert int to std::string.
    static std::string itos(int i);

private:
    // execution arguments:
    //  - id_: node id of itself
    //  - dst_id_: node id of destination of string message
    //  - str_msg_: content of string message
    //  - time_: amount of time, the node start trying to send string message after this amount of time
    int id_, dst_id_, time_;
    std::string str_msg_;

    // stream position to record where toX.txt has been read
    std::streampos position_;
    
    // sequence number of MS set
    int ms_seq_;

    // last time receive HELLO or TC from neighbors, default value is -1
    int last_hello_[MAX_NUM_NODES];
    int last_tc_[MAX_NUM_NODES];

    // previous HELLO message for each neighbor
    std::string prev_hello_[MAX_NUM_NODES];

    // latest MS sequence number received from each node
    int latest_seq_[MAX_NUM_NODES];

    // MPR set
    std::set<int> MPR_set_;
    
    // MS set
    std::set<int> MS_set_;

    // one-hop neighbor set
    std::set<int> one_hop_neighbor_set_;

    // two-hop neighbor set (one-hop neighbors are NOT included)
    std::set<int> two_hop_neighbor_set_;

    // one-hop neighbor table
    enum LinkType { NA, UNIDIR, BIDIR, MPR };
    LinkType neighbor_table_[MAX_NUM_NODES];

    // two-hop neighbor table (one-hop neighbors ARE included)
    // ex. two_hop_nei_access_thr_[i] is a set with the nodes where to reach i, you can access through
    std::set<int> two_hop_nei_access_thr_[MAX_NUM_NODES];

    // reverse version of two-hop neighbor table (one-hop neighbors ARE included)
    // ex. from_one_hop_[i] is a set with the nodes where from node i can reach and the distances between i are one hop
    std::set<int> from_one_hop_[MAX_NUM_NODES];

    // TC table
    std::set<int> tc_table_[MAX_NUM_NODES];

    // reverse version of TC table
    // ex. thr_last_hop_[i] is a set of nodes that from node i can reach to, and node i is the last hop to the nodes.
    std::set<int> thr_last_hop_[MAX_NUM_NODES];

    // routing table
    struct RoutingTableEntry {
        int next_hop, distance;
    };
    std::map<int, RoutingTableEntry > routing_table_;

    // Initialize values.
    void init();

    // generator of file name, return file name in string type
    std::string FilenameGenerator(std::string type);

    // Process toX.txt. Return true if there are new messages; otherwise, return false.
    bool ProcessToX(int time);

    // Calculate routing table.
    void CalculateRoutingTable();

    // Handle incoming HELLO message.
    bool HandleHelloMsg(std::string msg, int time);

    // Handle incoming TC message.
    bool HandleTcMsg(std::string msg, int time);

    // Handle incoming DATA message.
    void HandleDataMsg(std::string msg);

    // Remove node i from neighbor table(s).
    void RemoveNeighbor(int i);

    // Remove TC information from node i.
    void RemoveTcInfoFrom(int i);

    // Calculate and update MPR set.
    void CalculateMprSet();

    // Receive message, i.e. append string into Xreceived.txt.
    void ReceiveMessage(std::string msg);

    // Semd message, i.e. append string into formX.txt.
    void SendMessage(std::string msg);

    // Send HELLO message.
    void SendHelloMsg();

    // Send TC message.
    void SendTcMsg();

    // Send DATA message.
    void SendDataMsg(int next_hop);

    // Forward TC message.
    void ForwardTcMsg(std::string msg, std::string fromnbr);

    // Forward DATA message.
    void ForwardDataMsg(std::string msg, std::string nxthop, std::string fromnbr, int next_hop);

    // Generate content of whole HELLO message.
    std::string GenerateHelloMsg();

    // Generate content of whole TC message.
    std::string GenerateTcMsg();

    // Generate content of whole DATA message.
    std::string GenerateDataMsg(int next_hop);

};

#endif
