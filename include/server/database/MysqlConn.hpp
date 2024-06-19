#pragma once
#include "MySQL.hpp"
#include <chrono>
using namespace chrono;

class MySqlConn : public MySQL {
public:
    MySqlConn() {
        connect();
        refreshUseTime();
    }
    // 更新最后一次使用的时间
    void refreshUseTime() {
        _lastUseTime = steady_clock::now();
    }

    seconds getIdleTime() const {
        return duration_cast<seconds>(steady_clock::now() - _lastUseTime);
    }
private:
    steady_clock::time_point _lastUseTime; // 最后使用这个连接的时间
};