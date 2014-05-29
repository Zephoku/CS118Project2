#include "Window.h"
#include <stdio.h>
#include <time.h>

Window::Window() {
  timer = 1;
}

Window::~Window() {
  for (int i = 0; i < this->packets.size(); i++)
    delete this->packets[i];
}

// Disassembles Filename and saves in packets
// Returns 0 on success, 1 on error
int Window::disassemble(string filename) {
  const int bufSize = 1024;
  int curSeqNum = 0, packetSize = 0;
  char buf;
  FILE *file;
  Packet *packet;

  file = fopen(filename.c_str(), "rb");

  if (file == NULL) {
    return -1;
  }

  packet = new Packet();

  while( ! feof(file)) {
    buf = fgetc(file);

    packet->data[packetSize] = buf;
    packetSize++;

    // Create new packet
    if (packetSize >= bufSize) {
      time_t timer;
      time(&timer);
      packet->header.setSeqNum(curSeqNum * 1024 + packetSize);
      packet->header.setContentLength(packetSize);
      packet->header.setTimestamp(timer);
      this->packets.push_back(packet);

      // Reset packet
      packet = new Packet();
      packetSize = 0;
      curSeqNum++;
    }
  }

  // Create leftover packet
  if (packetSize > 0) {
    time_t timer;
    time(&timer);
    packet->header.setSeqNum(curSeqNum * 1024 + packetSize);
    packet->header.setContentLength(packetSize);
    packet->header.setTimestamp(timer);
    this->packets.push_back(packet);
  }

  fclose (file);
  return 0;
}

// Assembles packets and saves as filename
// Returns 0 on success, 1 on error
int Window::assemble(string filename) {
  FILE* file;
  file = fopen (filename.c_str(), "wb");

  if(file == NULL)
    return -1;

  for(int j = 0; j < this->packets.size(); j++) {
    fwrite( &(this->packets[j]->data[0]), 
        sizeof(char), 
        this->packets[j]->header.getContentLength(),
        file );
  }

  fclose (file);

  return 0;
}

