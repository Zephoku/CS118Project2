#include <time.h>

class Header {
  public:
    Header();

    int getSeqNum();
    void setSeqNum(int seqNum);
    int getAckNum();
    void setAckNum(int ackNum);
    time_t getTimestamp();
    void setTimestamp(time_t timestamp);
    int getContentLength();
    void setContentLength(int contentLength);
    int getFin();
    void setFin(int fin);
  private:
    int seqNum;
    int ackNum;
    time_t timestamp;
    int contentLength;
    int fin;
};
