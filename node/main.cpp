#include "Node.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>

using namespace std;

int main(int argc, char **argv) {

    /*
    if (argc != 5 || argc != 3) {
        cerr << "The number of arguments is not correct." << endl;
        cerr << "Need four arguments: <id> <dst-id> <string message> <time>" << endl;
        cerr << "Or two arguments: <id> <id>" << endl;
    }
    */
    
    // Handle arguments: 
    //  - argv[1]: node id of itself
    //  - argv[2]: node id of message destination
    //  - argv[3]: content of string message
    //  - argv[4]: time amount (after this time amount, begin trying send string message)
    // If destination id is same as id of itself, there is no string message to send.

    int id, dst_id, time;
    string str_msg;
    for (int i=1; i<argc; ++i) {
        stringstream str_stream;
        str_stream << argv[i];
        switch (i) {
            case 1:
                str_stream >> id;
                break;
            case 2:
                str_stream >> dst_id;
                break;
            case 3:
                str_msg = str_stream.str();
                break;
            case 4:
                str_stream >> time;
                break;
        }
    }

    if (argc == 5 && id != dst_id){
        Node node(id, dst_id, str_msg, time);
        node.StartProcess();
    }
    else {
        Node node(id, dst_id);
        node.StartProcess();
    }

}
