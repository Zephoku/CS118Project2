#include "Header.h"

Header::Header() {
  this->seqNum = 0;
  this->ackNum = 0;
  this->contentLength = 0;
  this->fin = 0;
}

int Header::getSeqNum() {
  return this->seqNum;
}

int Header::getAckNum() {
  return this->ackNum;
}

time_t Header::getTimestamp() {
  return this->timestamp;
}

int Header::getContentLength() {
  return this->contentLength;
}

int Header::getFin() {
  return fin;
}

void Header::setSeqNum(int seqNum) {
  this->seqNum = seqNum;
}

void Header::setAckNum(int ackNum) {
  this->ackNum = ackNum;
}

void Header::setTimestamp(time_t timestamp) {
  this->timestamp = timestamp;
}

void Header::setContentLength(int contentLength) {
  this->contentLength = contentLength;
}

void Header::setFin(int fin) {
  this->fin = fin;
}
