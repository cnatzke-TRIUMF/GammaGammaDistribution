#ifndef FITTING_H
#define FITTING_H

#include "TH1.h"
#include "Inputs.h"

class Fitting
{
public:
    Fitting(Inputs &inputs);
    ~Fitting();
    void GateAndFit();
    void GateAndProject(TH2 *hist);

    std::vector<TH1D *> GenerateProjections(TList *histList, float low_proj, float high_proj, float low_fit, float high_fit);
    std::vector<TH1D *> CloneProjections(std::vector<TH1D *> histVec, float lowFit, float highFit);
    TRWPeak *FitPeak(TH1D *inputHist, float peakPos, float lowFit, float highFit);

private:
    void GetBackgroundFraction(TH1 *hist);

    Inputs fInputs;
    TF1 *fFitFunction;
    double fBackgroundSubtractionFactor;
    std::vector<TH1 *> fProjectedHistograms;
    std::vector<TH1 *> fBGSubtractedHistograms;
    std::vector<TH1 *> fComptonHistograms;
    std::vector<TH1 *> fComptonFits;
    std::vector<TH1 *> fGateHistograms;
};
#endif
