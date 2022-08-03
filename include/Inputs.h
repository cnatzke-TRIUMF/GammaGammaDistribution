
#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <vector>
#include "TFile.h"
#include "TH2.h"

class Inputs
{
public:
    Inputs();
    ~Inputs();
    void ReadInputFiles(const std::string &histo_file, const std::string &config_file);
    void ReadConfigFile(const std::string &filename);
    void ReadInHistograms(const std::string &filename);
    void ExtractHistograms(TFile *file);
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
    std::vector<TH2D *> fHistogramVector;
};

#endif
