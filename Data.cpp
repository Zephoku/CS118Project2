#include "Data.h"

Data::Data() {
  data.resize(1024);
}

vector<char> Data::getData() {
  return this->data;
}

void Data::setData(vector<char> data) {
  this->data = data;
}
