#include <stdlib.h>
#include <vector>
using namespace std;

class Data {
  public:
    Data();
    vector<char> getData();
    void setData(vector<char> data);
  private:
    vector<char> data;
};
