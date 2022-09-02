//////////////////////////////////////////////////////////////////////////////////
// Fits peaks
//
// Author:          Connor Natzke (cnatzke@triumf.ca)
// Creation Date:   Tuesday May 12, 2020    T15:39:33-07:00
// Last Update:     Tuesday May 12, 2020    T15:39:33-07:00
//////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include "TH1.h"
#include "TH2.h"
#include "TPeakFitter.h"
#include "TRWPeak.h"
#include "TFile.h"

#include "Fitting.h"
#include "j_env.h"

/*************************************************************
 * Constructor
 ***************************************************************/
Fitting::Fitting(Inputs &inputs) : fInputs(inputs)
{
	TH1::SetDefaultSumw2(); // correctly propagate errors when summing histograms
							// std::cout << "Fitting Manager intialized" << std::endl;
}

/**************************************************************
 * Destructor
 ***************************************************************/
Fitting::~Fitting()
{
	// std::cout << "Fitting Manager deleted" << std::endl;
}

/**************************************************************
 * Gates and fits projections
 *
 * @param histList List of 2D gamma gamma matrices
 ***************************************************************/
void Fitting::GateAndFit()
{
	fProjectedHistograms.clear();
	fComptonHistograms.clear();
	fBGSubtractedHistograms.clear();

	for (auto const &iter : fInputs.GetPromptHistograms())
	{
		GateAndProject(iter);
		std::cout << iter->GetName() << std::endl;
	}

	TFile out("out.root", "RECREATE");
	std::cout << "Writing histograms to file:" << out.GetName() << std::endl;
	out.cd();
	for (auto const &h : fProjectedHistograms)
	{
		h->Write();
	}
	for (auto const &h : fGateHistograms)
	{
		h->Write();
	}
	for (auto const &h : fComptonHistograms)
	{
		h->Write();
	}
	for (auto const &h : fBGSubtractedHistograms)
	{
		h->Write();
	}
	out.Close();
}

/************************************************************
 * Creates projection of 2D histogram
 *
 * @param matrix gamma-gamma matrix
 ***************************************************************/
void Fitting::GateAndProject(TH2 *matrix)
{
	int gate_low = fInputs.GateLow();
	int gate_high = fInputs.GateHigh();
	int background_stat_uncertainty = 0.04; // from James's code
	// TH1 *output_hist = new TH1();

	fFitFunction = new TF1("quick_gaus_main", "gaus(0) + pol2(3)", 0, 0);
	fFitFunction->SetLineColor(1);

	TH1 *full_proj = hist_proj(matrix, 0, Form("%s_gate", matrix->GetName()), true);
	fProjectedHistograms.push_back(full_proj);

	TH1 *gate_hist = hist_gater_bin(gate_low - 1, gate_high, matrix, 0, Form("%s_coin", matrix->GetName()));
	gate_hist->SetLineColor(1);
	gate_hist->GetXaxis()->SetTitleOffset(1.0);
	fBGSubtractedHistograms.push_back(gate_hist);

	// Compton bg subtraction
	int compton_offset = gate_high + 2;

	TH1 *compton_hist = hist_gater_bin(compton_offset, matrix, 0, Form("%s_compton", matrix->GetName()));
	fComptonHistograms.push_back(compton_hist);

	GetBackgroundFraction(full_proj);
	std::cout << matrix->GetName() << " | " << fBackgroundSubtractionFactor << std::endl;
	/*
	   scaled_back_subtract(gate_hist, compton_hist, fBackgroundSubtractionFactor, output_hist, background_stat_uncertainty);

	   output_hist->SetTitle("");
   */
}

/************************************************************
 * Finds background subtraction factor
 *
 * @param hist projection along axis of gamma-gamma matrix
 ***************************************************************/
void Fitting::GetBackgroundFraction(TH1 *hist)
{
	double gate_count = hist->Integral(fInputs.GateLow(), fInputs.GateHigh(), "width");
	double background_count = gate_count;

	if (gate_count <= 0)
	{
		fBackgroundSubtractionFactor = 0;
		return;
	}

	TF1 *temp = QuickSingleGausAutoFit(hist, fInputs.GateCentroid(), fInputs.BgGateLow(), fInputs.BgGateHigh());
	fFitFunction->SetParameters(temp->GetParameters());
	fFitFunction->SetRange(temp->GetXmin(), temp->GetXmax());
	delete temp;

	// calculate background fraction from bg fit
	double x0, x1, m, c, d;
	x0 = hist->GetBinLowEdge(fInputs.GateLow());
	x1 = hist->GetBinLowEdge(fInputs.GateHigh() + 1);
	m = fFitFunction->GetParameter(4);
	c = fFitFunction->GetParameter(3);
	d = fFitFunction->GetParameter(5);

	background_count = 0.3333 * d * std::pow(x1, 3) + 0.5 * m * std::pow(x1, 2) + c * x1 - (0.3333 * d * std::pow(x0, 3) + 0.5 * m * std::pow(x0, 2) + c * x0);

	fBackgroundSubtractionFactor = background_count / gate_count;
	if (!fBackgroundSubtractionFactor)
		fBackgroundSubtractionFactor = 1;
	if (!(fBackgroundSubtractionFactor <= 1 && fBackgroundSubtractionFactor >= 0))
		fBackgroundSubtractionFactor = 0; // INF guard

} // GetBackgroundFraction

/**************************************************************
 * Fits gamma single peaks
 *
 * @param histList List of 2D gamma gamma matrices
 * @param low_proj Low value of projection range
 * @param high_proj High value of projection range
 * @param low_fit Low value of fit range
 * @param high_fit High value of fit range
 ***************************************************************/
std::vector<TH1D *> Fitting::GenerateProjections(TList *histList, float low_proj, float high_proj, float low_fit, float high_fit)
{

	int verbose = 1;
	TH2D *hist = NULL;
	TH1D *proj_hist = NULL;
	std::vector<TH1D *> proj_vec;

	if (low_proj >= high_proj)
	{
		std::cout << "Incorrect projection range: " << low_proj << " - " << high_proj << std::endl;
		return proj_vec;
	}
	if (low_fit >= high_fit)
	{
		std::cout << "Incorrect fitting range: " << low_fit << " - " << high_fit << std::endl;
		return proj_vec;
	}

	// creating projections
	for (int i = 0; i < histList->GetSize(); i++)
	{
		hist = (TH2D *)histList->At(i);
		hist->GetYaxis()->SetRangeUser(low_proj, high_proj);
		proj_hist = hist->ProjectionX();
		proj_hist->SetName(Form("projection_%d", i));
		proj_hist->GetXaxis()->SetRangeUser(low_fit - 10, high_fit + 10);
		proj_vec.push_back(proj_hist);
	}

	if (verbose > 0)
	{
		std::cout << "Generated " << proj_vec.size() << " projection histograms." << std::endl;
	}

	return proj_vec;
} // GenerateProjections

/**************************************************************
 * Clones projection histograms
 *
 * @param histList Vector of histograms to be cloned
 ***************************************************************/
std::vector<TH1D *> Fitting::CloneProjections(std::vector<TH1D *> histVec, float lowFit, float highFit)
{

	int verbose = 1;
	std::vector<TH1D *> cloned_hist_vec;
	TH1D *total_proj = NULL;
	TH1D *temp_hist;

	if (verbose > 0)
	{
		std::cout << "Cloning " << histVec.size() << " projection histograms." << std::endl;
	}

	for (unsigned int i = 0; i < histVec.size(); i++)
	{
		temp_hist = (TH1D *)histVec.at(i)->Clone(Form("projection_binned_%d", i));
		temp_hist->GetXaxis()->SetRangeUser(lowFit - 10, highFit + 10);
		if (i == 0)
		{
			total_proj = (TH1D *)histVec.at(i)->Clone("total_proj");
		}
		else
		{
			total_proj->Add(histVec.at(i));
		}
		cloned_hist_vec.push_back(temp_hist);
	}

	// place total projection at beginning of vector
	cloned_hist_vec.insert(cloned_hist_vec.begin(), total_proj);

	if (verbose > 0)
	{
		std::cout << "Cloned " << cloned_hist_vec.size() - 1 << " projection histograms." << std::endl;
	}

	return cloned_hist_vec;

} // CloneProjections

/************************************************************
 * Fits peak
 *
 * @param
 ***************************************************************/
TRWPeak *Fitting::FitPeak(TH1D *inputHist, float peakPos, float lowFit, float highFit)
{
	int verbose = 0;

	TPeakFitter *pf = new TPeakFitter(lowFit, highFit);
	TRWPeak *peak = new TRWPeak(peakPos);

	// using initial Guess
	peak->GetFitFunction()->SetParameter(1, peakPos);
	peak->GetFitFunction()->SetParameter(2, 1.0);

	pf->AddPeak(peak);
	pf->Fit(inputHist, "L");

	if (verbose > 0)
	{
		std::cout << "====================" << std::endl;
		std::cout << "Fit parameters: " << std::endl;
		for (int i = 0; i < peak->GetFitFunction()->GetNpar(); ++i)
		{
			double min, max;
			peak->GetFitFunction()->GetParLimits(i, min, max);
			std::cout << i << ": " << peak->GetFitFunction()->GetParameter(i) << "; " << min << " - " << max << std::endl;
		}
		std::cout << "====================" << std::endl;
		std::cout << "Summed peak area = " << peak->Area() << " +- " << peak->AreaErr() << std::endl;
		peakPos = peak->GetFitFunction()->GetParameter(1);
		double sigma = peak->GetFitFunction()->GetParameter(2);
		std::cout << "Fixing peak position to " << peakPos << ", and sigma to " << sigma << " (FWHM = " << 2.35 * sigma << ")" << std::endl;
	}

	if (peak->Area() < 1.)
	{
		std::cerr << "Error in fitting: either the fit went wrong, or there isn't enough statistics to do anything" << std::endl;
	}

	return peak;

} // FitPeak