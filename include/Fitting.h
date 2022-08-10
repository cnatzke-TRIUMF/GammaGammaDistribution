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
    std::vector<TH1 *> auto_gate(TH2 *raw_input, int xyz, int itr, Option_t *specopt, double sigma, double thresh, double Rd, double Ru);

private:
    Inputs fInputs;
    std::vector<TH1 *> fProjectedHistograms;
    std::vector<TH1 *> fBGSubtractedHistograms;
    std::vector<TH1 *> fBGHistograms;
    std::vector<TH1 *> fComptonFits;
};
#endif
