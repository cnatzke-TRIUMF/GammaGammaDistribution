
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
    void ReadConfigFile();
    void ReadInHistograms(const std::string &filename);
    void ExtractHistograms(TFile *file);
    std::vector<int> ProjectionGates();
    std::vector<int> ProjectionBgGates();

    inline void SetVerbose(int verbose) { fVerbose = verbose; }

    inline std::vector<TH2D *> GetPromptHistograms() { return fHistogramVector; };
    inline std::vector<TH2D *> GetEventMixedHistograms() { return fEventMixedHistogramVector; };
    inline int Element() { return fElement; };
    inline int Isotope() { return fIsotope; };
    inline int GateLow() { return fGateLow; };
    inline int GateHigh() { return fGateHigh; };
    inline double GateCentroid() { return fGateCentroid; };
    inline int BgGateLow() { return fBgGateLow; };
    inline int BgGateHigh() { return fBgGateHigh; };

private:
    std::string fConfigFileName;
    double fGateCentroid, fPeakCentroid;
    int fElement, fIsotope;
    int fGateLow, fGateHigh, fBgGateLow, fBgGateHigh;
    int fFitLow, fFitHigh;
    int fVerbose;
    std::vector<TH2D *> fHistogramVector;
    std::vector<TH2D *> fEventMixedHistogramVector;
};

#endif
