#ifndef TIMERSERVICE_H
#define TIMERSERVICE_H


class TCPConn;

class TimerService
{
public:

TimerService();

void RegisterTimer(uint32_t timerid);
void UnregisterTimer(uint32_t timerid);


private:

};

#endif //TIMERSERVICE_H
