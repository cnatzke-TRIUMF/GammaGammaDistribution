#ifndef FIT_MANAGER_H
#define FIT_MANAGER_H

#include "TH1.h"

class FitManager
{
public:
    FitManager();
    ~FitManager();
    std::vector<TH1D*> GenerateProjections(TList* histList, float low_proj, float high_proj, float low_fit, float high_fit);
    std::vector<TH1D*> CloneProjections(std::vector<TH1D*> histVec, float lowFit, float highFit);
    TRWPeak* FitPeak(TH1D* inputHist, float peakPos, float lowFit, float highFit);

private:
};
#endif
