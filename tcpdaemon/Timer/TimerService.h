#ifndef TIMERSERVICE_H
#define TIMERSERVICE_H

#include <inttypes.h>
#include <list>
#include  "../TCPConn.h"

class TCPDaemon;


struct DeltaTimer{
    DeltaTimer(uint32_t time, uint32_t seqNum, TCPConn* conn);


    uint32_t time;
    TCPConn* conn;
    uint32_t seqnum;
};

class TimerService
{
    public:


        TimerService(TCPDaemon& theDaemon);

        void addTimer(uint32_t time, uint32_t seqNum, TCPConn& theConn);
        void removeTimer(uint32_t seqNum, TCPConn& theConn);
        void removeAll(TCPConn& theConn);

        void TimeOut(DeltaTimer timer );
inline
    DeltaTimer getNext(){
        if (mTimers.empty())
        {
            return DeltaTimer(-1,0,NULL);
        }
        return mTimers.front();
    }


    private:
TCPDaemon& theDaemon;

std::list<DeltaTimer> mTimers;

};



#endif //TIMERSERVICE_H
