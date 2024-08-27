#pragma once
#include "CommonObject.h"  // Includes the CommonObject.h header file.

class DrawThread {
public:
    void operator()(CommonObjects& common);  // Overloaded operator() to make the object callable like a function, handles drawing operations using CommonObjects.
};
