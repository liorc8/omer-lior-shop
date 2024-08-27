#include <iostream>
#include <thread>
#include "CommonObject.h"
#include "DrawThread.h"
#include "DownloadThread.h"
#include <filesystem>

namespace fs = std::filesystem;  // Alias for the filesystem namespace

void ClearDirectory(const std::string& path) {
    if (fs::exists(path) && fs::is_directory(path)) {  // Checks if the path exists and is a directory
        for (const auto& entry : fs::directory_iterator(path)) {
            fs::remove_all(entry.path());  // Recursively removes all files and directories in the specified path
        }
    }
}

int main() {
    // Clear the "orders" and "customers" directories at the start
    ClearDirectory("orders");  // Clears the "orders" directory

    CommonObjects common;  // Create an instance of CommonObjects to hold shared data
    DrawThread draw;  // Create an instance of DrawThread for handling drawing operations
    DownloadThread down;  // Create an instance of DownloadThread for handling download operations

    auto draw_th = std::jthread([&] { draw(common); });  // Starts a thread for drawing, passing the CommonObjects instance
    auto down_th = std::jthread([&] { down(common); });  // Starts a thread for downloading, passing the CommonObjects instance

    down.SetUrl("dummyjson.com");  // Sets the URL for downloading data
    std::cout << "running...\n";  // Outputs a message indicating that the program is running

    draw_th.join();  // Waits for the drawing thread to finish
    down_th.join();  // Waits for the downloading thread to finish
}
