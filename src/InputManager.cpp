#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include "libconfig.h++"

#include "InputManager.h"

/* -------------------------------------------
    Constructor
------------------------------------------- */
InputManager::InputManager() : fVerbose(0)
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
InputManager::~InputManager()
{
}

/* -------------------------------------------
    Reads in and parses config file
------------------------------------------- */
int InputManager::ReadConfigFile(const std::string &filename)
{
    fInputFileName = filename;

    libconfig::Config cfg;
    // read the config file and check for errors
    try
    {
        cfg.readFile(filename.c_str());
    }
    catch (const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return (EXIT_FAILURE);
    }
    catch (const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
        return EXIT_FAILURE;
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
            std::cout << "\tPROJECTION GATES" << std::endl;
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
        return EXIT_FAILURE;
    }

    // Fitting gates
    try
    {
        if (fVerbose > 0)
        {
            std::cout << "\tFITTING GATES" << std::endl;
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
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
