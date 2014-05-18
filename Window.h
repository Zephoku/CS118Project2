#include "Header.h"
#include "Data.h"
#include <vector>
#include <cstdio>

using namespace std;

struct Packet {
  Header header;
  Data data;
};

class Window {
  public:
    Window();
    vector<Packet*> packets;
    vector<Packet*> disassemble(FILE file);
    vector<Packet*> assemble();
    const int timer;
    FILE getFILE();
    void setFILE(FILE file);
  private: 
    FILE file;
};
