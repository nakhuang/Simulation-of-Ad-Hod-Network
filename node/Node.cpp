#include "Node.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

// Directory with txt files
#define FILE_DIR "text/"

#define MAX_NUM_NODES 10

using namespace std;

Node::Node(int id, int dst_id, string str_msg, int time) : 
            id_(id), dst_id_(dst_id), str_msg_(str_msg), time_(time) {
    init();
}

Node::Node(int id, int dst_id) : id_(id), dst_id_(dst_id) {
    str_msg_ = "";
    time_ = -1;
    init();
}

void Node::init() {
    position_ = 0;
    ms_seq_ = 0;
    for (int i=0; i<MAX_NUM_NODES; i++) {
        last_hello_[i] = -1;
        last_tc_[i] = -1;
        prev_hello_[i] = "";
        latest_seq_[i] = 0;
        neighbor_table_[i] = NA;
    }
}

// Start running process for 120 seconds.
// It is the main protion of a Node.
void Node::StartProcess() {
#ifdef DEBUG
    cout << id_ << " " << dst_id_ << " " << str_msg_ << " " << time_ << endl;
    cout << "seq: " << ms_seq_ << endl;
#endif

    // Timer ticks every second in real time.
    int timer = 0;

    // Use flag to indicate if string message is sent or not.
    bool data_sent = false;

    // Record the next time trying to send string message.
    int time_to_try = time_;

    // Use flag to indicate if routing table need to be recalculated,
    bool recalculate_routing_table = false;

    // The process lasts for 120 seconds.
    // Run one round per second by using sleep(). 
    while (timer < 120) {
#ifdef DEBUG
        cout << "\n[time: " << timer << "]" << endl;
#endif
        
        recalculate_routing_table = false;

        // Every second, Node read its toX.txt to see if there are new messages to process.
        recalculate_routing_table |= ProcessToX(timer);

        // 1. Update neighbor table if necessary -> done in ProcessToX
        // 2. Update TC table if necessary -> done part in HandleTcMsg and part in below
        // 3. Recalculate routing table if necessary -> done below

        // Check last_tc_[i] for all node i. 
        // If have not receive TC for 45 seconds, remove TC information from node i.
        for (int i=0; i<MAX_NUM_NODES; ++i) {
            if (timer - last_tc_[i] >= 45 && !thr_last_hop_[i].empty()) {
                RemoveTcInfoFrom(i);
                recalculate_routing_table |= true;
            }
        }

        // Recalculate routing table if necessary.
        if (recalculate_routing_table) {
            CalculateRoutingTable();
        }

#ifdef DEBUG
        cout << "/** last_hello **/ ";
        for(int i=0; i<MAX_NUM_NODES; ++i) cout << "(" << i << ", " << last_hello_[i] << ") ";
        cout << "\n/** last_tc **/ ";
        for(int i=0; i<MAX_NUM_NODES; ++i) cout << "(" << i << ", " << last_tc_[i] << ") ";
        
        cout << "\n/** Routing Table **/" << endl;
        for(map<int, RoutingTableEntry>::iterator it = routing_table_.begin(); it != routing_table_.end(); it++){
            cout << it->first << " " << (it->second).next_hop << " " << (it->second).distance << endl;
        }

        cout << "\n/** TC table **/ " << endl;
        for(int i=0; i<MAX_NUM_NODES; ++i) {
            cout << "dst: " << i << " | ";
            for(set<int>::iterator it = tc_table_[i].begin(); it != tc_table_[i].end(); ++it) 
                cout << *it << " ";
            cout << endl;
        }
        cout << "\n/** reversed TC table **/ " << endl;
        for(int i=0; i<MAX_NUM_NODES; ++i) {
            cout << "from: " << i << " | ";
            for(set<int>::iterator it = thr_last_hop_[i].begin(); it != thr_last_hop_[i].end(); ++it) 
                cout << *it << " ";
            cout << endl;
        }

        cout << "\n/** MPR set **/ ";
        for(set<int>::iterator it = MPR_set_.begin(); it != MPR_set_.end(); ++it) cout << *it << " ";
        cout << "\n/** MS set **/ ";
        for(set<int>::iterator it = MS_set_.begin(); it != MS_set_.end(); ++it) cout << *it << " ";
        
        cout << "\n/** one-hop neighbor set **/ ";
        for(set<int>::iterator it = one_hop_neighbor_set_.begin(); it != one_hop_neighbor_set_.end(); ++it) cout << *it << " ";
        cout << "\n/** two-hop neighbor set **/ ";
        for(set<int>::iterator it = two_hop_neighbor_set_.begin(); it != two_hop_neighbor_set_.end(); ++it) cout << *it << " ";

        cout << "\n/** neighbor table **/ " << endl;
        for(int i=0; i<MAX_NUM_NODES; ++i) cout << i << " " << neighbor_table_[i] << endl;

        cout << "/** two-hop neighbor table **/ " << endl;
        for(int i=0; i<MAX_NUM_NODES; ++i) {
            cout << "dst: " << i << " | ";
            for(set<int>::iterator it = two_hop_nei_access_thr_[i].begin(); it != two_hop_nei_access_thr_[i].end(); ++it) 
                cout << *it << " ";
            cout << endl;
        }
        cout << "/** reversed two-hop neighbor table**/ " << endl;
        for(int i=0; i<MAX_NUM_NODES; ++i) {
            cout << "from: " << i << " | ";
            for(set<int>::iterator it = from_one_hop_[i].begin(); it != from_one_hop_[i].end(); ++it) 
                cout << *it << " ";
            cout << endl;
        }
#endif

        // If it is time to send the data string, send it.
        if (id_!= dst_id_ && !data_sent && timer == time_to_try) {
            map<int, RoutingTableEntry>::iterator dst_entry;
            dst_entry = routing_table_.find(dst_id_);

            if(dst_entry != routing_table_.end()) {
                SendDataMsg( (dst_entry->second).next_hop );
                data_sent = true;
            }
            else {
                time_to_try += 30;
            }
        }
        
        // If timer is a multiple of 5, send HELLO.
        if (timer % 5 == 0) {
            SendHelloMsg();
        }
        
        // If timer is a multiple of 10, and MS set is not empty, create and send TC.
        if (timer % 10 == 0 && !MS_set_.empty()) {
            SendTcMsg();
        }

        sleep(1);
        ++timer;
    }


}

// Process toX.txt. Return true if the routing table need to be recalcutaed; 
// otherwise, return false.
// Check if there new message; if so, process the messages properly,
bool Node::ProcessToX(int time) {

    // Use flags to indicate if there is any change in neighborhood or TC table.
    bool neighborhood_changed = false;
    bool tc_changed = false;

    // Open toX.txt for reading.
    string filename = FilenameGenerator("to");
    ifstream inputfile;
    inputfile.open(filename.c_str(), ios::in);
    if (!inputfile.is_open()) {
#ifdef DEBUG
        cerr << "Failed: Open file \"" << filename << "\"" << endl;
#endif
        return false;
    }

#ifdef DEBUG
    cout << "* Processing file \"" << filename << "\"" << endl;
#endif

    // Only process new messages.
    streampos last_byte_read = 0;
    inputfile.seekg(position_);
    string line;
    
    while (getline(inputfile, line)) {

#ifdef DEBUG
        cout << "* Processing message: " << line << endl;
#endif

        last_byte_read = inputfile.tellg();

        // Get the node id where this message from and the type of this message.
        // Type of messages and the formats: 
        //  - HELLO msg: * <node> HELLO UNIDIR <neighbor> ... <neighbor> 
        //                              BIDIR <neighbor> ... <neighbor> 
        //                              MPR <neighbor> ... <neighbor>
        //  - TC msg: * <fromnbr> TC <srcnode> <seqno> MS <msnode> ... <msnode>
        //  - DATA msg: <nxthop> <fromnbr> DATA <srcnode> <dstnode> <string>
        
        stringstream str_stream(line);
        string useless, type;
        int fromnbr;
        str_stream >> useless >> fromnbr >> type;
        
        if (type == "HELLO") {
            neighborhood_changed |= HandleHelloMsg(line, time);
        }        
        else if (type == "TC") {
            tc_changed |= HandleTcMsg(line, time);
        }
        else if (type == "DATA") {
            HandleDataMsg(line);
        }
            
    }
        
    position_ = max(last_byte_read, position_);
    inputfile.close();

    // Check last_hello_[i] for all node i. 
    // If have not receive HELLO for 15 seconds, remove node i from neighbor table.
    for (int i=0; i<MAX_NUM_NODES; ++i) {
        if ( one_hop_neighbor_set_.find(i) != one_hop_neighbor_set_.end() 
                && (time - last_hello_[i]) >= 15 ) {
            RemoveNeighbor(i);
            neighborhood_changed |= true;
        }
    }

    // From HELLOs, update MPRs if necessary.
    if (neighborhood_changed) {
        CalculateMprSet();
    }

    return (neighborhood_changed || tc_changed);
}

// Calculate routing table.
void Node::CalculateRoutingTable() {
#ifdef DEBUG
    cout << "* Calculate routing table." << endl;
#endif

    routing_table_.clear();

    // Add entries where destination is one-hop neighbor.
    for (set<int>::iterator it = one_hop_neighbor_set_.begin(); it != one_hop_neighbor_set_.end(); ++it) {
        RoutingTableEntry entry;
        entry.next_hop = *it;
        entry.distance = 1;
        routing_table_.insert( pair<int, RoutingTableEntry>(*it, entry) );
    }

    // Add entries where destination is two-hop neighbor.
    for (set<int>::iterator it = two_hop_neighbor_set_.begin(); it != two_hop_neighbor_set_.end(); ++it) {
        RoutingTableEntry entry;
        set<int>::iterator next_hop_to_dst = two_hop_nei_access_thr_[*it].begin();
        entry.next_hop = *next_hop_to_dst;
        entry.distance = 2;
        routing_table_.insert( pair<int, RoutingTableEntry>(*it, entry) );
    }

    // Add entries where distance is h+1 hops away, h starts from 2.
    // Loop ends when there is no new enrty added into routing table.
    int h = 2;
    bool entry_added = true;
    while (entry_added) {
        entry_added = false;

        // For all entries in TC table, check if there is new routing entry to add.
        for (int i=0; i<MAX_NUM_NODES; ++i) {  
            for (set<int>::iterator it = tc_table_[i].begin(); it != tc_table_[i].end(); ++it) {
                
                // Already exist entry with the destination i, no need to calculate this one.
                if (i == id_ || routing_table_.find(i) != routing_table_.end()) break;

                map<int, RoutingTableEntry>::iterator ret;
                ret = routing_table_.find(*it);
                if( ret != routing_table_.end() && (ret->second).distance == h) {
                    RoutingTableEntry entry;
                    entry.next_hop = (ret->second).next_hop;
                    entry.distance = h+1;
                    routing_table_.insert( pair<int, RoutingTableEntry>(i, entry) );
                    
                    entry_added = true;
                    break;
                }
            }
        }

        ++h;
    }

}

// ReceiveMessage, i.e. append message into Xreceived.txt.
void Node::ReceiveMessage(string msg) {
#ifdef DEBUG
    cout << "Receive msg: " << msg << endl;
#endif

    string filename = FilenameGenerator("received");
    
    ofstream outputfile;
    outputfile.open(filename.c_str(), ios::app);
    if (!outputfile.is_open()) {
#ifdef DEBUG
        cerr << "Failed: Open file \"" << filename << "\"." << endl;
#endif
        return;
    }

    outputfile << msg << endl;
    outputfile.close();
}


// Send message, i.e. append message into fromX.txt.
void Node::SendMessage(string msg) {
    string filename = FilenameGenerator("from");
    ofstream outputfile;
    outputfile.open(filename.c_str(), ios::app);
    if (!outputfile.is_open()) {
#ifdef DEBUG
        cerr << "Failed: Open file \"" << filename << "\"." << endl;
#endif
        return;
    }

    outputfile << msg << endl;
    outputfile.close();
}

// Send HELLO message.
void Node::SendHelloMsg() {
    string hello_msg = GenerateHelloMsg();
    SendMessage(hello_msg);    
}

// Send TC message.
void Node::SendTcMsg() {
    string tc_msg = GenerateTcMsg();
#ifdef DEBUG
    cout << "* Send my TC: " << tc_msg << endl;
#endif
    SendMessage(tc_msg);
}

// Send DATA message.
void Node::SendDataMsg(int next_hop) {
    string data_msg = GenerateDataMsg(next_hop);
#ifdef DEBUG
    cout << "* Send my DATA: " << data_msg << endl;
#endif
    SendMessage(data_msg);
}

// Forward TC message.
void Node::ForwardTcMsg(string msg, string fromnbr) {
    msg.replace(2, fromnbr.size(), itos(id_));
#ifdef DEBUG
    cout << "** Forward TC msg: " << msg << endl;
#endif
    SendMessage(msg);
}

// Forward DATA message.
void Node::ForwardDataMsg(string msg, string nxthop, string fromnbr, int next_hop) {
    msg.replace(nxthop.size()+1, fromnbr.size(), itos(id_));
    msg.replace(0, nxthop.size(), itos(next_hop));
#ifdef DEBUG
    cout << "** Forward DATA msg: " << msg << endl;
#endif
    SendMessage(msg);
}

// Generate a string of HELLO message.
string Node::GenerateHelloMsg() {
    string hello_str = "";

    hello_str += ( "* " + itos(id_) + " HELLO " );

    hello_str += "UNIDIR ";
    for (int i=0; i<MAX_NUM_NODES; ++i) {
        if (neighbor_table_[i] == UNIDIR)
            hello_str += itos(i) + " ";
    }

    hello_str += "BIDIR ";
    for (int i=0; i<MAX_NUM_NODES; ++i) {
        if (neighbor_table_[i] == BIDIR)
            hello_str += itos(i) + " ";
    }

    hello_str += "MPR ";
    for (int i=0; i<MAX_NUM_NODES; ++i) {
        if (neighbor_table_[i] == MPR)
            hello_str += itos(i) + " ";
    }

    return hello_str;
}

// Generate a string of TC message.
string Node::GenerateTcMsg() {
    ++ms_seq_;
    
    string tc_str = "";
    tc_str += ( "* " + itos(id_) + " TC " + itos(id_) + " " + itos(ms_seq_) + " MS ");

    for (set<int>::iterator it = MS_set_.begin(); it != MS_set_.end(); ++it) {
        tc_str += ( itos(*it) + " " );
    }

    return tc_str;
}

// Generate a string of DATA message.
string Node::GenerateDataMsg(int next_hop) {
    string data_str = "";
    data_str += (itos(next_hop) + " " + itos(id_) + " DATA " + itos(id_) + " " + itos(dst_id_) + " " + str_msg_);
    return data_str;
}

// Handle HELLO message.
// Return true if one-hop or two-hop neighbor has changed;
// otherwise, return false.
bool Node::HandleHelloMsg(string msg, int time) {
    stringstream str_stream(msg);
    string dumb, link_type;
    int fromnbr, nbr;
    
    // Use vector to record the neighbor information of node fromnbr.
    vector<int> unidir_of_fromnbr;
    vector<int> bidir_of_fromnbr;
    vector<int> mpr_of_fromnbr;

    // Use these flags to represent the neighborship between node id and fromnbr. 
    bool id_is_unidir_of_fromnbr = false;
    bool id_is_bidir_of_fromnbr = false;
    bool id_is_mpr_of_fromnbr = false;

    // Read HELLO message: * <fromnbr> HELLO.
    str_stream >> dumb >> fromnbr >> dumb;
    
    last_hello_[fromnbr] = time;

    // Compare previous HELLO message with this one.
    // If they are the same, nothing changes, no need to handle. (Since we don't have expire time.)
    if (msg == prev_hello_[fromnbr]) return false;
    prev_hello_[fromnbr] = msg;

    // Read and process remaining HELLO message content.
    while (str_stream >> nbr || !str_stream.eof()) {
        // Identify neighbor link types: UNIDIR, BIDIR, or MPR.
        if (str_stream.fail()) {
            str_stream.clear();
            str_stream >> link_type;
            continue;
        }

        // Put neighbor information of node fromnbr into seperate vectors.
        // Note that do not put id itself into any vector.
        if (link_type == "UNIDIR") {
            if (nbr == id_)  id_is_unidir_of_fromnbr = true;
            else    unidir_of_fromnbr.push_back(nbr);
        }
        else if (link_type ==  "BIDIR") {
            if (nbr == id_)  id_is_bidir_of_fromnbr = true;
            else    bidir_of_fromnbr.push_back(nbr);
        }
        else if (link_type ==  "MPR") {
            if (nbr == id_)  id_is_mpr_of_fromnbr = true;
            else mpr_of_fromnbr.push_back(nbr);
        }
    }

    // Based on the neighbor information of fromnbr, update neighbor table of node id itself.
    if (id_is_unidir_of_fromnbr || id_is_bidir_of_fromnbr || id_is_mpr_of_fromnbr) {
        // Update one-hop table and set.
        neighbor_table_[fromnbr] = BIDIR;
        one_hop_neighbor_set_.insert(fromnbr);

        // Clear two-hop information based on old HELLO of fromnbr.
        for (set<int>::iterator it = from_one_hop_[fromnbr].begin(); it != from_one_hop_[fromnbr].end(); ++it) {
            two_hop_nei_access_thr_[*it].erase(fromnbr);
            if (two_hop_nei_access_thr_[*it].empty()) two_hop_neighbor_set_.erase(*it);
        }
        from_one_hop_[fromnbr].clear();

        // Update two-hop tables.
        for (vector<int>::iterator it = bidir_of_fromnbr.begin(); it != bidir_of_fromnbr.end(); ++it) {
            from_one_hop_[fromnbr].insert(*it);
            two_hop_nei_access_thr_[*it].insert(fromnbr);
        }
        for (vector<int>::iterator it = mpr_of_fromnbr.begin(); it != mpr_of_fromnbr.end(); ++it) {
            from_one_hop_[fromnbr].insert(*it);
            two_hop_nei_access_thr_[*it].insert(fromnbr);
        }

        // Update two-hop set.
        two_hop_neighbor_set_.clear();
        for (int i=0; i<MAX_NUM_NODES; ++i) {
            if (!two_hop_nei_access_thr_[i].empty()) {
                two_hop_neighbor_set_.insert(i);
            }
        }
                
        // Update MS set.
        if (MS_set_.find(fromnbr) != MS_set_.end() && !id_is_mpr_of_fromnbr) MS_set_.erase(fromnbr);
        else if (id_is_mpr_of_fromnbr) MS_set_.insert(fromnbr);
    }
    else {
        neighbor_table_[fromnbr] = UNIDIR;
    }

    return true;
}

// Handle incoming TC message: 
//  1. Forward this TC message if necessary.
//  2. Update TC table if necessary.
// Return true if TC table is changed because of this TC message;
// otherwise, return false.
bool Node::HandleTcMsg(string msg, int time) {
    stringstream str_stream(msg);
    string dumb;
    int fromnbr, srcnode, seqno, msnode;

    str_stream >> dumb >> fromnbr >> dumb >> srcnode >> seqno >> dumb;

    last_tc_[srcnode] = time;

    // No need to process TC generating from itself and TC with old information.
    if(srcnode == id_ || seqno <= latest_seq_[srcnode]) return false;
    
    latest_seq_[srcnode] = seqno;

    // Forward this TC message if it is coming from the node in MS set.
    if (MS_set_.find(fromnbr) != MS_set_.end()) {
        ForwardTcMsg(msg, itos(fromnbr));
    }

    // Clear old information from srcnode.
    for (set<int>::iterator it = thr_last_hop_[srcnode].begin(); it != thr_last_hop_[srcnode].end(); ++it) {
        tc_table_[*it].erase(srcnode);
    }
    thr_last_hop_[srcnode].clear();

    // Update TC table.
    while (str_stream >> msnode) {
        thr_last_hop_[srcnode].insert(msnode);
        tc_table_[msnode].insert(srcnode);
    }

    return true;
}

// Handle DATA message. 
void Node::HandleDataMsg(string msg) {
#ifdef DEBUG
    cout << "* Handle DATA msg: " << msg << endl;
#endif

    stringstream str_stream(msg);
    int nxthop, fromnbr, srcnode, dstnode;
    string dumb, str_content;

    str_stream >> nxthop >> fromnbr >> dumb >> srcnode >> dstnode;

    str_content = msg;
    str_content.replace(0, (itos(nxthop)).size() + (itos(fromnbr)).size() + dumb.size() + 
                           (itos(srcnode)).size() + (itos(dstnode)).size() + 5, "" );

    if (dstnode == id_) {
        ReceiveMessage(str_content);
        return;
    }

    map<int, RoutingTableEntry>::iterator entry;
    entry = routing_table_.find(dstnode);
    
    // Forward DATA according to routing table.
    if (entry != routing_table_.end()) {
#ifdef DEBUG
        cout << "** Find entry with dst in routing table." << endl;
#endif
        ForwardDataMsg(msg, itos(nxthop), itos(fromnbr), (entry->second).next_hop);
    }
}


// Remove node i from neighbor table(s).
void Node::RemoveNeighbor(int i) {
#ifdef DEBUG
    cout << "* Remove node " << i << " from neighbors" << endl;
#endif

    one_hop_neighbor_set_.erase(i);
    neighbor_table_[i] = NA;
    MS_set_.erase(i);

    // Remove all accesses through node i.
    for (set<int>::iterator it = from_one_hop_[i].begin(); it != from_one_hop_[i].end(); ++it) {
        two_hop_nei_access_thr_[*it].erase(i);
        if (two_hop_nei_access_thr_[*it].empty())   two_hop_neighbor_set_.erase(*it);
    }
    from_one_hop_[i].clear();

    // Check if node i is a two-hop neighbor.
    if (!two_hop_nei_access_thr_[i].empty()) { 
        two_hop_neighbor_set_.insert(i);
    }
}

// Remove TC information from node i.
void Node::RemoveTcInfoFrom(int i) {
#ifdef DEBUG
    cout << "* Remove TC information from node " << i << endl;
#endif

    // Remove information based on TC from node i.
    for (set<int>::iterator it = thr_last_hop_[i].begin(); it != thr_last_hop_[i].end(); ++it) {
        tc_table_[*it].erase(i);
    }
    thr_last_hop_[i].clear();

}

// Calculate and update MPR set.
// Use traditional greedy algorithm to select MPRs.
void Node::CalculateMprSet() {
#ifdef DEBUG
    cout << "* Calculate MPR set." << endl;
#endif

    MPR_set_.clear();
    for (int i=0; i<MAX_NUM_NODES; ++i) {
        if (neighbor_table_[i] == MPR)
            neighbor_table_[i] = BIDIR;
    }

    set<int> remaining_two_hop(two_hop_neighbor_set_);
    int cnt[MAX_NUM_NODES], best_candidate;

    while (!remaining_two_hop.empty()) {        
        for (int i=0; i<MAX_NUM_NODES; ++i) cnt[i] = 0;
        best_candidate = -1;

        for (set<int>::iterator it = remaining_two_hop.begin(); it != remaining_two_hop.end(); ++it) {
            for (set<int>::iterator candidate = two_hop_nei_access_thr_[*it].begin(); 
                    candidate != two_hop_nei_access_thr_[*it].end(); ++candidate) {
                
                // Skip those are already selected as MPR.
                if (MPR_set_.find(*candidate) != MPR_set_.end()) continue;

                ++cnt[*candidate];
                
                if ( (best_candidate == -1) || (cnt[*candidate] > cnt[best_candidate]) )
                    best_candidate = *candidate;
            }
        }

        MPR_set_.insert(best_candidate);
        neighbor_table_[best_candidate] = MPR;

        for (set<int>::iterator it = from_one_hop_[best_candidate].begin(); 
                it != from_one_hop_[best_candidate].end(); ++it) {
            remaining_two_hop.erase(*it);
        }
    }
}

// Convert int to string.
string Node::itos(int i) {
    stringstream str_stream;
    string ret;
    str_stream << i;
    ret = str_stream.str();
    return ret;
}

// Generate file name for opening.
// Type can be: "from", "to", and "received".
string Node::FilenameGenerator(string type) {
    char s[100];
    strcpy(s, FILE_DIR);
    string filename(s);
    if (type != "received") filename += ( type + itos(id_) + ".txt" );
    else filename += ( itos(id_) + type + ".txt" );
    return filename;
}
