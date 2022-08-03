
#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <vector>

class Inputs
{
public:
    Inputs();
    ~Inputs();
    void ReadConfigFile(const std::string &filename);
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
