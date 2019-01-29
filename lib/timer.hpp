#ifndef __TIMER__
#define __TIMER__

#include <string>
#include <map>
#include <queue>

class TimerEntry {
    private:
        double _average;
        std::queue<double> _stamps;
        void init(int size);
    public:
        TimerEntry(int size);
        void push(double stamp);
        double average() const;
        void clear();
};

class Timer {
    private:
        std::map<std::string, double> _ticmap;
        std::map<std::string, TimerEntry> _tocmap;
        int _queueSize = 100;
    public:
        void tic(const std::string & name);
        void toc(const std::string & name);
        void report();
        void clear();
        void setLength(int n);
};

#endif 
