#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <windows.h>
#include <conio.h>

// Utility function to move the cursor
void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, coord);
}

// Utility function to hide/show the cursor (prevents blinking during animation)
void setCursorVisibility(bool visible) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = visible;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// Requirement 3: Dynamic Console Dimensions
void getConsoleDimensions(int &width, int &height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

class MarqueeConsole {
private:
    std::string text;
    std::atomic<int> speedMs; // Marquee Refresh Rate
    std::atomic<bool> isRunning;
    std::thread workerThread;
    std::mutex& consoleMutex; // Shared Mutex for Console Thread Safety

    void animationLoop() {
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

public:
    MarqueeConsole(std::mutex& mutex) 
        : text("CSOPESY Marquee"), speedMs(50), isRunning(false), consoleMutex(mutex) {}

    ~MarqueeConsole() {
        stop();
    }

    void start() {
        if (!isRunning) {
            isRunning = true;
            workerThread = std::thread(&MarqueeConsole::animationLoop, this);
        }
    }

    void stop() {
        if (isRunning) {
            isRunning = false;
            if (workerThread.joinable()) {
                workerThread.join();
            }
            clearMarqueeArea();
        }
    }

    void clearMarqueeArea() {
        std::lock_guard<std::mutex> lock(consoleMutex);
        int width, height;
        getConsoleDimensions(width, height);

        for (int r = 4; r <= height - 4; r++) {
            setCursorPosition(0, r);
            std::cout << std::string(width, ' ');
        }
    }

    void setText(const std::string& newText) {
        bool wasRunning = isRunning;
        if (wasRunning) stop();
        text = newText;
        if (wasRunning) start();
    }

    void setSpeed(int newSpeedMs) {
        if (newSpeedMs > 0) {
            speedMs = newSpeedMs;
        }
    }
};

int main() {
    system("cls");

    std::mutex consoleMutex;
    MarqueeConsole myMarquee(consoleMutex);

    std::string inputBuffer = "";
    std::string statusMessage = "";
    bool isAppRunning = true;

    // Polling rate delay (ms) for testing keyboard responsiveness
    int pollingRateMs = 10; //try changing this value to 500

    // Header Display (Requirements)
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        setCursorPosition(0, 0);
        std::cout << "Welcome to CSOPESY!\n";
        std::cout << "Group developer: Patrick Malicsi\n";
        std::cout << "Version date: 09/24/2026\n";
    }

    while (isAppRunning) {
        // Main Thread: Non-blocking Polling for Keyboard Input
        if (_kbhit()) {
            char ch = _getch();

            if (ch == '\r' || ch == '\n') { // ENTER
                std::string command = inputBuffer;
                inputBuffer = "";

                if (command == "exit") {
                    myMarquee.stop();
                    isAppRunning = false;
                }
                else if (command == "start_marquee") {
                    myMarquee.start();
                    statusMessage = "Status: Marquee started.";
                }
                else if (command == "stop_marquee") {
                    myMarquee.stop();
                    statusMessage = "Status: Marquee stopped.";
                }
                else if (command == "help") {
                    statusMessage = "Help: start_marquee | stop_marquee | set_text | set_speed | help | exit";
                }
                else if (command.rfind("set_text ", 0) == 0) {
                    std::string newText = command.substr(9);
                    myMarquee.setText(newText);
                    statusMessage = "Status: Marquee text updated.";
                }
                else if (command.rfind("set_speed ", 0) == 0) {
                    try {
                        int speed = std::stoi(command.substr(10));
                        myMarquee.setSpeed(speed);
                        statusMessage = "Status: Speed set to " + std::to_string(speed) + " ms.";
                    } catch (...) {
                        statusMessage = "Error: Invalid speed integer.";
                    }
                }
                else if (!command.empty()) {
                    statusMessage = "Unrecognized command: '" + command + "'";
                }
            }
            else if (ch == '\b') { // BACKSPACE
                if (!inputBuffer.empty()) {
                    inputBuffer.pop_back();
                }
            }
            else if (ch >= 32 && ch <= 126) { // PRINTABLE CHARACTERS
                inputBuffer += ch;
            }
        }

        // Render Command Prompt & Status Area dynamically
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            int width, height;
            getConsoleDimensions(width, height);

            int promptRow = height - 3;
            int statusRow = height - 2;

            // Render Prompt Line
            setCursorPosition(0, promptRow);
            std::cout << "Command > " << inputBuffer << std::string(width - (10 + inputBuffer.length()), ' ');

            // Render Status Line
            setCursorPosition(0, statusRow);
            std::cout << statusMessage << std::string(width - statusMessage.length(), ' ');

            // Place typing cursor at correct input index
            setCursorVisibility(true);
            setCursorPosition(10 + (int)inputBuffer.length(), promptRow);
        }

        // Polling Rate Sleep (Main thread delay)
        std::this_thread::sleep_for(std::chrono::milliseconds(pollingRateMs));
    }

    system("cls");
    return 0;
}
