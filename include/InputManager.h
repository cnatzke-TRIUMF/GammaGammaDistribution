
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <string>
#include <vector>

class InputManager
{
public:
    InputManager();
    ~InputManager();
    int ReadConfigFile(const std::string &filename);
    std::vector<int> ProjectionGates();
    std::vector<int> ProjectionBgGates();

    inline void SetVerbose(int verbose) { fVerbose = verbose; }

    inline int Element() { return fElement; };
    inline int Isotope() { return fIsotope; };

private:
    std::string fInputFileName;
    int fElement, fIsotope;
    int fGateLow, fGateHigh, fBgGateLow, fBgGateHigh;
    int fFitLow, fFitHigh, fCentroid;
    int fVerbose;
};

#endif
