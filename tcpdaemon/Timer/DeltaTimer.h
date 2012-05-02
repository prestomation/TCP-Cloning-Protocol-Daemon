#ifndef DELTATIMER_H
#define DELTATIMER_H



class DeltaTimer{


public:

DeltaTimer(TCPConn& theConn);
DeltaTimer* nextTimer;
void addTimer(DeltaTimer newTimer);
void removeTimer
uint32_t getTime();


private:
uint32_t mTime;
TCPConn& mConn;
uint32_t mSeqNum;

}

#endif //DELTATIMER_H
