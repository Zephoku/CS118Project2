#include "Header.h"
#include <vector>
#include <cstdio>
#include <string>

using namespace std;

struct Packet {
  Header header;
  char data[1024];
};

class Window {
  public:
    Window();
    ~Window();
    vector<Packet*> packets;
    int disassemble(string filename);
    int assemble(string filename);
    int timer;
};
