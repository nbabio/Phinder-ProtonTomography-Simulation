//----------------------------------------------------------------------//
// Plots from the simulation output                                     //
//                                                                      //
// Usage: root -l -b -q 'makePlotsOneProton.C("output.root")'           //
//----------------------------------------------------------------------//


void makePlotsOneProton(const char *fileName = "output.root") {
    TFile *f = TFile::Open(fileName);
    TTree *t = (TTree*) f->Get("hits");
    int ev, layer, trackID;
    double kin, time;
    t->SetBranchAddress("eventNumber", &ev);
    t->SetBranchAddress("layer", &layer);
    t->SetBranchAddress("genTrackID", &trackID);
    t->SetBranchAddress("kineticEnergy", &kin);
    t->SetBranchAddress("time", &time);

    //First hit (earliest time) of the primary in each (event, layer)
    std::map<std::pair<int,int>, std::pair<double,double>> first; // -> (time, kineticEnergy)
    for(Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        if(trackID != 1) continue;
        auto key = std::make_pair(ev, layer);
        if(!first.count(key) || time < first[key].first) first[key] = {time, kin};
    }

    TGraph *gE = new TGraph(), *gT = new TGraph();
    for(auto &p : first) {
        gE->AddPoint(p.first.second, p.second.second);
        gT->AddPoint(p.first.second, p.second.first);
    }
    TCanvas *c = new TCanvas("c", "", 1200, 500);
    c->Divide(2, 1);
    c->cd(1); gPad->DrawFrame(-0.5, 115, 7.5, 222, ";layer;Kinetic energy [MeV]"); gE->SetMarkerStyle(20); gE->Draw("P");
    c->cd(2); gPad->DrawFrame(-0.5, 0, 7.5, 15, ";layer;time [ns]");              gT->SetMarkerStyle(20); gT->Draw("P");
    c->SaveAs("firstHit.png");
}
