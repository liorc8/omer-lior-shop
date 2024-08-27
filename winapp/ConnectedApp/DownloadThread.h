#pragma once
#include "CommonObject.h"  // Includes the CommonObject.h header file.

struct ParsedURL {
    std::string host;  // Hostname part of the URL.
    std::string path;  // Path part of the URL.
};

class DownloadThread
{
public:
    void operator()(CommonObjects& common);  // Overloaded operator() to make the object callable like a function, performs the download task using CommonObjects.
    void SetUrl(std::string_view new_url);  // Sets the URL to be used for downloading.
private:
    std::string _download_url;  // Stores the URL for the download.
};
