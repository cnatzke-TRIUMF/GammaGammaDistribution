#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include "TFile.h"
#include "TList.h"

class HistogramManager
{
public:

    HistogramManager();
    ~HistogramManager();
    TList* LoadHistograms(std::string fileName, const char *dir_name);

private:
    TList* ExtractHistograms(TFile* inputFile, const char *dir_name);
	bool CheckFileIntegrity(TFile* inputFile);
};

#endif
