
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <string>

class InputManager
{
public:
    InputManager();
    ~InputManager();
    int ReadConfigFile(const std::string &filename);

    inline void SetVerbose(int verbose) { fVerbose = verbose; }
    inline int Element() { return fElement; };

private:
    std::string fInputFileName;
    int fElement, fIsotope;
    int fGateLow, fGateHigh, fBgGateLow, fBgGateHigh;
    int fFitLow, fFitHigh, fCentroid;
    int fVerbose;
};

#endif
