#include <iostream>
#include <string>
#include <chrono>
#include <conio.h>
#include "marquee_console.h" // Import your custom class

int main() {
    system("cls");

    std::mutex consoleMutex;
    MarqueeConsole myMarquee(consoleMutex);

    std::string inputBuffer = "";
    std::string statusMessage = "";
    bool isAppRunning = true;

    // Polling rate delay (ms) for testing keyboard responsiveness
    int pollingRateMs = 10; 

    // Header Display
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
