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
	fBGHistograms.clear();
	fBGSubtractedHistograms.clear();

	for (auto const &iter : fInputs.GetPromptHistograms())
	{
		GateAndProject(iter);
	}

	TFile out("out.root", "RECREATE");
	out.cd();
	for (auto const &h : fProjectedHistograms)
	{
		h->Write();
	}
	for (auto const &h : fBGHistograms)
	{
		h->Write();
	}
	for (auto const &h : fBGSubtractedHistograms)
	{
		h->Write();
	}
	out.Close();
}

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

/************************************************************
 * Creates projection of 2D histogram
 *
 * @param
 ***************************************************************/
void Fitting::GateAndProject(TH2 *hist)
{
	int gate_low = fInputs.GateLow();
	int gate_high = fInputs.GateHigh();
	int iterations = 25;
	double sigma = 1.5; // * Maybe fit original peak for sigma?
	Option_t *spec_options = "";

	std::cout << "Fitting histogram: " << hist->GetName() << std::endl;

	// 0 == x
	// true means no overflow/underflow bin
	TH1 *proj_raw = hist_proj(hist, 0, Form("proj_%s", hist->GetName()), true);
	TH1 *proj_sub = static_cast<TH1 *>(proj_raw->Clone("proj_sub"));
	TH1 *spec_background = TSpectrum::StaticBackground(proj_raw, iterations, spec_options);

	proj_sub->Add(spec_background, -1);

	TAxis *ax = proj_raw->GetXaxis();
	int bins = proj_raw->GetNbinsX();

	if (gate_low > gate_high)
	{
		std::cerr << "Gate values may be reversed, please check config file" << std::endl;
		int temp = gate_low;
		gate_low = gate_high;
		gate_high = temp;
	}

	// Finding bin value for gates
	int gate_bin_low = ax->FindBin(gate_low);
	int gate_bin_high = ax->FindBin(gate_high);
	int bg_start = ax->FindBin(fInputs.GateCentroid() + sigma * 3);

	double bincout_gate = proj_raw->Integral(gate_bin_low, gate_bin_high);
	double bincount_bg = spec_background->Integral(gate_bin_low, gate_bin_high);
	double backfrac = bincount_bg / bincout_gate;

	TH1 *gate_hist = hist_gater_bin(gate_bin_low, gate_bin_high, hist, 0, Form("gate_hist_%s", hist->GetName()));

	TH1 *compton_hist = hist_gater_bin(bg_start, hist, 0, Form("c_gate_%s", hist->GetName()));
	TH1 *output_hist = scaled_back_subtract(gate_hist, compton_hist, backfrac, 0.04);

	fProjectedHistograms.push_back(proj_raw);
	fBGHistograms.push_back(compton_hist);
	fBGSubtractedHistograms.push_back(output_hist);
}

/************************************************************
 * Reference
 ***************************************************************/
std::vector<TH1 *> auto_gate(TH2 *raw_input, int xyz = 0, int itr = 25, Option_t *specopt = "", double sigma = 1.5, double thresh = 0.05, double Rd = 1, double Ru = -1)
{
	std::vector<TH1 *> ret;

	TH1 *proj = hist_proj(raw_input, xyz, "projraw", true);
	TH1 *projsub = (TH1 *)proj->Clone("projsub");
	TH1 *specback = TSpectrum::StaticBackground(proj, itr, specopt);
	projsub->Add(specback, -1);

	TAxis *ax = proj->GetXaxis();
	int Nb = proj->GetNbinsX();
	//     double w=ax->GetBinWidth(1);

	TSpectrum *s = new TSpectrum(200);
	Int_t nfound = s->Search(projsub, sigma, "", thresh);
	Double_t *xpeaks;
	xpeaks = s->GetPositionX();

	if (Rd > Ru)
	{
		Rd = ax->GetBinLowEdge(1);
		Ru = ax->GetBinLowEdge(Nb + 1);
	}

	for (int p = 0; p < nfound; p++)
	{
		Double_t xp = xpeaks[p];
		if (xp < Rd || xp > Ru)
			continue;

		int b1 = ax->FindBin(xp - sigma * 2);
		int b2 = ax->FindBin(xp + sigma * 2);
		int b3 = ax->FindBin(xp + sigma * 3);

		double ingatecount = proj->Integral(b1, b2);
		double background = specback->Integral(b1, b2);
		double backfrack = background / ingatecount;

		TH1 *gate_hist = hist_gater_bin(b1, b2, raw_input, xyz, "gate_hist");

		TH1 *compton_hist = hist_gater_bin(b3, raw_input, xyz, "c_gate");
		TH1 *output_hist = scaled_back_subtract(gate_hist, compton_hist, backfrack, 0.04);
		delete compton_hist;
		delete gate_hist;

		stringstream ss;
		ss << "GateOn" << xp;
		output_hist->SetName(ss.str().c_str());
		ret.push_back(output_hist);
	}

	return ret;
}