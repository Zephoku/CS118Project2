#include "Window.h"

Window::Window() {
}

vector<Packet*> Header::disassemble(FILE file) {
  //TODO: Implement
}

vector<Packet*> Header::assemble() {
  //TODO: Implement
}

FILE Header::getFILE() {
  return this->file;
}

void Header::setFILE(FILE file) {
  this->file = file;
}
