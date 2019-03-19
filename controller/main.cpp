#include "Controller.h"

using namespace std;

int main(){
    Controller controller;

    controller.ParseTopology();

#ifdef DEBUG
    controller.PrintTopology();
#endif

    controller.StartProcess();
}
