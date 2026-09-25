#include "marquee_console.h"
#include <iostream>
#include <chrono>

// Utility function definitions
void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, coord);
}

void setCursorVisibility(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void getConsoleDimensions(int &width, int &height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

// MarqueeConsole Class Definitions
MarqueeConsole::MarqueeConsole(std::mutex& mutex) 
    : text("CSOPESY Marquee"), speedMs(50), isRunning(false), consoleMutex(mutex) {}

MarqueeConsole::~MarqueeConsole() {
    stop();
}

void MarqueeConsole::animationLoop() {
    int x = 0;
    int y = 4;
    int dx = 1;
    int dy = 1;

    while (isRunning) {
        int width, height;
        getConsoleDimensions(width, height);

        int minX = 0;
        int maxX = width;
        int minY = 4;        // Below the 3-line header
        int maxY = height - 4; // Above the command prompt section

        // Synchronized drawing block
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            setCursorVisibility(false);

            // Clear previous frame position
            setCursorPosition(x, y);
            std::cout << std::string(text.length(), ' ');

            // Move position
            x += dx;
            y += dy;

            // DVD Bounce Boundary Collision Check
            if (x <= minX || x + (int)text.length() >= maxX) {
                dx *= -1;
                if (x <= minX) x = minX;
                if (x + (int)text.length() >= maxX) x = maxX - (int)text.length();
            }
            if (y <= minY || y >= maxY) {
                dy *= -1;
                if (y <= minY) y = minY;
                if (y >= maxY) y = maxY;
            }

            // Render marquee at new position
            setCursorPosition(x, y);
            std::cout << text;
            std::cout.flush();
        }

        // Marquee Refresh Rate Control
        std::this_thread::sleep_for(std::chrono::milliseconds(speedMs));
    }
}

void MarqueeConsole::start() {
    if (!isRunning) {
        isRunning = true;
        workerThread = std::thread(&MarqueeConsole::animationLoop, this);
    }
}

void MarqueeConsole::stop() {
    if (isRunning) {
        isRunning = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
        clearMarqueeArea();
    }
}

void MarqueeConsole::clearMarqueeArea() {
    std::lock_guard<std::mutex> lock(consoleMutex);
    int width, height;
    getConsoleDimensions(width, height);

    for (int r = 4; r <= height - 4; r++) {
        setCursorPosition(0, r);
        std::cout << std::string(width, ' ');
    }
}

void MarqueeConsole::setText(const std::string& newText) {
    bool wasRunning = isRunning;
    if (wasRunning) stop();
    text = newText;
    if (wasRunning) start();
}

void MarqueeConsole::setSpeed(int newSpeedMs) {
    if (newSpeedMs > 0) {
        speedMs = newSpeedMs;
    }
}
