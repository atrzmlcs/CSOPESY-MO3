#ifndef MARQUEECONSOLE_H
#define MARQUEECONSOLE_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

// Utility function declarations
void setCursorPosition(int x, int y);
void setCursorVisibility(bool visible);
void getConsoleDimensions(int &width, int &height);

class MarqueeConsole {
private:
    std::string text;
    std::atomic<int> speedMs; // Marquee Refresh Rate
    std::atomic<bool> isRunning;
    std::thread workerThread;
    std::mutex& consoleMutex; // Shared Mutex for Console Thread Safety

    void animationLoop();

public:
    MarqueeConsole(std::mutex& mutex);
    ~MarqueeConsole();

    void start();
    void stop();
    void clearMarqueeArea();
    void setText(const std::string& newText);
    void setSpeed(int newSpeedMs);
};

#endif
