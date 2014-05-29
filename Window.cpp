#include "Window.h"
#include <stdio.h>
#include <stdio.h>

Window::Window() {
  timer = 1;
}

vector<Packet*> Window::disassemble(FILE* file) {
  const int bufSize = 1024;
  Packet *packet;
  char buf;
  int curSeqNum = 0, packetSize = 0;

  if (file == NULL) 
    perror ("Error opening file");
  else {
    packet = new Packet();
    packet->header.setSeqNum(curSeqNum);

    while( ! feof(file)) {
      buf = fgetc(file);

      packet->data.data.push_back(buf);
      packetSize++;

      // Create new packet
      if (packetSize >= bufSize) {
        packet->header.setContentLength(packetSize);
        // TODO: Set timestamp
        this->packets.push_back(packet);

        // Reset packet
        packet = new Packet();
        packetSize = 0;
      }
    }

    // Create leftover packet
    if (packetSize > 0) {
      packet->header.setContentLength(packetSize);
      // TODO: Set timestamp
      this->packets.push_back(packet);

      packet = new Packet();
      packetSize = 0;
    }

    fclose (file);
  }
  return this->packets;
}

FILE* Window::assemble() {
  FILE* file;
  file = fopen ("testout.txt", "wb");

  for(int j = 0; j < this->packets.size(); j++) {
    fwrite(&(this->packets[j]->data.data[0]), sizeof(char), this->packets[j]->data.data.size(), file);
  }

  fclose (file);
}

FILE Window::getFILE() {
  return this->file;
}

void Window::setFILE(FILE file) {
  this->file = file;
}
