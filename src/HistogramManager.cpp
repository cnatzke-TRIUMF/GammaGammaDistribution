//////////////////////////////////////////////////////////////////////////////////
// Checks ROOT file and loads required histograms
//
// Author:          Connor Natzke (cnatzke@triumf.ca)
// Creation Date:   Tuesday May 12, 2020    T15:39:33-07:00
// Last Update:     Tuesday May 12, 2020    T15:39:33-07:00
//////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "TDirectory.h"
#include "TKey.h"
#include "HistogramManager.h"

/************************************************************//**
 * Constructor
 ***************************************************************/
HistogramManager::HistogramManager(){
	//std::cout << "Histogram Manager intialized" << std::endl;
}

/************************************************************//**
 * Destructor
 ***************************************************************/
HistogramManager::~HistogramManager(){
	//std::cout << "Histogram Manager deleted" << std::endl;
}

/************************************************************//**
 * Opens Root files
 *
 * @param fileName  Name of ROOT file
 ***************************************************************/
TList* HistogramManager::LoadHistograms(std::string fileName, const char *dir_name){
	TFile *in_file = new TFile(fileName.c_str());
    TList *hist_list;

	if (in_file->IsOpen()) {
		std::cout << "Opened ROOT file: " << in_file->GetName() << std::endl;
		hist_list = ExtractHistograms(in_file, dir_name);
        return hist_list;
	} else {
		std::cerr << "HistogramManager::LoadHistograms Could not open ROOT file: " << in_file->GetName() << std::endl;
        return hist_list;
	}
} // end OpenRootFile

/************************************************************//**
 * Checks for required directories
 *
 * @param verbose Verbosity level
 ***************************************************************/
TList* HistogramManager::ExtractHistograms(TFile* inputFile, const char *dir_name){

    int verbose = 0;
	TList *hist_list = new TList();
	TObject *obj;
	TKey *key;

	// returns list of -1's if file is bad
	if (!CheckFileIntegrity(inputFile)) return hist_list;

	TDirectory *hist_dir = dynamic_cast<TDirectory*>(inputFile->Get(dir_name));
	TIter next( hist_dir->GetListOfKeys());
    std::cout << "Retrieving " << dir_name << " histograms ... " << std::endl;
	while ((key = (TKey *) next())) {
		obj = hist_dir->Get(key->GetName()); // copy object to memory
        hist_list->Add(obj);
		if (verbose > 0) std::cout << " found object: " << key->GetName() << std::endl;
	}
    std::cout << "Retrieving " << dir_name << " histograms ... [DONE]" << std::endl;
	std::cout << std::endl;

	return hist_list;
} // LoadHistograms

/************************************************************//**
 * Checks for required directories
 *
 * @param verbose Verbosity level
 ***************************************************************/
bool HistogramManager::CheckFileIntegrity(TFile* inputFile){
	if (inputFile->GetDirectory("TimeRandomSubtacted") && inputFile->GetDirectory("EventMixed")) {
		std::cout << "Found time-random subtracted histograms" << std::endl;
		std::cout << "Found event mixed histograms" << std::endl;
		return true;
	} else {
		std::cout << "Could not find required histograms ... exiting" << std::endl;
		return false;
	}
} // CheckFileIntegrity
