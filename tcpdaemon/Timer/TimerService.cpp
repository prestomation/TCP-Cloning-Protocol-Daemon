
#include "TimerService.h"
#include "../TCPDaemon.h"
#include <iostream>
#include <string>
using namespace std;

TimerService::TimerService(TCPDaemon& theDaemon): theDaemon(theDaemon), mTimers()
{
}

void TimerService::addTimer(uint32_t time, uint32_t seqNum, TCPConn& theConn)
{

    DeltaTimer newTimer(time, seqNum, &theConn);
    if( mTimers.empty())
    {
        //If we have no timers, set daemon timeout and return
        theDaemon.setNextTimeout(newTimer);
        cout << "Resetting timeout first timer" << endl;
        mTimers.push_back(newTimer);
        return;
    }

    uint32_t timeDiff = time;
    cout << "Adding time: " << timeDiff << endl;
    list<DeltaTimer>::iterator iter = mTimers.begin();

    while(iter != mTimers.end())
    {
            if(timeDiff < iter->time)
            {
                //cout << "timeDiff: " << timeDiff <<" less than itertime: " << iter->time << endl;
                newTimer.time = timeDiff;
                mTimers.insert(iter, newTimer);
                //cout << "Timer " << seqNum << " added with time:"<< timeDiff << endl;
                iter->time -= timeDiff;
                return;
            }
            timeDiff -= iter->time;
            //cout << "New time diff: " << timeDiff << endl;
            iter++;
    }

    mTimers.push_back(newTimer);

    cout << "Timer added to end" << seqNum <<endl;

}

void TimerService::removeTimer(uint32_t seqNum, TCPConn& theConn)
{
    list<DeltaTimer>::iterator iter = mTimers.begin();
    cout << "Looking for " << seqNum << " owned by " << &theConn << endl;
    while(iter != mTimers.end())
    {
        //cout << "Found " << iter->seqnum << " owned by " << iter->conn << endl;
        if (iter->seqnum <= seqNum && iter->conn == &theConn )
        {
            cout << "Timer removed "<< iter->seqnum << endl;
            mTimers.erase(iter++);
        }
        else
        {
            iter++;
        }
    }
}

void TimerService::removeAll(TCPConn& theConn)
{
    list<DeltaTimer>::iterator iter = mTimers.begin();
    //mTimers.clear(); //TODO: we really want to do the loop down there
    while(iter != mTimers.end())
    {
        cout << "Found " << iter->seqnum << " owned by " << iter->conn << endl;
        if (iter->conn == &theConn )
        {
            cout << "Timer removed "<< iter->seqnum << endl;
            mTimers.erase(iter++);
        }
        else
        {
            iter++;
        }
    }
}

 
void TimerService::TimeOut(DeltaTimer timer)
{
    if(mTimers.empty())
    {
        //Nothing to expire
        DeltaTimer notimer(-1,0,NULL);
        //reset the daemon to infinite timeout
        cout << "Resetting timeout due to none left" << endl;
        theDaemon.setNextTimeout(notimer);
        return;
    }
    cout << "Front timer expired" << endl;
    cout << mTimers.size() << " Timers" << endl;
    DeltaTimer expiredTimer = mTimers.front();

    list<DeltaTimer>::iterator iter;
    //cout << "Looking for " << timer.seqnum << " owned by " << timer.conn << endl;
    for(iter = mTimers.begin(); iter != mTimers.end(); iter++)
    {
        //cout << "Found " << iter->seqnum << " owned by " << iter->conn << endl;
        if (iter->seqnum == timer.seqnum && iter->conn == timer.conn )
        {
            //cout << "Timer "<< iter->seqnum <<" removed due to expiration" << endl;
            mTimers.erase(iter);
            expiredTimer.conn->ExpireTimer();
            return;
        }
    }
    cout << "Resetting poll() timeout" << endl;
    theDaemon.setNextTimeout(mTimers.front());

}

DeltaTimer::DeltaTimer(uint32_t time, uint32_t seqNum, TCPConn* conn)
    :time(time), conn(conn), seqnum(seqNum){}
