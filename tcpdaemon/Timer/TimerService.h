#ifndef TIMERSERVICE_H
#define TIMERSERVICE_H

#include <inttypes.h>
#include <list>
#include  "../TCPConn.h"

class TCPDaemon;

//The DeltaTimer structure holds all info related to a packet timer
//As there is one global TimerService, each timer holds a seqnum and pointer to a TCPConn to
//Uniquely identify the timer back to a particular TCPConn.
struct DeltaTimer{
    DeltaTimer(uint32_t time, uint32_t seqNum, TCPConn* conn);
    uint32_t time;
    TCPConn* conn;
    uint32_t seqnum;
};

class TimerService
{
    public:

	//The TimerService is instantiated with a reference back to the TCPDaemon
        TimerService(TCPDaemon& theDaemon);

	/**
	 * Add a new Timer to be tracked. 
	 *
	 * @param time, timeout in milliseconds for this new timer, starting from the current time
	 * @param seqNum the sequence number identifying the packet to be timed out
	 * @param theConn A pointer back the TCPConn to be alerted when this timer times out
	 */
        void addTimer(uint32_t time, uint32_t seqNum, TCPConn& theConn);

	/**
	 * Remove a timer. This is called when a ACK has been succesfully received
	 * This removes all timers associated with theConn that have a seqnum equal to 
	 * or lower than seqNum. This helps with the cumulative ACK
	 *
	 * @param seqNum the sequence number identifying the packet to be removed
	 * @param theConn A pointer back the TCPConn owning the timer to be removed
	 */
        void removeTimer(uint32_t seqNum, TCPConn& theConn);

	/**
	 * Remove all timers owned by a connection. 
	 *
	 * @param theConn A pointer back the TCPConn owning the timers to be removed
	 */
        void removeAll(TCPConn& theConn);

	/**
	 * Called by the TCPDaemon when the poll times out. The DeltaTimer that has timed out is passed in
	 * to ensure the right timer is expired. The front timer should always be expired, unless
	 * it has been expired/ACK'd  
	 *
	 * @param theConn A pointer back the TCPConn owning the timers to be removed
	 */
        void TimeOut(DeltaTimer timer );


	/**
	 * Called by the TCPDaemon when the timer currently being waited on is removed, so the Daemon can get the next time out
	 *
	 * @returns the next DelatTimer
	 */

inline
    DeltaTimer getNext(){
        if (mTimers.empty())
        {
            return DeltaTimer(-1,0,NULL);
        }
        return mTimers.front();
    }


    private:
//A reference back to the Daemon
TCPDaemon& theDaemon;

//The doubly-linked list of Timers
std::list<DeltaTimer> mTimers;

};



#endif //TIMERSERVICE_H
