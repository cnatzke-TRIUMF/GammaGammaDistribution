
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <string>

class InputManager
{
public:
    InputManager();
    ~InputManager();
    int ReadConfigFile(const std::string &filename);

private:
    std::string fInputFileName;
};

#endif
