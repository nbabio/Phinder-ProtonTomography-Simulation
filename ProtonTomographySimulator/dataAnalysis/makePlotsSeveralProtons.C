//----------------------------------------------------------------------//
// Plots from the simulation output                                     //
//                                                                      //
// Usage: root -l -b -q 'makePlotsSeveralProtons.C("output.root", "directoryPlots")'           //
//----------------------------------------------------------------------//
#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TString.h"
#include "TH1D.h"
#include "TF1.h"
#include "TLegend.h"
#include "TLine.h"
#include "TMath.h"
#include "TEfficiency.h"
#include "TProfile.h"

#include <vector>
#include <algorithm>
#include <map>
#include <tuple>
#include <unordered_map>



//----------------------------------------------------------------------//
// Draws varY:varX of a tree into a 2D histogram with automatic ranges  //
//----------------------------------------------------------------------//
TH2D *makeTH2(TTree *t, const char *varX, const char *varY, const char *name,
              const char *title, int nbinsX = 200, int nbinsY = 200, const char *cut = "") {
    double xmin = t->GetMinimum(varX), xmax = t->GetMaximum(varX);
    double ymin = t->GetMinimum(varY), ymax = t->GetMaximum(varY);
    double mx = 0.02*(xmax - xmin), my = 0.02*(ymax - ymin);  // small margin
    TH2D *h = new TH2D(name, title, nbinsX, xmin - mx, xmax + mx, nbinsY, ymin - my, ymax + my);
    t->Draw(Form("%s:%s>>%s", varY, varX, name), "", "goff");
    return h;
}

//----------------------------------------------------------------------//
// Plot 1: position of the scintillation photons at creation            //
//----------------------------------------------------------------------//
void plotPhotonCreationPosition(TFile *f, const char *outDir) {

    TTree *t = (TTree*) f->Get("photons");
    if(!t) { printf("No 'photons' tree in file\n"); return; }
    printf("photons tree: %lld entries\n", t->GetEntries());

    TH2D *hXY = makeTH2(t, "x", "y", "hPhotXY", "Photon creation position;x [cm];y [cm]");
    TH2D *hZY = makeTH2(t, "z", "y", "hPhotZY", "Photon creation position;z [cm];y [cm]");
    TH2D *hZX = makeTH2(t, "z", "x", "hPhotZX", "Photon creation position;z [cm];x [cm]");

    TCanvas *c = new TCanvas("cPhotPos", "Photon creation position", 1800, 600);
    c->Divide(3, 1);
    c->cd(1); gPad->SetLogz(); gPad->SetRightMargin(0.15); hXY->Draw("COLZ");
    c->cd(2); gPad->SetLogz(); gPad->SetRightMargin(0.15); hZY->Draw("COLZ");
    c->cd(3); gPad->SetLogz(); gPad->SetRightMargin(0.15); hZX->Draw("COLZ");

    c->SaveAs(Form("%s/photonCreationPosition.png", outDir));
    c->SaveAs(Form("%s/photonCreationPosition.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 2: number of scintillation photons generated per proton         //
// (one proton per event is assumed)                                    //
//----------------------------------------------------------------------//
void plotPhotonsPerProton(TFile *f, const char *outDir, int nEvents) {

    TTree *t = (TTree*) f->Get("photons");
    if(!t) { printf("No 'photons' tree in file\n"); return; }

    int evt;
    t->SetBranchStatus("*", 0);             // only read eventNumber -> much faster
    t->SetBranchStatus("eventNumber", 1);
    t->SetBranchAddress("eventNumber", &evt);

    //Count photons per event; events without photons stay at 0
    int maxEvt = (int) t->GetMaximum("eventNumber");
    std::vector<long> nPhot(std::max(nEvents, maxEvt + 1), 0);
    for(Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        nPhot[evt]++;
    }
    t->ResetBranchAddresses(); t->SetBranchStatus("*", 1);

    long maxN = *std::max_element(nPhot.begin(), nPhot.end());
    TH1D *h = new TH1D("hPhotPerProton", "Scintillation photons per proton;N_{#gamma} generated;Protons",
                       100, 0, 1.05*maxN);
    for(long n : nPhot) h->Fill(n);

    printf("Protons: %zu, mean photons/proton: %.1f, RMS: %.1f, protons with 0 photons: %ld\n",
           nPhot.size(), h->GetMean(), h->GetRMS(), (long) std::count(nPhot.begin(), nPhot.end(), 0L));

    TCanvas *c = new TCanvas("cPhotPerProton", "Photons per proton", 900, 700);
    h->SetLineColor(kAzure + 2);
    h->SetFillColorAlpha(kAzure + 2, 0.3);
    h->Draw("HIST");
    c->SaveAs(Form("%s/photonsPerProton.png", outDir));
    c->SaveAs(Form("%s/photonsPerProton.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 3: energy deposited in the fibers per proton                    //
// (sum of all the hits of the event, one proton per event is assumed)  //
//----------------------------------------------------------------------//
void plotEnergyPerProton(TFile *f, const char *outDir, int nEvents) {

    TTree *t = (TTree*) f->Get("hits");
    if(!t) { printf("No 'hits' tree in file\n"); return; }

    int evt;
    double edep;
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("eventNumber", 1);
    t->SetBranchStatus("energy", 1);
    t->SetBranchAddress("eventNumber", &evt);
    t->SetBranchAddress("energy", &edep);

    //Sum the deposited energy per event; events without hits stay at 0
    int maxEvt = (int) t->GetMaximum("eventNumber");
    std::vector<double> eProton(std::max(nEvents, maxEvt + 1), 0.0);
    for(Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        eProton[evt] += edep;
    }
    t->ResetBranchAddresses(); t->SetBranchStatus("*", 1);

    double maxE = *std::max_element(eProton.begin(), eProton.end());
    TH1D *h = new TH1D("hEdepPerProton", "Energy deposited in the fibers per proton;E_{dep} [MeV];Protons",
                       100, 0, 1.05*maxE);
    for(double e : eProton) h->Fill(e);

    printf("Protons: %zu, mean Edep/proton: %.3f MeV, RMS: %.3f MeV\n",
           eProton.size(), h->GetMean(), h->GetRMS());

    TCanvas *c = new TCanvas("cEdepPerProton", "Energy deposited per proton", 900, 700);
    h->SetLineColor(kOrange + 7);
    h->SetFillColorAlpha(kOrange + 7, 0.3);
    h->Draw("HIST");
    c->SaveAs(Form("%s/energyPerProton.png", outDir));
    c->SaveAs(Form("%s/energyPerProton.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 4: time distribution of the scintillation photons at creation   //
// Left: global time. Right: time relative to the first photon of the   //
// event (removes the beam t0 spread), with an exponential fit.         //
//----------------------------------------------------------------------//
void plotPhotonCreationTime(TFile *f, const char *outDir, int nEvents) {

    TTree *t = (TTree*) f->Get("photons");
    if(!t) { printf("No 'photons' tree in file\n"); return; }

    int evt, det, layer;
    double time;
    t->SetBranchStatus("*", 0);
    t->SetBranchStatus("eventNumber", 1);
    t->SetBranchStatus("det", 1);
    t->SetBranchStatus("layer", 1);
    t->SetBranchStatus("time", 1);
    t->SetBranchAddress("eventNumber", &evt);
    t->SetBranchAddress("det", &det);
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("time", &time);


    //First pass: time of the first photon of each (event, detector, layer)
    // -> removes the beam t0 spread and the proton time of flight
    std::map<std::tuple<int,int,int>, double> tFirst;
    for(Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        auto key = std::make_tuple(evt, det, layer);
        auto it = tFirst.find(key);
        if(it == tFirst.end() || time < it->second) tFirst[key] = time;
    }


    //Second pass: fill the histograms
    // Photons from radioactive decays (minutes-hours later) are counted but not plotted
    const double tWindow = 50.;  // ns
    TH1D *hAbs = new TH1D("hPhotTimeAbs", "Photon creation time;t [ns];Photons", 200, -15, tWindow);
    TH1D *hRel = new TH1D("hPhotTimeRel", "Photon creation time relative to first photon in the layer;t - t_{first} [ns];Photons",
                          200, 0, 30);
    long nDelayed = 0;
    for(Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        if(time > 1000.) nDelayed++;   // > 1 us
        hAbs->Fill(time);
        hRel->Fill(time - tFirst[std::make_tuple(evt, det, layer)]);
    }
    t->ResetBranchAddresses(); t->SetBranchStatus("*", 1);
    printf("Delayed photons (t > 1 us, radioactive decays): %ld of %lld\n", nDelayed, t->GetEntries());


    //Exponential fit to the tail -> scintillation decay constant
    TF1 *fExp = new TF1("fExp", "[0]*exp(-x/[1])", 0.5, 30);
    fExp->SetParameters(hRel->GetMaximum(), 3.0);
    fExp->SetParNames("A", "#tau");
    hRel->Fit(fExp, "RQ0");
    printf("Decay constant from fit: tau = %.2f +- %.2f ns\n", fExp->GetParameter(1), fExp->GetParError(1));

    TCanvas *c = new TCanvas("cPhotTime", "Photon creation time", 1600, 700);
    c->Divide(2, 1);
    c->cd(1); // gPad->SetLogy();
    hAbs->SetLineColor(kAzure + 2);
    hAbs->Draw("HIST");
    c->cd(2); gPad->SetLogy();
    hRel->SetLineColor(kAzure + 2);
    hRel->Draw("HIST");
    // fExp->SetLineColor(kRed + 1);
    // fExp->Draw("SAME");

    c->SaveAs(Form("%s/photonCreationTime.png", outDir));
    c->SaveAs(Form("%s/photonCreationTime.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 5: spatial distribution of the signal photons (exitPhotons)     //
// End-face photons (|localz| = fiber half length) are the signal;      //
// the rest escape through the lateral surface of the cladding.         //
//----------------------------------------------------------------------//
void plotSignalPhotonPosition(TFile *f, const char *outDir) {

    TTree *t = (TTree*) f->Get("exitPhotons");
    if(!t || t->GetEntries() == 0) { printf("No 'exitPhotons' in file\n"); return; }

    double halfL = t->GetMaximum("localz");
    TString endCut = Form("abs(localz) > %f", halfL - 1e-3);
    TString latCut = "!(" + endCut + ")";
    long nEnd = t->GetEntries(endCut), nLat = t->GetEntries(latCut);
    printf("exitPhotons: %lld (fiber ends: %ld, lateral: %ld), fiber half length: %.2f cm\n",
           t->GetEntries(), nEnd, nLat, halfL);

    TH2D *hXY  = makeTH2(t, "x", "y", "hSigXY", "Signal photons exit position;x [cm];y [cm]", 200, 200, endCut);
    TH2D *hZY  = makeTH2(t, "z", "y", "hSigZY", "Signal photons exit position;z [cm];y [cm]", 200, 200, endCut);
    TH2D *hEnd = makeTH2(t, "localx", "localy", "hSigEndFace",
                         "Exit point on the fiber end face;local x [cm];local y [cm]", 100, 100, endCut);
    TH1D *hLat = new TH1D("hSigLateral", "Photons escaping through the lateral surface;local z [cm];Photons",
                          200, -1.02*halfL, 1.02*halfL);
    t->Draw("localz>>hSigLateral", latCut, "goff");

    TCanvas *c = new TCanvas("cSigPos", "Signal photons position", 2100, 700);
    c->Divide(3, 1);
    c->cd(1); gPad->SetLogz(); gPad->SetRightMargin(0.15); hXY->Draw("COLZ");
    c->cd(2); gPad->SetLogz(); gPad->SetRightMargin(0.15); hZY->Draw("COLZ");
    c->cd(3); gPad->SetLogz(); gPad->SetRightMargin(0.15); hEnd->Draw("COLZ");
    // c->cd(4); gPad->SetLogy(); hLat->SetLineColor(kOrange + 7); hLat->Draw("HIST");

    c->SaveAs(Form("%s/signalPhotonPosition.png", outDir));
    c->SaveAs(Form("%s/signalPhotonPosition.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 6: time distribution of the signal photons (fiber end faces)    //
// Left: global exit time (end faces vs lateral escapes).               //
// Middle: transit time in the fiber, t_exit - t_creation (matched by   //
//         eventNumber and trackID with the "photons" tree).            //
// Right: exit time relative to the first photon created in the same    //
//        (event, det, layer) -> pulse shape without beam t0 and TOF.   //
//----------------------------------------------------------------------//
void plotSignalPhotonTime(TFile *f, const char *outDir) {

    TTree *tc = (TTree*) f->Get("photons");
    TTree *te = (TTree*) f->Get("exitPhotons");
    if(!tc || !te || te->GetEntries() == 0) { printf("Missing 'photons' or 'exitPhotons'\n"); return; }

    double halfL = te->GetMaximum("localz");
    auto key = [](int e, int tr) { return ((long long) e << 32) | (unsigned int) tr; };

    //Exit photons
    int eEvt, eDet, eLayer, eTrack, eSide;
    double eTime, eLz;
    te->SetBranchStatus("*", 0);
    for(auto b : {"eventNumber", "det", "layer", "trackID", "time", "localz", "side"}) te->SetBranchStatus(b, 1);
    te->SetBranchAddress("eventNumber", &eEvt);
    te->SetBranchAddress("det", &eDet);
    te->SetBranchAddress("layer", &eLayer);
    te->SetBranchAddress("trackID", &eTrack);
    te->SetBranchAddress("time", &eTime);
    te->SetBranchAddress("localz", &eLz);
    te->SetBranchAddress("side", &eSide);

    //Created photons
    int cEvt, cDet, cLayer, cTrack;
    double cTime;
    tc->SetBranchStatus("*", 0);
    for(auto b : {"eventNumber", "det", "layer", "trackID", "time"}) tc->SetBranchStatus(b, 1);
    tc->SetBranchAddress("eventNumber", &cEvt);
    tc->SetBranchAddress("det", &cDet);
    tc->SetBranchAddress("layer", &cLayer);
    tc->SetBranchAddress("trackID", &cTrack);
    tc->SetBranchAddress("time", &cTime);

    //Pass 1: which created photons leave the fibers (only those are stored -> low memory)
    std::unordered_map<long long, double> tCreate;
    for(Long64_t i = 0; i < te->GetEntries(); i++) {
        te->GetEntry(i);
        tCreate[key(eEvt, eTrack)] = -1e30;
    }

    //Pass 2: creation time of those photons + first created photon per (event, det, layer)
    std::map<std::tuple<int,int,int>, double> tFirst;
    for(Long64_t i = 0; i < tc->GetEntries(); i++) {
        tc->GetEntry(i);
        auto k = std::make_tuple(cEvt, cDet, cLayer);
        auto it = tFirst.find(k);
        if(it == tFirst.end() || cTime < it->second) tFirst[k] = cTime;
        auto jt = tCreate.find(key(cEvt, cTrack));
        if(jt != tCreate.end()) jt->second = cTime;
    }

    //Pass 3: fill
    TH1D *hAbsEndP = new TH1D("hSigTimeAbsEndP", "Signal photons exit time;t_{exit} [ns];Photons", 200, -15, 60);
    TH1D *hAbsEndM = new TH1D("hSigTimeAbsEndM", "", 200, -15, 60);
    TH1D *hAbsLat = new TH1D("hSigTimeAbsLat", "", 200, -15, 60);
    TH1D *hTransit = new TH1D("hSigTransit", "Transit time in the fiber (fiber ends);t_{exit} - t_{creation} [ns];Photons",
                              200, 0, 8);
    TH1D *hRel = new TH1D("hSigTimeRel", "Signal photons exit time relative to first photon in the layer;t_{exit} - t_{first} [ns];Photons",
                          200, 0, 20);
    long nDelayed = 0;
    long nUnmatched = 0;

    for(Long64_t i = 0; i < te->GetEntries(); i++) {
        te->GetEntry(i);
        if(eTime > 1000.) { nDelayed++; continue; }   // radioactive decays
        bool isEnd = std::abs(eLz) > halfL - 1e-3;
        if(!isEnd) { hAbsLat->Fill(eTime); continue; }
        (eSide > 0 ? hAbsEndP : hAbsEndM)->Fill(eTime);
        double tc0 = tCreate[key(eEvt, eTrack)];
        if(tc0 < -1e29) { nUnmatched++; continue; }
        hTransit->Fill(eTime - tc0);
        hRel->Fill(eTime - tFirst[std::make_tuple(eEvt, eDet, eLayer)]);
    }
    te->ResetBranchAddresses(); te->SetBranchStatus("*", 1);
    tc->ResetBranchAddresses(); tc->SetBranchStatus("*", 1);
    printf("Signal photons: delayed (t > 1 us): %ld, without matching created photon: %ld\n", nDelayed, nUnmatched);
    printf("Mean transit time: %.2f ns, RMS: %.2f ns\n", hTransit->GetMean(), hTransit->GetRMS());

    TCanvas *c = new TCanvas("cSigTime", "Signal photons time", 2100, 700);
    c->Divide(3, 1);
    c->cd(1); gPad->SetLogy();
    hAbsEndP->SetMaximum(1.2*std::max(hAbsEndP->GetMaximum(), hAbsEndM->GetMaximum()));
    hAbsEndP->SetLineColor(kAzure + 2); hAbsEndP->Draw("HIST");
    hAbsEndM->SetLineColor(kOrange + 7); hAbsEndM->Draw("HIST SAME");
    // hAbsLat->SetLineColor(kOrange + 7); hAbsLat->Draw("HIST SAME");
    TLegend *leg = new TLegend(0.55, 0.75, 0.88, 0.88);
    leg->AddEntry(hAbsEndP, "Fiber ends (signal, positive side)", "l");
    leg->AddEntry(hAbsEndM, "Fiber ends (signal, negative side)", "l");
    // leg->AddEntry(hAbsLat, "Lateral escapes", "l");
    leg->Draw();
    c->cd(2); // gPad->SetLogy();
    hTransit->SetLineColor(kAzure + 2); hTransit->Draw("HIST");
    c->cd(3); // gPad->SetLogy();
    hRel->SetLineColor(kAzure + 2); hRel->Draw("HIST");

    c->SaveAs(Form("%s/signalPhotonTime.png", outDir));
    c->SaveAs(Form("%s/signalPhotonTime.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 7: wavelength and energy spectra of the created photons         //
// (scintillation / Cherenkov) and of the signal photons (fiber ends).  //
// Top: normalized spectra. Bottom: signal / created (transport         //
// efficiency of the fiber as a function of wavelength / energy).       //
//----------------------------------------------------------------------//
void plotPhotonSpectra(TFile *f, const char *outDir) {

    TTree *tc = (TTree*) f->Get("photons");
    TTree *te = (TTree*) f->Get("exitPhotons");
    if(!tc || !te || te->GetEntries() == 0) { printf("Missing 'photons' or 'exitPhotons'\n"); return; }

    double halfL = te->GetMaximum("localz");
    TString endCut = Form("abs(localz) > %f", halfL - 1e-3);

    const char *vars[2] = {"wavelength", "energy"};
    const char *axis[2] = {"#lambda [nm]", "E [eV]"};
    double lo[2] = {370., 2.3}, hi[2] = {530., 3.4};
    const int nb = 80;

    TCanvas *c = new TCanvas("cSpectra", "Photon spectra", 1600, 1200);
    c->Divide(2, 2);

    for(int k = 0; k < 2; k++) {
        TH1D *hScint = new TH1D(Form("hScint_%s", vars[k]), Form("Photon spectra;%s;Normalized", axis[k]), nb, lo[k], hi[k]);
        TH1D *hCher  = new TH1D(Form("hCher_%s",  vars[k]), "", nb, lo[k], hi[k]);
        TH1D *hSig   = new TH1D(Form("hSig_%s",   vars[k]), "", nb, lo[k], hi[k]);
        tc->Draw(Form("%s>>hScint_%s", vars[k], vars[k]), "process==0", "goff");
        tc->Draw(Form("%s>>hCher_%s",  vars[k], vars[k]), "process==1", "goff");
        te->Draw(Form("%s>>hSig_%s",   vars[k], vars[k]), endCut, "goff");
        if(k == 0) printf("Created: scintillation %.0f, Cherenkov %.0f; signal (fiber ends): %.0f\n",
                          hScint->Integral(), hCher->Integral(), hSig->Integral());

        //Transport efficiency: signal / all created (before normalizing)
        TH1D *hEff = (TH1D*) hSig->Clone(Form("hEff_%s", vars[k]));
        TH1D *hAll = (TH1D*) hScint->Clone(Form("hAll_%s", vars[k]));
        hAll->Add(hCher);
        hEff->Divide(hSig, hAll, 1., 1., "B");   // binomial errors
        hEff->SetTitle(Form("Transport efficiency;%s;Signal / created", axis[k]));

        //Normalize to unit area to compare shapes
        double ymax = 0;
        for(TH1D *h : {hScint, hCher, hSig}) {
            if(h->Integral() > 0) h->Scale(1./h->Integral());
            ymax = std::max(ymax, h->GetMaximum());
        }

        c->cd(k + 1);
        hScint->SetMaximum(1.15*ymax);
        hScint->SetLineColor(kAzure + 2); hScint->SetLineWidth(2); hScint->Draw("HIST");
    //    hCher->SetLineColor(kGray + 1);   hCher->SetLineWidth(2);  hCher->Draw("HIST SAME");
        hSig->SetLineColor(kOrange + 7);  hSig->SetLineWidth(2);   hSig->Draw("HIST SAME");
        TLegend *leg = new TLegend(0.60, 0.72, 0.88, 0.88);
        leg->AddEntry(hScint, "Created: scintillation", "l");
    //    leg->AddEntry(hCher,  "Created: Cherenkov", "l");
        leg->AddEntry(hSig,   "Signal (fiber ends)", "l");
        leg->Draw();

        c->cd(k + 3);
        hEff->SetLineColor(kOrange + 7); hEff->SetMarkerColor(kOrange + 7); hEff->SetMarkerStyle(20);
        hEff->SetMarkerSize(0.6);
        hEff->Draw("E");
    }

    c->SaveAs(Form("%s/photonSpectra.png", outDir));
    c->SaveAs(Form("%s/photonSpectra.pdf", outDir));
}

//----------------------------------------------------------------------//
// Plot 8: photon transport in the fibers. Created photons ("photons")  //
// are matched by (eventNumber, trackID) with the signal photons        //
// (exitPhotons on the fiber end faces).                                //
// - Angular acceptance and trapping efficiency per (det, layer)        //
// - Propagation time vs track length, optical path vs 1/|cos|          //
// - Exit angle in air                                                  //
// Refractive indices of the core and cladding are hardcoded            //
//----------------------------------------------------------------------//
void plotPhotonTransport(TFile *f, const char *outDir, double nCore = 1.59, double nClad = 1.49) {

    TTree *tc = (TTree*) f->Get("photons");
    TTree *te = (TTree*) f->Get("exitPhotons");
    if(!tc || !te || te->GetEntries() == 0) { printf("Missing 'photons' or 'exitPhotons'\n"); return; }

    double halfL = te->GetMaximum("localz");
    auto key = [](int e, int tr) { return ((long long) e << 32) | (unsigned int) tr; };

    //Created photons
    int cEvt, cDet, cLayer, cTrack;
    double cDirz, cLocalz, cTime;
    tc->SetBranchStatus("*", 0);
    for(auto b : {"eventNumber", "det", "layer", "trackID", "dirz", "localz", "time"}) tc->SetBranchStatus(b, 1);
    tc->SetBranchAddress("eventNumber", &cEvt);
    tc->SetBranchAddress("det", &cDet);
    tc->SetBranchAddress("layer", &cLayer);
    tc->SetBranchAddress("trackID", &cTrack);
    tc->SetBranchAddress("dirz", &cDirz);
    tc->SetBranchAddress("localz", &cLocalz);
    tc->SetBranchAddress("time", &cTime);

    //Exit photons
    int eEvt, eTrack, eVol;
    double eLocalz, eTime, eLength, eDirOutz;
    te->SetBranchStatus("*", 0);
    for(auto b : {"eventNumber", "trackID", "exitVolume", "localz", "time", "trackLength", "dirOutz"}) te->SetBranchStatus(b, 1);
    te->SetBranchAddress("eventNumber", &eEvt);
    te->SetBranchAddress("trackID", &eTrack);
    te->SetBranchAddress("exitVolume", &eVol);
    te->SetBranchAddress("localz", &eLocalz);
    te->SetBranchAddress("time", &eTime);
    te->SetBranchAddress("trackLength", &eLength);
    te->SetBranchAddress("dirOutz", &eDirOutz);

    //Pass 1: signal photons (fiber end faces) -> only those are stored
    struct CInfo { double dirz = 0, localz = 0, time = 0; int det = -1, layer = -1; };
    std::unordered_map<long long, CInfo> created;
    for(Long64_t i = 0; i < te->GetEntries(); i++) {
        te->GetEntry(i);
        if(std::abs(eLocalz) > halfL - 1e-3) created[key(eEvt, eTrack)];   // det = -1 -> not matched yet
    }

    TH1D *hCosAll  = new TH1D("hCosAll",  ";|cos#theta| at creation (w.r.t. fiber axis);Photons", 50, 0, 1);
    TH1D *hCosExit = new TH1D("hCosExit", ";|cos#theta| at creation (w.r.t. fiber axis);Photons", 50, 0, 1);
    TH2D *hDtL     = new TH2D("hDtL",     ";Track length [cm];t_{exit} - t_{creation} [ns]", 100, 30, 130, 100, 0, 10);
    TH2D *hPathCos = new TH2D("hPathCos", ";1/|cos#theta| at creation;Track length / |#Deltaz|", 100, 1, 1.5, 100, 1, 1.5);
    TH1D *hOut     = new TH1D("hOut",     ";Exit angle in air #theta_{out} [deg];Photons", 90, 0, 90);
    //Efficiency per (det, layer): bin = 10*det + layer
    TH1D *hDLAll   = new TH1D("hDLAll",  "", 40, -0.5, 39.5);
    TH1D *hDLExit  = new TH1D("hDLExit", "", 40, -0.5, 39.5);

    //Pass 2: created photons
    for(Long64_t i = 0; i < tc->GetEntries(); i++) {
        tc->GetEntry(i);
        hCosAll->Fill(std::abs(cDirz));
        hDLAll->Fill(10*cDet + cLayer);
        auto it = created.find(key(cEvt, cTrack));
        if(it != created.end()) it->second = {cDirz, cLocalz, cTime, cDet, cLayer};
    }

    //Pass 3: signal photons
    long nEnd = 0, nMatched = 0, nCore0 = 0;
    for(Long64_t i = 0; i < te->GetEntries(); i++) {
        te->GetEntry(i);
        if(std::abs(eLocalz) <= halfL - 1e-3) continue;   // lateral escapes
        nEnd++;
        hOut->Fill(TMath::ACos(std::abs(eDirOutz)) * TMath::RadToDeg());
        if(eVol == 0) nCore0++;

        auto it = created.find(key(eEvt, eTrack));
        if(it == created.end() || it->second.det < 0) continue;
        const CInfo &c = it->second;
        double dz = std::abs(eLocalz - c.localz);
        double absCos = std::abs(c.dirz);
        nMatched++;
        hCosExit->Fill(absCos);
        hDLExit->Fill(10*c.det + c.layer);
        hDtL->Fill(eLength, eTime - c.time);
        if(dz > 0 && absCos > 0) hPathCos->Fill(1.0/absCos, eLength/dz);
    }
    tc->ResetBranchAddresses(); tc->SetBranchStatus("*", 1);
    te->ResetBranchAddresses(); te->SetBranchStatus("*", 1);

    //Summary
    double nC = tc->GetEntries();
    double cosCrit = nClad/nCore;
    printf("Signal photons (fiber ends): %ld (matched to a created photon: %ld)\n", nEnd, nMatched);
    printf("  from core / cladding: %ld / %ld\n", nCore0, nEnd - nCore0);
    printf("Trapping efficiency:    %.2f %% (%.2f %% per end)\n", 100*nEnd/nC, 50*nEnd/nC);
    printf("Meridional estimate:    %.2f %% per end, (1 - n_clad/n_core)/2\n", 50*(1 - cosCrit));
    printf("Critical angle:         %.1f deg (cos = %.3f)\n", TMath::ACos(cosCrit)*TMath::RadToDeg(), cosCrit);
    TProfile *pDtL = hDtL->ProfileX("pDtL");
    if(pDtL->GetEntries() > 2 && pDtL->Fit("pol1", "Q0") == 0) {
        double slope = pDtL->GetFunction("pol1")->GetParameter(1); // ns/cm
        printf("Effective speed:        %.2f cm/ns (c/n_core = %.2f cm/ns)\n", 1.0/slope, 29.98/nCore);
    }

    //Efficiencies (built before styling the histograms)
    TEfficiency *eff = new TEfficiency(*hCosExit, *hCosAll);
    eff->SetName("acceptance");
    eff->SetTitle(";|cos#theta| at creation;Fraction that reaches the fiber ends");
    TEfficiency *effDL = new TEfficiency(*hDLExit, *hDLAll);
    effDL->SetName("effDetLayer");
    effDL->SetTitle(";10 #times det + layer;Trapping efficiency");
    hCosAll->SetLineColor(kAzure + 2);  hCosAll->SetLineWidth(2);
    hCosExit->SetLineColor(kOrange + 7); hCosExit->SetLineWidth(2);

    //Angular acceptance
    TCanvas *c1 = new TCanvas("cTransAcc", "Angular acceptance", 1600, 700);
    c1->Divide(2, 1);
    c1->cd(1); gPad->SetLogy();
    hCosAll->SetMinimum(0.5);
    hCosAll->Draw("HIST"); hCosExit->Draw("HIST SAME");
    TLegend *leg = new TLegend(0.15, 0.75, 0.5, 0.88);
    leg->AddEntry(hCosAll, "Created", "l"); leg->AddEntry(hCosExit, "Signal (fiber ends)", "l"); leg->Draw();
    c1->cd(2);
    eff->Draw("AP");
    gPad->Update();
    TLine *lc = new TLine(cosCrit, 0, cosCrit, 1.05);
    lc->SetLineStyle(2); lc->SetLineColor(kGray + 2); lc->Draw();
    c1->SaveAs(Form("%s/photonAcceptance.png", outDir));
    c1->SaveAs(Form("%s/photonAcceptance.pdf", outDir));

    //Efficiency per detector/layer and exit angle in air
    TCanvas *c2 = new TCanvas("cTransEff", "Trapping efficiency and exit angle", 1600, 700);
    c2->Divide(2, 1);
    c2->cd(1); effDL->Draw("AP");
    c2->cd(2); hOut->SetLineColor(kAzure + 2); hOut->SetLineWidth(2); hOut->Draw("HIST");
    c2->SaveAs(Form("%s/trappingEfficiency_exitAngle.png", outDir));
    c2->SaveAs(Form("%s/trappingEfficiency_exitAngle.pdf", outDir));

    //Timing and optical path
    TCanvas *c3 = new TCanvas("cTransPath", "Propagation time and optical path", 1600, 700);
    c3->Divide(2, 1);
    c3->cd(1); gPad->SetRightMargin(0.15); hDtL->Draw("COLZ");
    c3->cd(2); gPad->SetRightMargin(0.15); hPathCos->Draw("COLZ");
    c3->SaveAs(Form("%s/propagationTime_opticalPath.png", outDir));
    c3->SaveAs(Form("%s/propagationTime_opticalPath.pdf", outDir));
}


//----------------------------------------------------------------------//
// Main                                                                 //
//----------------------------------------------------------------------//
void makePlotsSeveralProtons(const char *fileName = "output.root", const char *outDir = "plots", int nEvents = 0) {

    gStyle->SetOptStat(0);
    gStyle->SetPalette(kViridis);
    gSystem->mkdir(outDir, kTRUE);

    TFile *f = TFile::Open(fileName);
    if(!f || f->IsZombie()) { printf("Cannot open %s\n", fileName); return; }

    // Plots are added here, one call per plot
    plotPhotonTransport(f, outDir);
    plotPhotonCreationPosition(f, outDir);
    plotPhotonsPerProton(f, outDir, nEvents);
    plotEnergyPerProton(f, outDir, nEvents);
    plotPhotonCreationTime(f, outDir, nEvents);
    plotSignalPhotonPosition(f, outDir);
    plotSignalPhotonTime(f, outDir); 
    plotPhotonSpectra(f, outDir);

    f->Close();
}
