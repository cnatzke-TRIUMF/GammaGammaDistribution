#ifndef GENERATE_DISTRIBUTION_H
#define GENERATE_DISTRIBUTION_H

#include <string>

// void OpenRootFile(std::string fileName);
int PlotDistribution(std::vector<TH1D *> corrHists, std::vector<TH1D *> uncorrHists, float fitLow, float fitHigh);
int AutoFileDetect(std::string fileName);
void InitializeGRSIEnv();
void PrintUsage(char *argv[]);

std::string fHistFile, fConfigFile;

#endif
