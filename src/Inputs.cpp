#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <stdlib.h>
#include "libconfig.h++"

#include "Inputs.h"
#include "TDirectory.h"
#include "TKey.h"

/* -------------------------------------------
    Constructor
------------------------------------------- */
Inputs::Inputs() : fVerbose(0)
{
    int init_value = -1;
    fElement = fIsotope = init_value;
    fGateLow = fGateHigh = init_value;
    fBgGateLow = fBgGateHigh = init_value;
    fFitLow = fFitHigh = init_value;
    fCentroid = init_value;
}

/* -------------------------------------------
    Destructor
------------------------------------------- */
Inputs::~Inputs()
{
}

/* -------------------------------------------
    Reads in and manages input files
------------------------------------------- */
void Inputs::ReadInputFiles(const std::string &hist_file, const std::string &config_file)
{
    fConfigFileName = config_file;
    ReadConfigFile();
    ReadInHistograms(hist_file);

    for (auto const &iter : fHistogramVector)
    {
        std::cout << "Name: " << iter << std::endl;
        std::cout << "Name: " << iter->GetName() << std::endl;
    }

    for (auto const &iter : fEventMixedHistogramVector)
    {
        std::cout << "Name: " << iter->GetName() << std::endl;
    }
}

/* -------------------------------------------
    Reads in and parses config file
------------------------------------------- */
void Inputs::ReadConfigFile()
{

    libconfig::Config cfg;
    // read the config file and check for errors
    try
    {
        cfg.readFile(fConfigFileName.c_str());
    }
    catch (const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        exit(EXIT_FAILURE);
    }
    catch (const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
        exit(EXIT_FAILURE);
    }

    // get the nuclei of interest
    try
    {
        fElement = cfg.lookup("element");
        fIsotope = cfg.lookup("isotope");

        if (fVerbose > 0)
        {
            std::cout << "Nuclei of interest: z=" << fElement << " a=" << fIsotope << std::endl;
            std::cout << std::endl;
        }
    }
    catch (const libconfig::SettingNotFoundException &nfex)
    {
        std::cerr << "No 'element' or 'isotope' setting in configuration file." << std::endl;
    }

    // Projection gates
    try
    {
        if (fVerbose > 0)
        {
            std::cout << "---> PROJECTION GATES" << std::endl;
            std::cout << std::setw(20) << std::left << "GATE_LOW"
                      << "  "
                      << std::setw(20) << std::left << "GATE_HIGH"
                      << "  "
                      << std::setw(20) << std::left << "BG_GATE_HIGH"
                      << "  "
                      << std::setw(20) << std::left << "BG_GATE_HIGH"
                      << "  "
                      << std::endl;
        }

        if (cfg.lookupValue("projection_gates.gate_low", fGateLow) && cfg.lookupValue("projection_gates.gate_high", fGateHigh) && cfg.lookupValue("projection_gates.bg_gate_low", fBgGateLow) && cfg.lookupValue("projection_gates.bg_gate_high", fBgGateHigh))
        {
            if (fVerbose > 0)
            {
                std::cout << std::setw(20) << std::left << fGateLow << "  "
                          << std::setw(20) << std::left << fGateHigh << "  "
                          << std::setw(20) << std::left << fBgGateLow << "  "
                          << std::setw(20) << std::left << fBgGateHigh << "  "
                          << std::endl;
                std::cout << std::endl;
            }
        }
    }

    catch (const libconfig::SettingNotFoundException &nfex)
    {
        std::cerr << "Missing projection gate parameters" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Fitting gates
    try
    {
        if (fVerbose > 0)
        {
            std::cout << "---> FITTING GATES" << std::endl;
            std::cout << std::setw(20) << std::left << "CENTROID"
                      << "  "
                      << std::setw(20) << std::left << "LOWER_BOUND"
                      << "  "
                      << std::setw(20) << std::left << "UPPER_BOUND"
                      << "  "
                      << std::endl;
        }

        if (cfg.lookupValue("fitting_gates.centroid", fCentroid) && cfg.lookupValue("fitting_gates.fit_low", fFitLow) && cfg.lookupValue("fitting_gates.fit_high", fFitHigh))
        {
            if (fVerbose > 0)
            {
                std::cout << std::setw(20) << std::left << fCentroid << "  "
                          << std::setw(20) << std::left << fFitLow << "  "
                          << std::setw(20) << std::left << fFitHigh << "  "
                          << std::endl;
                std::cout << std::endl;
            }
        }
    }

    catch (const libconfig::SettingNotFoundException &nfex)
    {
        std::cerr << "Missing fitting parameters" << std::endl;
        exit(EXIT_FAILURE);
    }

    return;
}

std::vector<int> Inputs::ProjectionGates()
{
    std::vector<int> v;
    v.push_back(fGateLow);
    v.push_back(fGateHigh);

    return v;
}

std::vector<int> Inputs::ProjectionBgGates()
{
    std::vector<int> v;
    v.push_back(fBgGateLow);
    v.push_back(fBgGateHigh);

    return v;
}

/*********************************************
 * Read histograms into list
 *
 * @param filename ROOT file containing histogram
 *********************************************/
void Inputs::ReadInHistograms(const std::string &filename)
{
    TFile *file = new TFile(filename.c_str());

    if (file->IsOpen())
    {
        std::cout << "Found ROOT file: " << file->GetName() << std::endl;
        ExtractHistograms(file);
    }
    else
    {
        std::cerr << "Could not open ROOT file: " << file->GetName() << std::endl;
        exit(EXIT_FAILURE);
    }
}

/*********************************************
 * Reads histograms into vector
 *
 * @param filename ROOT file containing histogram
 *********************************************/
void Inputs::ExtractHistograms(TFile *file)
{
    std::vector<std::string> dir_names = {"time-random-subtracted", "event-mixed"};
    TH2D *h;
    TKey *key;

    for (auto const &dir_iter : dir_names)
    {
        std::cout << "Retrieving " << dir_iter << " histograms" << std::endl;
        TDirectory *dir = dynamic_cast<TDirectory *>(file->Get(dir_iter.c_str()));
        TIter next(dir->GetListOfKeys());
        while ((key = (TKey *)next()))
        {
            h = static_cast<TH2D *>(dir->Get(key->GetName()));

            if (dir_iter == dir_names.at(0))
            {
                fHistogramVector.push_back(h);
            }
            else if (dir_iter == dir_names.at(1))
            {
                fEventMixedHistogramVector.push_back(h);
            }

            if (fVerbose > 0)
                std::cout << " Found histogram: " << key->GetName() << std::endl;
        }
    }
}