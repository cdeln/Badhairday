#include "timer.hpp"
#include "gtl/exception.hpp"
#include <sstream>
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm>

TimerEntry::TimerEntry(int size) {
    init(size);
}

void TimerEntry::init(int size) {
    for(int i = 0; i < size; ++i) {
        _stamps.push(0);
    }
    _average = 0;
}

void TimerEntry::push(double newStamp) {
        double lastStamp = _stamps.front();
        _stamps.pop();
        _stamps.push(newStamp);
        _average = _average - lastStamp + newStamp;
}

void TimerEntry::clear() {
    int size = _stamps.size();
    _stamps = std::queue<double>();
    _average = 0;
    init( size );
}

double TimerEntry::average() const {
    return _average;
}

void Timer::tic(const std::string & name) {
    auto it = _ticmap.find(name);
    if( it != _ticmap.end() ) {
        it->second = std::clock(); 
    }
    else {
        _ticmap.insert(it, std::make_pair(name, static_cast<double>(std::clock())));
    }
}

void Timer::toc(const std::string & name) {
    auto it = _ticmap.find(name);
    double elapsed;
    if( it != _ticmap.end() ) {
        elapsed = static_cast<double>(std::clock() - it->second) / CLOCKS_PER_SEC; 
    }
    else {
        std::stringstream ss;
        ss << "toc without tic for name: " << name;
        throw gtl::Exception(ss.str());
    }

    auto tocit = _tocmap.find(name);
    double newStamp = elapsed / _queueSize;
    if( tocit == _tocmap.end() ) {
        TimerEntry entry(_queueSize);
        entry.push(newStamp);
        _tocmap.insert(tocit, std::make_pair(name, entry)); 
    }
    else {
        tocit->second.push(newStamp);
    }
}

void Timer::report() {
    std::cout << "-------------------------" << std::endl;
    std::vector<std::string> names;
    std::vector<TimerEntry> values;
    std::vector<int> indices;
    int index = 0;
    for(auto it = _tocmap.begin(); it != _tocmap.end(); ++it) {
        names.push_back(it->first);
        values.push_back(it->second);
        indices.push_back(index);
        index += 1;
    }
    std::sort(indices.begin(), indices.end(),
            [values](const int & a, const int & b) -> bool {
                return values[a].average() > values[b].average();
            });
    for(unsigned int k = 0; k < indices.size(); ++k) {
        int i = indices[k];
        std::cout << names[i] << ": " << 1000 * values[i].average() << " ms" << std::endl;
    }
    /*
    for(auto it = _tocmap.begin(); it != _tocmap.end(); ++it) {
        std::cout << it->first << ": " << 1000 * it->second.average() << " ms" << std::endl;
    }
    */
}

void Timer::clear() {
    for(auto it = _tocmap.begin(); it != _tocmap.end(); ++it) {
        it->second.clear();
    }
}

void Timer::setLength(int n) {
    _queueSize = n;
    clear();
}
