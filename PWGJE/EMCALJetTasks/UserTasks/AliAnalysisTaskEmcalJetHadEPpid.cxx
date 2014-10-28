// Jet-Hadron Correlations PID
// Event plane dependence task.
//
// Author: J Mazer

// task head include
#include "AliAnalysisTaskEmcalJetHadEPpid.h"

// general ROOT includes
#include <TCanvas.h>
#include <TChain.h>
#include <TClonesArray.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <THnSparse.h>
#include <TList.h>
#include <TLorentzVector.h>
#include <TParameter.h>
#include <TParticle.h>
#include <TTree.h>
#include <TVector3.h>
#include <TObjArray.h>

// AliROOT includes
#include "AliAODEvent.h"
#include "AliESDEvent.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisTask.h"
#include "AliCentrality.h"
#include "AliEmcalJet.h"
#include "AliAODJet.h"
#include "AliVCluster.h"
#include "AliVTrack.h"
#include <AliVEvent.h>
#include <AliVParticle.h>
#include "AliRhoParameter.h"
#include "AliEmcalParticle.h"

// Localized Rho includes
#include "AliLocalRhoParameter.h"
#include "AliAnalysisTaskLocalRho.h"

// event handler (and pico's) includes
#include <AliInputEventHandler.h>
#include <AliVEventHandler.h>
#include "AliESDInputHandler.h"
#include "AliPicoTrack.h"
#include "AliEventPoolManager.h"
#include "AliESDtrackCuts.h"

// PID includes
#include "AliPIDResponse.h"
#include "AliTPCPIDResponse.h"
#include "AliESDpid.h"

// magnetic field includes
#include "TGeoGlobalMagField.h"
#include "AliMagF.h"

// container includes
#include "AliJetContainer.h"
#include "AliParticleContainer.h"
#include "AliClusterContainer.h"

using std::cout;
using std::endl;

ClassImp(AliAnalysisTaskEmcalJetHadEPpid)

//________________________________________________________________________
AliAnalysisTaskEmcalJetHadEPpid::AliAnalysisTaskEmcalJetHadEPpid() : 
  AliAnalysisTaskEmcalJet("correlations",kFALSE), 
  fPhimin(-10), fPhimax(10),
  fEtamin(-0.9), fEtamax(0.9),
  fAreacut(0.0), fTrkBias(5), fClusBias(5), fTrkEta(0.9), 
  fJetPtcut(15.0), fJetRad(0.4), fConstituentCut(0.15),
  fesdTrackCuts(0),
  fDoEventMixing(0), fMixingTracks(50000), fNMIXtracks(5000), fNMIXevents(5),
  fTriggerEventType(AliVEvent::kAny), fMixingEventType(AliVEvent::kAny),
  doPlotGlobalRho(0), doVariableBinning(0), dovarbinTHnSparse(0), 
  makeQAhistos(0), makeBIAShistos(0), makeextraCORRhistos(0), makeoldJEThadhistos(0),
  allpidAXIS(0), fcutType("EMCAL"), doPID(0), doPIDtrackBIAS(0),
  doComments(0),
  doFlavourJetAnalysis(0), fJetFlavTag(-99),
  fBeam(kNA),
  fLocalRhoVal(0),
  fTracksName(""), fTracksNameME(""), fJetsName(""),
  event(0),
  isPItpc(0), isKtpc(0), isPtpc(0), // pid TPC
  isPIits(0), isKits(0), isPits(0), // pid ITS
  isPItof(0), isKtof(0), isPtof(0), // pid TOF
  fPoolMgr(0x0),
  fPIDResponse(0x0), fTPCResponse(),
  fESD(0), fAOD(0), fVevent(0),  
  fHistEventQA(0), fHistEventSelectionQA(0),
  fHistCentZvertGA(0), fHistCentZvertJE(0), fHistCentZvertMB(0), fHistCentZvertAny(0),
  fHistTPCdEdX(0), fHistITSsignal(0), //fHistTOFsignal(0),
  fHistRhovsCent(0), fHistNjetvsCent(0), fHistCentrality(0),
  fHistZvtx(0), fHistMult(0),
  fHistJetPhi(0), fHistTrackPhi(0),
  fHistLocalRhoJetpt(0),
  fHistJetHaddPhiIN(0), fHistJetHaddPhiOUT(0), fHistJetHaddPhiMID(0),
  fHistJetHaddPhiBias(0), fHistJetHaddPhiINBias(0), fHistJetHaddPhiOUTBias(0), fHistJetHaddPhiMIDBias(0),
  fHistMEdPHI(0),
  fHistTrackPtallcent(0),
  fHistJetEtaPhi(0), fHistJetHEtaPhi(0),
  fHistSEphieta(0), fHistMEphieta(0),
  fHistJetHaddPHI(0),
  fHistPID(0),
  fhnPID(0x0), fhnMixedEvents(0x0), fhnJH(0x0), fhnCorr(0x0),
  fJetsCont(0), fTracksCont(0), fCaloClustersCont(0),
  fContainerAllJets(0), fContainerPIDJets(1)
{
  // Default Constructor 
  for(Int_t ilab=0; ilab<4; ilab++){
    for(Int_t ipta=0; ipta<7; ipta++){
      //fHistTrackEtaPhi[ilab][ipta]=0; // keep out for now
    }  // end of pt-associated loop
  } // end of lab loop
    
  for(Int_t itrackpt=0; itrackpt<9; itrackpt++){
    fHistJetHadbindPhi[itrackpt]=0;
    fHistJetHadbindPhiIN[itrackpt]=0;
    fHistJetHadbindPhiMID[itrackpt]=0;
    fHistJetHadbindPhiOUT[itrackpt]=0;
  } // end of trackpt bin loop

  for(Int_t icent = 0; icent<6; ++icent){
    for(Int_t iptjet = 0; iptjet<5; ++iptjet){
      for(Int_t ieta = 0; ieta<3; ++ieta){
        fHistJetH[icent][iptjet][ieta]=0;
        fHistJetHBias[icent][iptjet][ieta]=0;
        fHistJetHTT[icent][iptjet][ieta]=0;
      } // end of eta loop
    } // end of pt-jet loop
  } // end of centrality loop

  // centrality dependent histo's
  for (Int_t i = 0;i<6;++i){
    fHistJetPt[i]				= 0;
    fHistJetPtBias[i]			= 0;
    fHistJetPtTT[i]				= 0;
    fHistAreavsRawPt[i]			= 0;
    fHistJetPtvsTrackPt[i]      = 0;
    fHistRawJetPtvsTrackPt[i]   = 0;
    fHistTrackPt[i]             = 0;
    fHistEP0[i]                 = 0;
    fHistEP0A[i]                = 0;
    fHistEP0C[i]                = 0;
    fHistEPAvsC[i]              = 0;
    fHistJetPtcorrGlRho[i]		= 0;
    fHistJetPtvsdEP[i]          = 0;
    fHistJetPtvsdEPBias[i]      = 0;
    fHistRhovsdEP[i]            = 0;
    fHistJetEtaPhiPt[i]         = 0;
    fHistJetEtaPhiPtBias[i]     = 0;
    fHistJetPtArea[i]           = 0;
    fHistJetPtAreaBias[i]       = 0;
    fHistJetPtNcon[i]           = 0;
    fHistJetPtNconBias[i]       = 0;
    fHistJetPtNconCh[i]         = 0;
    fHistJetPtNconBiasCh[i]     = 0;
    fHistJetPtNconEm[i]         = 0;
    fHistJetPtNconBiasEm[i]     = 0;
    fHistJetHaddPhiINcent[i]    = 0;
    fHistJetHaddPhiOUTcent[i]   = 0;
    fHistJetHaddPhiMIDcent[i]   = 0;
  }

  SetMakeGeneralHistograms(kTRUE);
  
}

//________________________________________________________________________
AliAnalysisTaskEmcalJetHadEPpid::AliAnalysisTaskEmcalJetHadEPpid(const char *name) :
  AliAnalysisTaskEmcalJet(name,kTRUE),
  fPhimin(-10), fPhimax(10),
  fEtamin(-0.9), fEtamax(0.9),
  fAreacut(0.0), fTrkBias(5), fClusBias(5), fTrkEta(0.9), 
  fJetPtcut(15.0), fJetRad(0.4), fConstituentCut(0.15),
  fesdTrackCuts(0),
  fDoEventMixing(0), fMixingTracks(50000), fNMIXtracks(5000), fNMIXevents(5),
  fTriggerEventType(AliVEvent::kAny), fMixingEventType(AliVEvent::kAny),
  doPlotGlobalRho(0), doVariableBinning(0), dovarbinTHnSparse(0), 
  makeQAhistos(0), makeBIAShistos(0), makeextraCORRhistos(0), makeoldJEThadhistos(0),
  allpidAXIS(0), fcutType("EMCAL"), doPID(0), doPIDtrackBIAS(0),
  doComments(0),
  doFlavourJetAnalysis(0), fJetFlavTag(-99),
  fBeam(kNA),
  fLocalRhoVal(0),
  fTracksName(""), fTracksNameME(""), fJetsName(""),
  event(0),
  isPItpc(0), isKtpc(0), isPtpc(0), // pid TPC
  isPIits(0), isKits(0), isPits(0), // pid ITS
  isPItof(0), isKtof(0), isPtof(0), // pid TOF
  fPoolMgr(0x0),
  fPIDResponse(0x0), fTPCResponse(),
  fESD(0), fAOD(0), fVevent(0),  
  fHistEventQA(0), fHistEventSelectionQA(0),
  fHistCentZvertGA(0), fHistCentZvertJE(0), fHistCentZvertMB(0), fHistCentZvertAny(0),
  fHistTPCdEdX(0), fHistITSsignal(0), //fHistTOFsignal(0),
  fHistRhovsCent(0), fHistNjetvsCent(0), fHistCentrality(0),
  fHistZvtx(0), fHistMult(0),
  fHistJetPhi(0), fHistTrackPhi(0),
  fHistLocalRhoJetpt(0),
  fHistJetHaddPhiIN(0), fHistJetHaddPhiOUT(0), fHistJetHaddPhiMID(0),
  fHistJetHaddPhiBias(0), fHistJetHaddPhiINBias(0), fHistJetHaddPhiOUTBias(0), fHistJetHaddPhiMIDBias(0),
  fHistMEdPHI(0),
  fHistTrackPtallcent(0),
  fHistJetEtaPhi(0), fHistJetHEtaPhi(0),
  fHistSEphieta(0), fHistMEphieta(0),
  fHistJetHaddPHI(0),
  fHistPID(0),
  fhnPID(0x0), fhnMixedEvents(0x0), fhnJH(0x0), fhnCorr(0x0),
  fJetsCont(0), fTracksCont(0), fCaloClustersCont(0),
  fContainerAllJets(0), fContainerPIDJets(1)
{   
  // Default Constructor 
  for(Int_t ilab=0; ilab<4; ilab++){
    for(Int_t ipta=0; ipta<7; ipta++){
      //fHistTrackEtaPhi[ilab][ipta]=0; //keep out for now
    }  // end of pt-associated loop
  } // end of lab loop

  for(Int_t itrackpt=0; itrackpt<9; itrackpt++){
    fHistJetHadbindPhi[itrackpt]=0;
    fHistJetHadbindPhiIN[itrackpt]=0;
    fHistJetHadbindPhiMID[itrackpt]=0;
    fHistJetHadbindPhiOUT[itrackpt]=0;
  } // end of trackpt bin loop
    
  for(Int_t icent = 0; icent<6; ++icent){
    for(Int_t iptjet = 0; iptjet<5; ++iptjet){
      for(Int_t ieta = 0; ieta<3; ++ieta){
        fHistJetH[icent][iptjet][ieta]=0;
        fHistJetHBias[icent][iptjet][ieta]=0;
        fHistJetHTT[icent][iptjet][ieta]=0;
      } // end of eta loop
    } // end of pt-jet loop
  } // end of centrality loop

  // centrality dependent histo's
  for (Int_t i = 0;i<6;++i){
    fHistJetPt[i]				= 0;
    fHistJetPtBias[i]			= 0;
    fHistJetPtTT[i]				= 0;
    fHistAreavsRawPt[i]			= 0;
    fHistJetPtvsTrackPt[i]      = 0;
    fHistRawJetPtvsTrackPt[i]   = 0;
    fHistTrackPt[i]             = 0;
    fHistEP0[i]                 = 0;
    fHistEP0A[i]                = 0;
    fHistEP0C[i]                = 0;
    fHistEPAvsC[i]              = 0;
    fHistJetPtcorrGlRho[i]      = 0;
    fHistJetPtvsdEP[i]          = 0;
    fHistJetPtvsdEPBias[i]      = 0;
    fHistRhovsdEP[i]            = 0;
    fHistJetEtaPhiPt[i]         = 0;
    fHistJetEtaPhiPtBias[i]     = 0;
    fHistJetPtArea[i]           = 0;
    fHistJetPtAreaBias[i]       = 0;
    fHistJetPtNcon[i]           = 0;
    fHistJetPtNconBias[i]       = 0;
    fHistJetPtNconCh[i]         = 0;
    fHistJetPtNconBiasCh[i]     = 0;
    fHistJetPtNconEm[i]         = 0;
    fHistJetPtNconBiasEm[i]     = 0;
    fHistJetHaddPhiINcent[i]    = 0;
    fHistJetHaddPhiOUTcent[i]   = 0;
    fHistJetHaddPhiMIDcent[i]   = 0;
  }

  SetMakeGeneralHistograms(kTRUE);

}

//_______________________________________________________________________
AliAnalysisTaskEmcalJetHadEPpid::~AliAnalysisTaskEmcalJetHadEPpid()
{
  // destructor
  if (fOutput) {
    delete fOutput;
    fOutput = 0;
  }
}

//________________________________________________________________________
void AliAnalysisTaskEmcalJetHadEPpid::UserCreateOutputObjects()
{ // This is called ONCE!
  if (!fCreateHisto) return;
  AliAnalysisTaskEmcalJet::UserCreateOutputObjects();
  OpenFile(1); // do I need the 1?

  // char array for naming histograms
  int nchar = 200;
  char name[nchar];

  // strings for titles
  TString name1;
  TString title1;

  // create histograms that arn't array
  fHistNjetvsCent = new TH2F("NjetvsCent", "NjetvsCent", 100, 0.0, 100.0, 100, 0, 100);
  fHistJetHaddPHI = new TH1F("fHistJetHaddPHI", "Jet-Hadron #Delta#varphi", 128,-0.5*TMath::Pi(),1.5*TMath::Pi());
  fHistJetHaddPhiIN = new TH1F("fHistJetHaddPhiIN","Jet-Hadron #Delta#varphi IN PLANE", 128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
  fHistJetHaddPhiOUT = new TH1F("fHistJetHaddPhiOUT","Jet-Hadron #Delta#varphi OUT PLANE",128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
  fHistJetHaddPhiMID = new TH1F("fHistJetHaddPhiMID","Jet-Hadron #Delta#varphi MIDDLE of PLANE",128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
  fHistLocalRhoJetpt = new TH1F("fHistLocalRhoJetpt","Local Rho corrected Jet p_{T}", 50, -50, 200);

  // Centrality and Zvertex distribution for various triggers - Event Mixing QA
  fHistCentZvertGA = new TH2F("fHistCentZvertGA", "Centrality - Z-vertex distribution for GA trigger", 20, 0, 100, 10, -10, 10);
  fOutput->Add(fHistCentZvertGA);
  fHistCentZvertJE = new TH2F("fHistCentZvertJE", "Centrality - Z-vertex distribution for JE trigger", 20, 0, 100, 10, -10, 10);
  fOutput->Add(fHistCentZvertJE);
  fHistCentZvertMB = new TH2F("fHistCentZvertMB", "Centrality - Z-vertex distribution for MB trigger", 20, 0, 100, 10, -10, 10);
  fOutput->Add(fHistCentZvertMB);
  fHistCentZvertAny = new TH2F("fHistCentZvertAny", "Centrality - Z-vertex distribution for kAny trigger", 20, 0, 100, 10, -10, 10);
  fOutput->Add(fHistCentZvertAny);

  // Event QA histo  
  fHistEventQA = new TH1F("fHistEventQA", "Event Counter at checkpoints in code", 20, 0.5, 20.5);
  SetfHistQAcounterLabels(fHistEventQA); 
  fOutput->Add(fHistEventQA);

  // Event Selection QA histo
  fHistEventSelectionQA = new TH1F("fHistEventSelectionQA", "Trigger Selection Counter", 20, 0.5, 20.5);
  SetfHistEvtSelQALabels(fHistEventSelectionQA);
  fOutput->Add(fHistEventSelectionQA);

  // add to output lists
  fOutput->Add(fHistNjetvsCent);
  fOutput->Add(fHistJetHaddPHI);
  fOutput->Add(fHistJetHaddPhiIN);
  fOutput->Add(fHistJetHaddPhiOUT);
  fOutput->Add(fHistJetHaddPhiMID);
  fOutput->Add(fHistLocalRhoJetpt);

  // create histo's used for general QA
  if (makeQAhistos) {
    fHistTPCdEdX = new TH2F("TPCdEdX", "TPCdEdX", 2000, 0.0, 100.0, 500, 0, 500); 
    fHistITSsignal = new TH2F("ITSsignal", "ITSsignal", 2000, 0.0, 100.0, 500, 0, 500);
    //  fHistTOFsignal = new TH2F("TOFsignal", "TOFsignal", 2000, 0.0, 100.0, 500, 0, 500);
    fHistCentrality = new TH1F("fHistCentrality","centrality",100,0,100);
    fHistZvtx = new TH1F("fHistZvertex","z vertex",60,-30,30);
    fHistJetPhi = new TH1F("fHistJetPhi", "Jet #phi Distribution", 128, -2.0*TMath::Pi(), 2.0*TMath::Pi());
    fHistTrackPhi = new TH1F("fHistTrackPhi", "Track #phi Distribution", 128, -2.0*TMath::Pi(), 2.0*TMath::Pi());  
    fHistRhovsCent = new TH2F("RhovsCent", "RhovsCent", 100, 0.0, 100.0, 500, 0, 500);
    fHistTrackPtallcent = new TH1F("fHistTrackPtallcent", "p_{T} distribution", 1000, 0.0, 100.0);
    fHistJetEtaPhi = new TH2F("fHistJetEtaPhi","Jet #eta-#phi",512,-1.8,1.8,512,-3.2,3.2);
    fHistJetHEtaPhi = new TH2F("fHistJetHEtaPhi","Jet-Hadron #Delta#eta-#Delta#phi", 72, -1.8, 1.8, 72, -1.6, 4.8);
    fHistSEphieta = new TH2F("fHistSEphieta", "Single Event #phi-#eta distribution", 64,-0.5*TMath::Pi(), 1.5*TMath::Pi(), 64,-1.8,1.8);  // was 64 bins
    fHistMEphieta = new TH2F("fHistMEphieta", "Mixed Event #phi-#eta distribution", 64, -0.5*TMath::Pi(), 1.5*TMath::Pi(), 64,-1.8,1.8);  // was 64 bins

    // add to output list
    fOutput->Add(fHistTPCdEdX);
    fOutput->Add(fHistITSsignal);
    //fOutput->Add(fHistTOFsignal);
    fOutput->Add(fHistCentrality);
    fOutput->Add(fHistZvtx);
    fOutput->Add(fHistJetPhi);
    fOutput->Add(fHistTrackPhi);
    //fOutput->Add(fHistTrackEtaPhi);
    fOutput->Add(fHistTrackPtallcent);
    fOutput->Add(fHistJetEtaPhi);
    fOutput->Add(fHistJetHEtaPhi);
    fOutput->Add(fHistSEphieta);
    fOutput->Add(fHistMEphieta);

    //for(Int_t ipta=0; ipta<7; ++ipta){ 
    //  for(Int_t ilab=0; ilab<4; ++ilab){
    //    snprintf(name, nchar, "fHistTrackEtaPhi_%i_%i", ilab,ipta);
    //    fHistTrackEtaPhi[ilab][ipta] = new TH2F(name,name,400,-1,1,640,0.0,2.*TMath::Pi());
    //    fOutput->Add(fHistTrackEtaPhi[ilab][ipta]);
    //  } // end of lab loop
    //} // end of pt-associated loop

    for (Int_t i = 0;i<6;++i){
      name1 = TString(Form("EP0_%i",i));
      title1 = TString(Form("EP VZero cent bin %i",i));
      fHistEP0[i] = new TH1F(name1,title1,144,-TMath::Pi(),TMath::Pi());
      fOutput->Add(fHistEP0[i]);

      name1 = TString(Form("EP0A_%i",i));
      title1 = TString(Form("EP VZero cent bin %i",i));
      fHistEP0A[i] = new TH1F(name1,title1,144,-TMath::Pi(),TMath::Pi());
      fOutput->Add(fHistEP0A[i]);

      name1 = TString(Form("EP0C_%i",i));
      title1 = TString(Form("EP VZero cent bin %i",i));
      fHistEP0C[i] = new TH1F(name1,title1,144,-TMath::Pi(),TMath::Pi());
      fOutput->Add(fHistEP0C[i]);

      name1 = TString(Form("EPAvsC_%i",i));
      title1 = TString(Form("EP VZero cent bin %i",i));
      fHistEPAvsC[i] = new TH2F(name1,title1,144,-TMath::Pi(),TMath::Pi(),144,-TMath::Pi(),TMath::Pi());
      fOutput->Add(fHistEPAvsC[i]);

      name1 = TString(Form("JetPtvsTrackPt_%i",i));
      title1 = TString(Form("Jet p_{T} vs Leading Track p_{T} cent bin %i",i));
      fHistJetPtvsTrackPt[i] = new TH2F(name1,title1,250,-50,200,250,0,50);
      fOutput->Add(fHistJetPtvsTrackPt[i]);

      name1 = TString(Form("RawJetPtvsTrackPt_%i",i));
      title1 = TString(Form("Raw Jet p_{T} vs Leading Track p_{T} cent bin %i",i));
      fHistRawJetPtvsTrackPt[i] = new TH2F(name1,title1,250,-50,200,250,0,50);
      fOutput->Add(fHistRawJetPtvsTrackPt[i]);

      name1 = TString(Form("TrackPt_%i",i));
      title1 = TString(Form("Track p_{T} cent bin %i",i));
      fHistTrackPt[i] = new TH1F(name1,title1,1000,0,100); // up to 200?
      fOutput->Add(fHistTrackPt[i]);   

      name1 = TString(Form("JetPtcorrGLrho_%i",i));
      title1 = TString(Form("Jet p_{T} corrected with Global #rho cent bin %i",i));
      fHistJetPtcorrGlRho[i] = new TH1F(name1,title1,300,-100,200); // up to 200?
      fOutput->Add(fHistJetPtcorrGlRho[i]);  
    
      name1 = TString(Form("JetPtvsdEP_%i",i));
      title1 = TString(Form("Jet p_{T} vs #DeltaEP cent bin %i",i));
      fHistJetPtvsdEP[i] = new TH2F(name1,title1,250,-50,200,288,-2*TMath::Pi(),2*TMath::Pi());
      fOutput->Add(fHistJetPtvsdEP[i]);

      name1 = TString(Form("RhovsdEP_%i",i));
      title1 = TString(Form("#rho vs #DeltaEP cent bin %i",i));
      fHistRhovsdEP[i] = new TH2F(name1,title1,500,0,500,288,-2*TMath::Pi(),2*TMath::Pi());
      fOutput->Add(fHistRhovsdEP[i]);

      name1 = TString(Form("JetEtaPhiPt_%i",i));
      title1 = TString(Form("Jet #eta-#phi p_{T} cent bin %i",i));
      fHistJetEtaPhiPt[i] = new TH3F(name1,title1,250,-50,200,100,-1,1,64,-3.2,3.2);
      fOutput->Add(fHistJetEtaPhiPt[i]);

      name1 = TString(Form("JetPtArea_%i",i));
      title1 = TString(Form("Jet p_{T} Area cent bin %i",i));
      fHistJetPtArea[i] = new TH2F(name1,title1,250,-50,200,100,0,1);
      fOutput->Add(fHistJetPtArea[i]);

      snprintf(name, nchar, "fHistAreavsRawPt_%i",i);
      fHistAreavsRawPt[i] = new TH2F(name,name,100,0,1,200,0,200);
      fOutput->Add(fHistAreavsRawPt[i]);
    } // loop over centrality

  } // QA histo switch

  if (makeBIAShistos) {
    fHistJetHaddPhiBias = new TH1F("fHistJetHaddPhiBias","Jet-Hadron #Delta#varphi with bias",128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
    fHistJetHaddPhiINBias = new TH1F("fHistJetHaddPhiINBias","Jet-Hadron #Delta#varphi IN PLANE with bias", 128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
    fHistJetHaddPhiOUTBias = new TH1F("fHistJetHaddPhiOUTBias","Jet-Hadron #Delta#varphi OUT PLANE with bias",128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
    fHistJetHaddPhiMIDBias = new TH1F("fHistJetHaddPhiMIDBias","Jet-Hadron #Delta#varphi MIDDLE of PLANE with bias",128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
 
    // add to output list
    fOutput->Add(fHistJetHaddPhiBias);
    fOutput->Add(fHistJetHaddPhiINBias);
    fOutput->Add(fHistJetHaddPhiOUTBias);
    fOutput->Add(fHistJetHaddPhiMIDBias);

    for (Int_t i = 0;i<6;++i){
      name1 = TString(Form("JetPtvsdEPBias_%i",i));
      title1 = TString(Form("Bias Jet p_{T} vs #DeltaEP cent bin %i",i));
      fHistJetPtvsdEPBias[i] = new TH2F(name1,title1,250,-50,200,288,-2*TMath::Pi(),2*TMath::Pi());
      fOutput->Add(fHistJetPtvsdEPBias[i]);

      name1 = TString(Form("JetEtaPhiPtBias_%i",i));
      title1 = TString(Form("Jet #eta-#phi p_{T} Bias cent bin %i",i));
      fHistJetEtaPhiPtBias[i] = new TH3F(name1,title1,250,-50,200,100,-1,1,64,-3.2,3.2);
      fOutput->Add(fHistJetEtaPhiPtBias[i]);

      name1 = TString(Form("JetPtAreaBias_%i",i));
      title1 = TString(Form("Jet p_{T} Area Bias cent bin %i",i));
      fHistJetPtAreaBias[i] = new TH2F(name1,title1,250,-50,200,100,0,1);
      fOutput->Add(fHistJetPtAreaBias[i]);
    }  // end of centrality loop
  } // bias histos

  if (makeoldJEThadhistos) {
    for(Int_t icent = 0; icent<6; ++icent){
      snprintf(name, nchar, "fHistJetPtTT_%i",icent);
      fHistJetPtTT[icent] = new TH1F(name,name,200,0,200);
      fOutput->Add(fHistJetPtTT[icent]);

      snprintf(name, nchar, "fHistJetPt_%i",icent);
      fHistJetPt[icent] = new TH1F(name,name,200,0,200);
      fOutput->Add(fHistJetPt[icent]);

      snprintf(name, nchar, "fHistJetPtBias_%i",icent);
      fHistJetPtBias[icent] = new TH1F(name,name,200,0,200);
      fOutput->Add(fHistJetPtBias[icent]);

      for(Int_t iptjet = 0; iptjet<5; ++iptjet){
        for(Int_t ieta = 0; ieta<3; ++ieta){
          snprintf(name, nchar, "fHistJetH_%i_%i_%i",icent,iptjet,ieta);
          fHistJetH[icent][iptjet][ieta]=new TH2F(name,name,64,-0.5*TMath::Pi(),1.5*TMath::Pi(),300,0,30);
          fOutput->Add(fHistJetH[icent][iptjet][ieta]);

          snprintf(name, nchar, "fHistJetHBias_%i_%i_%i",icent,iptjet,ieta);
          fHistJetHBias[icent][iptjet][ieta]=new TH2F(name,name,64,-0.5*TMath::Pi(),1.5*TMath::Pi(),300,0,30);
          fOutput->Add(fHistJetHBias[icent][iptjet][ieta]);

          snprintf(name, nchar, "fHistJetHTT_%i_%i_%i",icent,iptjet,ieta);
          fHistJetHTT[icent][iptjet][ieta]=new TH2F(name,name,64,-0.5*TMath::Pi(),1.5*TMath::Pi(),300,0,30);
          fOutput->Add(fHistJetHTT[icent][iptjet][ieta]);
        } // end of eta loop
      } // end of pt-jet loop
    } // end of centrality loop
  } // make JetHadhisto old

  if (makeextraCORRhistos) {
    for(Int_t itrackpt=0; itrackpt<9; itrackpt++){
      snprintf(name, nchar, "fHistJetHadbindPhi_%i",itrackpt);
      fHistJetHadbindPhi[itrackpt] = new TH1F(name,name,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHadbindPhi[itrackpt]);

      snprintf(name, nchar, "fHistJetHadbindPhiIN_%i",itrackpt);
      fHistJetHadbindPhiIN[itrackpt] = new TH1F(name,name,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHadbindPhiIN[itrackpt]);

      snprintf(name, nchar, "fHistJetHadbindPhiMID_%i",itrackpt);
      fHistJetHadbindPhiMID[itrackpt] = new TH1F(name,name,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHadbindPhiMID[itrackpt]);

      snprintf(name, nchar, "fHistJetHadbindPhiOUT_%i",itrackpt);
      fHistJetHadbindPhiOUT[itrackpt] = new TH1F(name,name,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHadbindPhiOUT[itrackpt]);
    } // end of trackpt bin loop

    for (Int_t i = 0;i<6;++i){
      name1 = TString(Form("JetHaddPhiINcent_%i",i));
      title1 = TString(Form("Jet Hadron #Delta#varphi Distribution IN PLANE cent bin %i",i));
      fHistJetHaddPhiINcent[i] = new TH1F(name1,title1,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHaddPhiINcent[i]);
  
      name1 = TString(Form("JetHaddPhiOUTcent_%i",i));
      title1 = TString(Form("Jet Hadron #Delta#varphi Distribution OUT PLANE cent bin %i",i));
      fHistJetHaddPhiOUTcent[i] = new TH1F(name1,title1,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHaddPhiOUTcent[i]);

      name1 = TString(Form("JetHaddPhiMIDcent_%i",i));
      title1 = TString(Form("Jet Hadron #Delta#varphi Distribution MIDDLE of PLANE cent bin %i",i));
      fHistJetHaddPhiMIDcent[i] = new TH1F(name1,title1,128,-0.5*TMath::Pi(), 1.5*TMath::Pi());
      fOutput->Add(fHistJetHaddPhiMIDcent[i]);

      name1 = TString(Form("JetPtNcon_%i",i));
      title1 = TString(Form("Jet p_{T} Ncon cent bin %i",i));
      fHistJetPtNcon[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNcon[i]);

      name1 = TString(Form("JetPtNconBias_%i",i));
      title1 = TString(Form("Jet p_{T} NconBias cent bin %i",i));
      fHistJetPtNconBias[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNconBias[i]);

      name1 = TString(Form("JetPtNconCh_%i",i));
      title1 = TString(Form("Jet p_{T} NconCh cent bin %i",i));
      fHistJetPtNconCh[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNconCh[i]);

      name1 = TString(Form("JetPtNconBiasCh_%i",i));
      title1 = TString(Form("Jet p_{T} NconBiasCh cent bin %i",i));
      fHistJetPtNconBiasCh[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNconBiasCh[i]);

      name1 = TString(Form("JetPtNconEm_%i",i));
      title1 = TString(Form("Jet p_{T} NconEm cent bin %i",i));
      fHistJetPtNconEm[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNconEm[i]);

      name1 = TString(Form("JetPtNconBiasEm_%i",i));
      title1 = TString(Form("Jet p_{T} NconBiasEm cent bin %i",i));
      fHistJetPtNconBiasEm[i] = new TH2F(name1,title1,250,-50,200,100,0,2000);
      fOutput->Add(fHistJetPtNconBiasEm[i]);
    } // extra Correlations histos switch
  }

  // variable binned pt for THnSparse's
  Double_t xlowjetPT[] = {15, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 45, 50, 55, 60, 65, 70, 75, 80, 100, 150, 200, 300};
  Double_t xlowtrPT[] = {0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.6,0.7,0.8,0.9,1.0,1.2,1.4,1.6,1.8,2.0,2.2,2.4,2.6,2.8,3.0,3.2,3.4,3.6,3.8,4.0,4.2,4.4,4.6,4.8,5.0,5.5,6.0,6.5,7.0,7.5,8.0,9.0,10.0,12.0,14.0,16.0,18.0,20.0,25.0,30.0,40.0,50.0,75.0};

  // tracks: 51, jets: 26
  // number of bins you tell histogram should be (# in array - 1) because the last bin
  // is the right-most edge of the histogram 
  // i.e. this is for PT and there are 57 numbers (bins) thus we have 56 bins in our histo
  Int_t nbinsjetPT = sizeof(xlowjetPT)/sizeof(Double_t) - 1;
  Int_t nbinstrPT = sizeof(xlowtrPT)/sizeof(Double_t) - 1;
  
  // set up jet-hadron sparse
  UInt_t bitcodeMESE = 0; // bit coded, see GetDimParams() below
  UInt_t bitcodePID = 0;  // bit coded, see GetDimParamsPID() below
  UInt_t bitcodeCorr = 0; // bit coded, see GetDimparamsCorr() below
  bitcodeMESE = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6; // | 1<<7 | 1<<8 | 1<<9;
  if(fDoEventMixing) {
    fhnJH = NewTHnSparseF("fhnJH", bitcodeMESE);
  
    if(dovarbinTHnSparse){
      fhnJH->GetAxis(1)->Set(nbinsjetPT, xlowjetPT);
      fhnJH->GetAxis(2)->Set(nbinstrPT, xlowtrPT);
    }
	
    fOutput->Add(fhnJH);
  }

  bitcodeCorr = 1<<0 | 1<<1 | 1<<2 | 1<<3; // | 1<<4 | 1<<5;
  fhnCorr = NewTHnSparseFCorr("fhnCorr", bitcodeCorr);
  if(dovarbinTHnSparse) fhnCorr->GetAxis(1)->Set(nbinsjetPT, xlowjetPT);
  fOutput->Add(fhnCorr);  

  /*
    Double_t centralityBins[nCentralityBins+1];
    for(Int_t ic=0; ic<nCentralityBins+1; ic++){
      if(ic==nCentralityBins) centralityBins[ic]=500;
      else centralityBins[ic]=10.0*ic; 
    }
  */

  // set up centrality bins for mixed events
  // for pp we need mult bins for event mixing. Create binning here, to also make a histogram from it
  Int_t nCentralityBinspp = 8;
  //Double_t centralityBinspp[nCentralityBinspp+1];
  Double_t centralityBinspp[9] = {0.0, 4., 9, 15, 25, 35, 55, 100.0, 500.0};  

  // Setup for Pb-Pb collisions
  Int_t nCentralityBinsPbPb = 10; //100;
  Double_t centralityBinsPbPb[nCentralityBinsPbPb+1];
  for(Int_t ic=0; ic<nCentralityBinsPbPb; ic++){
      centralityBinsPbPb[ic]=10.0*ic; //1.0*ic;
  }

  if(fBeam == 0) fHistMult = new TH1F("fHistMult","multiplicity",nCentralityBinspp,centralityBinspp);
  if(fBeam == 1) fHistMult = new TH1F("fHistMult","multiplicity",nCentralityBinsPbPb,centralityBinsPbPb);
//  fOutput->Add(fHistMult);

  // Event Mixing
  Int_t trackDepth = fMixingTracks;
  Int_t poolsize   = 1000;  // Maximum number of events, ignored in the present implemented of AliEventPoolManager
  Int_t nZvtxBins  = 5+1+5;
  Double_t vertexBins[] = { -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10};
  Double_t* zvtxbin = vertexBins;
  if(fBeam == 0) fPoolMgr = new AliEventPoolManager(poolsize, trackDepth, nCentralityBinspp, centralityBinspp, nZvtxBins, zvtxbin);
  if(fBeam == 1) fPoolMgr = new AliEventPoolManager(poolsize, trackDepth, nCentralityBinsPbPb, centralityBinsPbPb, nZvtxBins, zvtxbin);

  // set up event mixing sparse
  if(fDoEventMixing){
    bitcodeMESE = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6; // | 1<<7 | 1<<8 | 1<<9;
    fhnMixedEvents = NewTHnSparseF("fhnMixedEvents", bitcodeMESE);  

    // set up some variable binning of the sparse
    if(dovarbinTHnSparse){
     fhnMixedEvents->GetAxis(1)->Set(nbinsjetPT, xlowjetPT);
     fhnMixedEvents->GetAxis(2)->Set(nbinstrPT, xlowtrPT);
    }

    fOutput->Add(fhnMixedEvents);
  } // end of do-eventmixing

  // set up PID sparse
  if(doPID){
    // ****************************** PID *****************************************************
    // set up PID handler
    AliAnalysisManager *man=AliAnalysisManager::GetAnalysisManager();
    AliInputEventHandler* inputHandler = (AliInputEventHandler*) (man->GetInputEventHandler());
    if(!inputHandler) {
        AliFatal("Input handler needed");
		return;
    }

    // PID response object
    fPIDResponse = inputHandler->GetPIDResponse();
    if (!fPIDResponse) {
        AliError("PIDResponse object was not created");
	    return;
    }
    // *****************************************************************************************

    // PID counter
    fHistPID = new TH1F("fHistPID","PID Counter", 15, 0.5, 15.5);
    SetfHistPIDcounterLabels(fHistPID);
    fOutput->Add(fHistPID);

    if(allpidAXIS) {
      bitcodePID = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 |
              1<<10 | 1<<11 | 1<<12 | 1<<13 | 1<<14 | 1<<15 | 1<<16 | 1<<17 | 1<<18 | 1<<19 |
              1<<20;
      fhnPID = NewTHnSparseFPID("fhnPID", bitcodePID);
    } else {
	  bitcodePID = 1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<8 | 1<<9 |
              1<<10 | 1<<11 | 1<<12 | 1<<13;
      fhnPID = NewTHnSparseFPID("fhnPID", bitcodePID);
	}

    // set some variable binning of sparse
    if(dovarbinTHnSparse){
     fhnPID->GetAxis(1)->Set(nbinstrPT, xlowtrPT);
     fhnPID->GetAxis(8)->Set(nbinsjetPT, xlowjetPT);
    }

    fOutput->Add(fhnPID);
  } // end of do-PID

  // =========== Switch on Sumw2 for all histos ===========
  for (Int_t i=0; i<fOutput->GetEntries(); ++i) {
    TH1 *h1 = dynamic_cast<TH1*>(fOutput->At(i));
    if (h1){
      h1->Sumw2();
      continue;
    }
    TH2 *h2 = dynamic_cast<TH2*>(fOutput->At(i));
    if (h2){
      h2->Sumw2();
      continue;
    }
    TH3 *h3 = dynamic_cast<TH3*>(fOutput->At(i));
    if (h3){
      h3->Sumw2();
      continue;
    }
    THnSparse *hn = dynamic_cast<THnSparse*>(fOutput->At(i));
    if(hn)hn->Sumw2();
  }

  PostData(1, fOutput);
}

//_________________________________________________________________________
void AliAnalysisTaskEmcalJetHadEPpid::ExecOnce()
{
  AliAnalysisTaskEmcalJet::ExecOnce();

  if (fJetsCont && fJetsCont->GetArray() == 0) fJetsCont = 0;
  if (fTracksCont && fTracksCont->GetArray() == 0) fTracksCont = 0;
  if (fCaloClustersCont && fCaloClustersCont->GetArray() == 0) fCaloClustersCont = 0;
}

//_________________________________________________________________________
Bool_t AliAnalysisTaskEmcalJetHadEPpid::Run()
{ // Main loop called for each event
  // TEST TEST TEST TEST for OBJECTS!

  fHistEventQA->Fill(1); // All Events that get entered

  // check and fill a Event Selection QA histogram for different trigger selections
  UInt_t trig = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
  if(trig == 0) fHistEventSelectionQA->Fill(1);
  if(trig & AliVEvent::kAny) fHistEventSelectionQA->Fill(2);
  if(trig & AliVEvent::kAnyINT) fHistEventSelectionQA->Fill(3);
  if(trig & AliVEvent::kMB) fHistEventSelectionQA->Fill(4);
  if(trig & AliVEvent::kINT7) fHistEventSelectionQA->Fill(5);
  if(trig & AliVEvent::kEMC1) fHistEventSelectionQA->Fill(6);
  if(trig & AliVEvent::kEMC7) fHistEventSelectionQA->Fill(7);
  if(trig & AliVEvent::kEMC8) fHistEventSelectionQA->Fill(8);
  if(trig & AliVEvent::kEMCEJE) fHistEventSelectionQA->Fill(9);
  if(trig & AliVEvent::kEMCEGA) fHistEventSelectionQA->Fill(10);
  if(trig & AliVEvent::kCentral) fHistEventSelectionQA->Fill(11);
  if(trig & AliVEvent::kSemiCentral) fHistEventSelectionQA->Fill(12);
  if(trig & AliVEvent::kINT8) fHistEventSelectionQA->Fill(13);

  if(trig & (AliVEvent::kEMCEJE | AliVEvent::kMB)) fHistEventSelectionQA->Fill(14);
  if(trig & (AliVEvent::kEMCEGA | AliVEvent::kMB)) fHistEventSelectionQA->Fill(15);
  if(trig & (AliVEvent::kAnyINT | AliVEvent::kMB)) fHistEventSelectionQA->Fill(16);

  //if(trig & (AliVEvent::kEMCEJE && AliVEvent::kMB)) fHistEventSelectionQA->Fill(17);
  //if(trig & (AliVEvent::kEMCEGA && AliVEvent::kMB)) fHistEventSelectionQA->Fill(18);
  //if(trig & (AliVEvent::kAnyINT && AliVEvent::kMB)) fHistEventSelectionQA->Fill(19);

  // see if we are running on PbPb and try and get LocalRho object
  if(GetBeamType() == 1) {
    if(!fLocalRho){
      AliError(Form("Couldn't get fLocalRho object, try to get it from Event based on name\n"));
      fLocalRho = GetLocalRhoFromEvent(fLocalRhoName);
      if(!fLocalRho) return kTRUE;
    }
  } // check for LocalRho object if PbPb data

  if(!fTracks){
    AliError(Form("No fTracks object!!\n"));
    return kTRUE;
  }
  if(!fJets){
    AliError(Form("No fJets object!!\n"));
    return kTRUE;
  }

  fHistEventQA->Fill(2); // events after object check

  // what kind of event do we have: AOD or ESD?
  Bool_t useAOD; 
  if (dynamic_cast<AliAODEvent*>(InputEvent())) useAOD = kTRUE;
  else useAOD = kFALSE;

  // if we have ESD event, set up ESD object
  if(!useAOD){
    fESD = dynamic_cast<AliESDEvent*>(InputEvent());
    if (!fESD) {
      AliError(Form("ERROR: fESD not available\n"));
      return kTRUE;
    }
  }

  // if we have AOD event, set up AOD object
  if(useAOD){
    fAOD = dynamic_cast<AliAODEvent*>(InputEvent());
    if(!fAOD) {
      AliError(Form("ERROR: fAOD not available\n"));
      return kTRUE;
    }
  }

  fHistEventQA->Fill(3); // events after Aod/esd check

  // get centrality
  Int_t centbin = GetCentBin(fCent);
  if (makeQAhistos) fHistCentrality->Fill(fCent); // won't be filled in pp collision (Keep this in mind!)

  // if we are on PbPb data do cut on centrality > 90%, else by default DON'T
  if (GetBeamType() == 1) {
    // apply cut to event on Centrality > 90%
    if(fCent>90) return kTRUE;
  }

  // BEAM TYPE enumerator: kNA = -1, kpp = 0, kAA = 1, kpA = 2
  // for pp analyses we will just use the first centrality bin
  if(GetBeamType() == 0) if (centbin == -1) centbin = 0;
  if(GetBeamType() == 1) if (centbin == -1) return kTRUE;

  fHistEventQA->Fill(4);  // events after centrality check

  // get vertex information
  Double_t fvertex[3]={0,0,0};
  InputEvent()->GetPrimaryVertex()->GetXYZ(fvertex);
  Double_t zVtx=fvertex[2];
  if (makeQAhistos) fHistZvtx->Fill(zVtx);

  // get z-vertex bin
  //Int_t zVbin = GetzVertexBin(zVtx);

  // apply zVtx cut
  if(fabs(zVtx)>10.0) return kTRUE;

  fHistEventQA->Fill(5); // events after zvertex check

  // create pointer to list of input event
  TList *list = InputEvent()->GetList();
  if(!list) {
    AliError(Form("ERROR: list not attached\n"));
    return kTRUE;
  }

  fHistEventQA->Fill(6); // events after list check

  // initialize TClonesArray pointers to jets and tracks
  TClonesArray *jets = 0;
  TClonesArray *tracks = 0; 
  TClonesArray *tracksME = 0;

  // get Tracks object
  tracks = dynamic_cast<TClonesArray*>(list->FindObject(fTracks));
  if (!tracks) {
    AliError(Form("Pointer to tracks %s == 0", fTracks->GetName()));
    return kTRUE;
  } // verify existence of tracks

  // get ME Tracks object
  tracksME = dynamic_cast<TClonesArray*>(list->FindObject(fTracksNameME));
  if (!tracksME) {
    AliError(Form("Pointer to tracks %s == 0", fTracksNameME.Data()));
    return kTRUE;
  } // verify existence of tracks

  // get Jets object
  jets = dynamic_cast<TClonesArray*>(list->FindObject(fJets));
  if(!jets){
    AliError(Form("Pointer to jets %s == 0", fJets->GetName()));
    return kTRUE;
  } // verify existence of jets

  fHistEventQA->Fill(7);  // events after track/jet pointer check

  // get number of jets and tracks
  const Int_t Njets = jets->GetEntries(); 
  const Int_t Ntracks = tracks->GetEntries();
  if(Ntracks<1)   return kTRUE;
  if(Njets<1)	  return kTRUE;

  fHistEventQA->Fill(8); // events after #track and jets < 1 check

  if (makeQAhistos) fHistMult->Fill(Ntracks);  // fill multiplicity distribution

  // initialize track parameters
  Int_t iTT=-1;
  Double_t ptmax=-10;
  Int_t NtrackAcc = 0;

  fVevent = dynamic_cast<AliVEvent*>(InputEvent());
  if (!fVevent) {
    printf("ERROR: fVEvent not available\n");
    return kTRUE;
  }

  // fill event mixing QA
  if(trig & AliVEvent::kEMCEGA) fHistCentZvertGA->Fill(fCent, zVtx);
  if(trig & AliVEvent::kEMCEJE) fHistCentZvertJE->Fill(fCent, zVtx);
  if(trig & AliVEvent::kMB) fHistCentZvertMB->Fill(fCent, zVtx);
  if(trig & AliVEvent::kAny) fHistCentZvertAny->Fill(fCent, zVtx);

  // loop over tracks - to get hardest track (highest pt)
  for (Int_t iTracks = 0; iTracks < Ntracks; iTracks++){    
    AliVTrack *track = dynamic_cast<AliVTrack*>(tracks->At(iTracks));
    if(!track){
      AliError(Form("Couldn't get AliVtrack %d\n", iTracks));    
      continue;
    }

    // track cuts
    if(TMath::Abs(track->Eta())>0.9) continue;
    if(track->Pt()<0.15) continue;
    //iCount++;
    NtrackAcc++;

    if(track->Pt()>ptmax){
      ptmax=track->Pt();             // max pT track
      iTT=iTracks;                   // trigger tracks
    } // check if Pt>maxpt

    if (makeQAhistos) fHistTrackPhi->Fill(track->Phi()); 
    if (makeQAhistos) fHistTrackPt[centbin]->Fill(track->Pt());
    if (makeQAhistos) fHistTrackPtallcent->Fill(track->Pt());
  } // end of loop over tracks

  // get rho from event and fill relative histo's
  fRho = GetRhoFromEvent(fRhoName);
  fRhoVal = fRho->GetVal();

  if (makeQAhistos) {
    fHistRhovsdEP[centbin]->Fill(fRhoVal,fEPV0); // Global Rho vs delta Event Plane angle
    fHistRhovsCent->Fill(fCent,fRhoVal);        // Global Rho vs Centrality
    fHistEP0[centbin]->Fill(fEPV0);
    fHistEP0A[centbin]->Fill(fEPV0A);
    fHistEP0C[centbin]->Fill(fEPV0C);
    fHistEPAvsC[centbin]->Fill(fEPV0A,fEPV0C);
  }

  // initialize jet parameters
  Int_t ijethi=-1;
  Double_t highestjetpt=0.0;
  Int_t passedTTcut=0;
  Int_t NjetAcc = 0;
  Double_t leadhadronPT = 0;

  // loop over jets in an event - to find highest jet pT and apply some cuts
  for (Int_t ijet = 0; ijet < Njets; ijet++){
    // get our jets
    AliEmcalJet *jet = static_cast<AliEmcalJet*>(jets->At(ijet));
    if (!jet) continue;

    // apply jet cuts
    if ((jet->Phi()<fPhimin)||(jet->Phi()>fPhimax)) continue;
    if ((jet->Eta()<fEtamin)||(jet->Eta()>fEtamax)) continue;
    if (makeQAhistos) fHistAreavsRawPt[centbin]->Fill(jet->Pt(),jet->Area());
    if(!AcceptMyJet(jet)) continue;

    NjetAcc++;                     // # of accepted jets

    // if FlavourJetAnalysis, get desired FlavTag and check against Jet
    if(doFlavourJetAnalysis) { if(!AcceptFlavourJet(jet, fJetFlavTag)) continue;}

    // use this to get total # of jets passing cuts in events!!!!!!!!
    if (makeQAhistos) fHistJetPhi->Fill(jet->Phi()); // Jet Phi histogram (filled)

    // get highest Pt jet in event
    if(highestjetpt<jet->Pt()){
      ijethi=ijet;
      highestjetpt=jet->Pt();
    }
  } // end of looping over jets

  // accepted jets
  fHistNjetvsCent->Fill(fCent,NjetAcc);
  Int_t NJETAcc = 0;
  fHistEventQA->Fill(9); // events after track/jet loop to get highest pt

  // loop over jets in event and make appropriate cuts
  for (Int_t ijet = 0; ijet < Njets; ++ijet) {
     AliEmcalJet *jet = static_cast<AliEmcalJet*>(fJets->At(ijet));
     if (!jet) continue;

     // check our jet is in our selected trigger event
     if (!(trig & fTriggerEventType)) continue;

	 // (should probably be higher..., but makes a cut on jet pT)
     if (jet->Pt()<0.1) continue;
     // do we accept jet? apply jet cuts
     if (!AcceptMyJet(jet)) continue;

     // if FlavourJetAnalysis, get desired FlavTag and check against Jet
     if(doFlavourJetAnalysis) { if(!AcceptFlavourJet(jet, fJetFlavTag)) continue;}

     fHistEventQA->Fill(10); // accepted jets

     // check on lead jet
     Double_t leadjet=0;
     if (ijet==ijethi) leadjet=1;

     // check on leading hadron pt
     if (ijet==ijethi) leadhadronPT = GetLeadingHadronPt(jet);

     // initialize and calculate various parameters: pt, eta, phi, rho, etc...
     Double_t jetphi = jet->Phi();      // phi of jet
     NJETAcc++;   // # accepted jets
     Double_t jeteta = jet->Eta();     // ETA of jet
     Double_t jetPt = -500; 
     Double_t jetPtGlobal = -500; 
     Double_t jetPtLocal = -500;            // initialize corr jet pt
     if(GetBeamType() == 1) {
       fLocalRhoVal = fLocalRho->GetLocalVal(jetphi, fJetRad); //GetJetRadius(0)); // get local rho value
       jetPtLocal = jet->Pt()-jet->Area()*fLocalRhoVal; // corrected pT of jet using Rho modulated for V2 and V3
     }
     jetPt = jet->Pt();
     jetPtGlobal = jet->Pt()-jet->Area()*fRhoVal;  // corrected pT of jet from rho value
     Double_t dEP = -500;                    // initialize angle between jet and event plane
     dEP = RelativeEPJET(jetphi,fEPV0); // angle betweeen jet and event plane

     // make histo's
     if(makeQAhistos) fHistJetPtvsTrackPt[centbin]->Fill(jetPt,jet->MaxTrackPt());
     if(makeQAhistos) fHistRawJetPtvsTrackPt[centbin]->Fill(jetPt,jet->MaxTrackPt());
     if(makeQAhistos) fHistJetPtcorrGlRho[centbin]->Fill(jetPtGlobal);
     if(makeQAhistos) fHistJetPtvsdEP[centbin]->Fill(jetPt, dEP);
     if(makeQAhistos) fHistJetEtaPhiPt[centbin]->Fill(jetPt,jet->Eta(),jet->Phi());
     if(makeQAhistos) fHistJetPtArea[centbin]->Fill(jetPt,jet->Area());
     if(makeQAhistos) fHistJetEtaPhi->Fill(jet->Eta(),jet->Phi());     // fill jet eta-phi distribution histo
     if(makeextraCORRhistos) fHistJetPtNcon[centbin]->Fill(jetPt,jet->GetNumberOfConstituents());
     if(makeextraCORRhistos) fHistJetPtNconCh[centbin]->Fill(jetPt,jet->GetNumberOfTracks());
     if(makeextraCORRhistos) fHistJetPtNconEm[centbin]->Fill(jetPt,jet->GetNumberOfClusters());
     if (makeoldJEThadhistos) fHistJetPt[centbin]->Fill(jet->Pt());  // fill #jets vs pT histo
     //fHistDeltaPtvsArea->Fill(jetPt,jet->Area());

     // make histo's with BIAS applied
     if (jet->MaxTrackPt()>fTrkBias){    
       if(makeBIAShistos) fHistJetPtvsdEPBias[centbin]->Fill(jetPt, dEP);
       if(makeBIAShistos) fHistJetEtaPhiPtBias[centbin]->Fill(jetPt,jet->Eta(),jet->Phi());
       if(makeextraCORRhistos) fHistJetPtAreaBias[centbin]->Fill(jetPt,jet->Area());
       if(makeextraCORRhistos) fHistJetPtNconBias[centbin]->Fill(jetPt,jet->GetNumberOfConstituents());
       if(makeextraCORRhistos) fHistJetPtNconBiasCh[centbin]->Fill(jetPt,jet->GetNumberOfTracks());
       if(makeextraCORRhistos) fHistJetPtNconBiasEm[centbin]->Fill(jetPt,jet->GetNumberOfClusters());
     }

    //if(leadjet && centbin==0){
    //  if(makeextraCORRhistos)	fHistJetPt[centbin+1]->Fill(jet->Pt());
    //}
    if ((jet->MaxTrackPt()>fTrkBias) || (jet->MaxClusterPt()>fClusBias)){
      if (makeoldJEThadhistos){ 
        fHistJetPtBias[centbin]->Fill(jet->Pt());
        //if(leadjet && centbin==0) fHistJetPtBias[centbin+1]->Fill(jet->Pt());
      }
    }  // end of MaxTrackPt>ftrkBias or maxclusterPt>fclusBias

    // do we have trigger tracks
    if(iTT>0){
      AliVTrack* TT = static_cast<AliVTrack*>(tracks->At(iTT));
      if(TMath::Abs(jet->Phi()-TT->Phi()-TMath::Pi())<0.6) passedTTcut=1;
      else passedTTcut=0;
    } // end of check on iTT > 0

    if(passedTTcut) {  
      if (makeoldJEThadhistos) fHistJetPtTT[centbin]->Fill(jet->Pt());
    }

    // cut on HIGHEST jet pt in event (15 GeV default)
    //if (highestjetpt>fJetPtcut) {
    if (jet->Pt() > fJetPtcut) {
      fHistEventQA->Fill(11); // jets meeting pt threshold

      // does our max track or cluster pass the bias?
      if ((jet->MaxTrackPt()>fTrkBias) || (jet->MaxClusterPt()>fClusBias)){
        // set up and fill Jet-Hadron Correction THnSparse
        Double_t CorrEntries[4] = {fCent, jet->Pt(), dEP, zVtx};
        fhnCorr->Fill(CorrEntries);    // fill Sparse Histo with Correction entries

        if(GetBeamType() == 1) fHistLocalRhoJetpt->Fill(jetPtLocal);
      }

      // loop over all track for an event containing a jet with a pT>fJetPtCut  (15)GeV
      for (Int_t iTracks = 0; iTracks < Ntracks; ++iTracks) {
        AliVTrack *track = dynamic_cast<AliVTrack*>(tracks->At(iTracks));
        if (!track) {
          AliError(Form("Couldn't get AliVtrack %d\n", iTracks));
          continue;
        } 

    	// apply track cuts
        if(TMath::Abs(track->Eta())>fTrkEta) continue;
        if (track->Pt()<0.15) continue;

        fHistEventQA->Fill(12); // accepted tracks in events from trigger jets

        // calculate and get some track parameters
		Double_t trCharge = -99;
        trCharge = track->Charge();
        Double_t tracketa=track->Eta();   // eta of track
        Double_t deta=tracketa-jeteta;    // dETA between track and jet
        //Double_t dR=sqrt(deta*deta+dphijh*dphijh);     // difference of R between jet and hadron track		

        Int_t ieta = -1;       // initialize deta bin
        Int_t iptjet = -1;     // initialize jet pT bin
        if (makeoldJEThadhistos)  {
  		  ieta=GetEtaBin(deta);             // bin of eta
	      if(ieta<0) continue;              // double check we don't have a negative array index
          iptjet=GetpTjetBin(jet->Pt());    // bin of jet pt
   	      if(iptjet<0) continue; 			  // double check we don't have a negative array index
		}

        // dPHI between jet and hadron
        Double_t dphijh = RelativePhi(jet->Phi(), track->Phi()); // angle between jet and hadron

        // fill some jet-hadron histo's
        if (makeoldJEThadhistos) fHistJetH[centbin][iptjet][ieta]->Fill(dphijh,track->Pt());  // fill jet-hadron dPHI--track pT distribution
        if(makeQAhistos) fHistJetHEtaPhi->Fill(deta,dphijh);                          // fill jet-hadron  eta--phi distribution
	    fHistJetHaddPHI->Fill(dphijh);
        if(passedTTcut){
          if (makeoldJEThadhistos) fHistJetHTT[centbin][iptjet][ieta]->Fill(dphijh,track->Pt());
        }

        // does our max track or cluster pass the bias?
        if ((jet->MaxTrackPt()>fTrkBias) || (jet->MaxClusterPt()>fClusBias)){
          // set up and fill Jet-Hadron THnSparse
          Double_t triggerEntries[9] = {fCent, jet->Pt(), track->Pt(), deta, dphijh, dEP, zVtx, trCharge, leadjet};
          if(fDoEventMixing) fhnJH->Fill(triggerEntries);    // fill Sparse Histo with trigger entries

          // fill histo's
          if(makeQAhistos) fHistSEphieta->Fill(dphijh, deta); // single event distribution
          if (makeoldJEThadhistos) fHistJetHBias[centbin][iptjet][ieta]->Fill(dphijh,track->Pt());

          if (makeBIAShistos) {
            fHistJetHaddPhiBias->Fill(dphijh);
	
            // in plane and out of plane histo's
            if( dEP>0 && dEP<=(TMath::Pi()/6) ){
              // we are IN plane
              fHistJetHaddPhiINBias->Fill(dphijh);
            }else if( dEP>(TMath::Pi()/3) && dEP<=(TMath::Pi()/2) ){
              // we are OUT of PLANE
              fHistJetHaddPhiOUTBias->Fill(dphijh);
            }else if( dEP>(TMath::Pi()/6) && dEP<=(TMath::Pi()/3) ){
              // we are in middle of plane
              fHistJetHaddPhiMIDBias->Fill(dphijh);
            }
		  } // BIAS histos switch
        } // end of check maxtrackpt>ftrackbias or maxclusterpt>fclustbias

        // **************************************************************************************************************
        // *********************************** PID **********************************************************************
        // **************************************************************************************************************
        if(doPIDtrackBIAS){
    	  //if(ptmax < fTrkBias) continue;    // force PID to happen when max track pt > 5.0 GeV
          if(leadhadronPT < fTrkBias) continue; // force PID to happen when lead hadron pt > 5.0 GeV
        }

        // some variables for PID
        Double_t pt = -999; 
        Double_t dEdx = -999;
        Double_t ITSsig = -999;
        Double_t TOFsig = -999;
        Double_t charge = -999;

        // nSigma of particles in TPC, TOF, and ITS
        Double_t nSigmaPion_TPC, nSigmaProton_TPC, nSigmaKaon_TPC;
        Double_t nSigmaPion_TOF, nSigmaProton_TOF, nSigmaKaon_TOF;
        Double_t nSigmaPion_ITS, nSigmaProton_ITS, nSigmaKaon_ITS;

        if(doPID){
          // get parameters of track
          charge = track->Charge();    // charge of track
          pt     = track->Pt();        // pT of track

          // extra attempt 
          AliVEvent *vevent=InputEvent();
          if (!vevent||!fPIDResponse) return kTRUE; // just return, maybe put at beginning

          fHistEventQA->Fill(13); // check for AliVEvent and fPIDresponse objects

          // get detector signals
          dEdx = track->GetTPCsignal();
          ITSsig = track->GetITSsignal();
          TOFsig = track->GetTOFsignal();

          // TPC nSigma's
          nSigmaPion_TPC = fPIDResponse->NumberOfSigmasTPC(track,AliPID::kPion);
          nSigmaKaon_TPC = fPIDResponse->NumberOfSigmasTPC(track,AliPID::kKaon);
          nSigmaProton_TPC = fPIDResponse->NumberOfSigmasTPC(track,AliPID::kProton);

          // TOF nSigma's
          nSigmaPion_TOF = fPIDResponse->NumberOfSigmasTOF(track,AliPID::kPion);
          nSigmaKaon_TOF = fPIDResponse->NumberOfSigmasTOF(track,AliPID::kKaon);
          nSigmaProton_TOF = fPIDResponse->NumberOfSigmasTOF(track,AliPID::kProton);

          // ITS nSigma's
          nSigmaPion_ITS = fPIDResponse->NumberOfSigmasITS(track,AliPID::kPion);
          nSigmaKaon_ITS = fPIDResponse->NumberOfSigmasITS(track,AliPID::kKaon);
          nSigmaProton_ITS = fPIDResponse->NumberOfSigmasITS(track,AliPID::kProton);

          // fill detector signal histograms
          if (makeQAhistos) fHistTPCdEdX->Fill(pt, dEdx);
          if (makeQAhistos) fHistITSsignal->Fill(pt, ITSsig);
          //if (makeQAhistos) fHistTOFsignal->Fill(pt, TOFsig);

          // Tests to PID pions, kaons, and protons,          (default is undentified tracks)
          Double_t nPIDtpc = 0;
          Double_t nPIDits = 0; 
          Double_t nPIDtof = 0;
          Double_t nPID = -99;

          // check track has pT < 0.900 GeV  - use TPC pid
          if (pt<0.900 && dEdx>0) {
	        nPIDtpc = 4;
            nPID = 1;

            // PION check - TPC
            if (TMath::Abs(nSigmaPion_TPC)<2 && TMath::Abs(nSigmaKaon_TPC)>2 && TMath::Abs(nSigmaProton_TPC)>2 ){
              isPItpc = kTRUE;
              nPIDtpc = 1;
              nPID=2;
            }else isPItpc = kFALSE; 

            // KAON check - TPC
            if (TMath::Abs(nSigmaKaon_TPC)<2 && TMath::Abs(nSigmaPion_TPC)>3 && TMath::Abs(nSigmaProton_TPC)>2 ){
              isKtpc = kTRUE;
              nPIDtpc = 2;
              nPID=3;
            }else isKtpc = kFALSE;

            // PROTON check - TPC
            if (TMath::Abs(nSigmaProton_TPC)<2 && TMath::Abs(nSigmaPion_TPC)>3 && TMath::Abs(nSigmaKaon_TPC)>2 ){
              isPtpc = kTRUE;
              nPIDtpc = 3;
              nPID=4;
            }else isPtpc = kFALSE;
          }  // cut on track pT for TPC

          // check track has pT < 0.500 GeV - use ITS pid
          if (pt<0.500 && ITSsig>0) {
            nPIDits = 4;
            nPID = 5;

            // PION check - ITS
            if (TMath::Abs(nSigmaPion_ITS)<2 && TMath::Abs(nSigmaKaon_ITS)>2 && TMath::Abs(nSigmaProton_ITS)>2 ){
              isPIits = kTRUE;
              nPIDits = 1; 
	          nPID=6;
            }else isPIits = kFALSE;

            // KAON check - ITS
            if (TMath::Abs(nSigmaKaon_ITS)<2 && TMath::Abs(nSigmaPion_ITS)>3 && TMath::Abs(nSigmaProton_ITS)>2 ){
              isKits = kTRUE;
              nPIDits = 2;
              nPID=7;
            }else isKits = kFALSE;

            // PROTON check - ITS
            if (TMath::Abs(nSigmaProton_ITS)<2 && TMath::Abs(nSigmaPion_ITS)>3 && TMath::Abs(nSigmaKaon_ITS)>2 ){
              isPits = kTRUE;
              nPIDits = 3;
	          nPID=8;
            }else isPits = kFALSE;
          }  // cut on track pT for ITS

          // check track has 0.900 GeV < pT < 2.500 GeV - use TOF pid
          if (pt>0.900 && pt<2.500 && TOFsig>0) {
	        nPIDtof = 4;
            nPID = 9;

            // PION check - TOF
            if (TMath::Abs(nSigmaPion_TOF)<2 && TMath::Abs(nSigmaKaon_TOF)>2 && TMath::Abs(nSigmaProton_TOF)>2 ){
              isPItof = kTRUE;
              nPIDtof = 1;
              nPID=10;
            }else isPItof = kFALSE;

            // KAON check - TOF
            if (TMath::Abs(nSigmaKaon_TOF)<2 && TMath::Abs(nSigmaPion_TOF)>3 && TMath::Abs(nSigmaProton_TOF)>2 ){
              isKtof = kTRUE;
              nPIDtof = 2;
              nPID=11;
            }else isKtof = kFALSE;

            // PROTON check - TOF
            if (TMath::Abs(nSigmaProton_TOF)<2 && TMath::Abs(nSigmaPion_TOF)>3 && TMath::Abs(nSigmaKaon_TOF)>2 ){
              isPtof = kTRUE;
              nPIDtof = 3;
              nPID=12;
            }else isPtof = kFALSE;
          }  // cut on track pT for TOF

          if (nPID == -99) nPID = 14;
 	      fHistPID->Fill(nPID);

          // PID sparse getting filled 
          if (allpidAXIS) { // FILL ALL axis
			Double_t pid_EntriesALL[21] = {fCent,pt,charge,deta,dphijh,leadjet,zVtx,dEP,jetPt,
                                      nSigmaPion_TPC, nSigmaPion_TOF, // pion nSig values in TPC/TOF
  				      				  nPIDtpc, nPIDits, nPIDtof,       // PID label for each detector
 									  nSigmaProton_TPC, nSigmaKaon_TPC,  // nSig in TPC
                                      nSigmaPion_ITS, nSigmaProton_ITS, nSigmaKaon_ITS,  // nSig in ITS
                                      nSigmaProton_TOF, nSigmaKaon_TOF,  // nSig in TOF
                                      }; //array for PID sparse     
		    fhnPID->Fill(pid_EntriesALL);
		  } else {
            // PID sparse getting filled 
            Double_t pid_Entries[14] = {fCent,pt,charge,deta,dphijh,leadjet,zVtx,dEP,jetPt,
                                      nSigmaPion_TPC, nSigmaPion_TOF, // pion nSig values in TPC/TOF
  				      				  nPIDtpc, nPIDits, nPIDtof       // PID label for each detector
                                      }; //array for PID sparse                           
            fhnPID->Fill(pid_Entries);   // fill Sparse histo of PID tracks 
		  } // minimal pid sparse filling

       	} // end of doPID check

  	    // get track pt bin
        Int_t itrackpt = -500;              // initialize track pT bin
        itrackpt = GetpTtrackBin(track->Pt());

	    // all tracks: jet hadron correlations in hadron pt bins
        if(makeextraCORRhistos) fHistJetHadbindPhi[itrackpt]->Fill(dphijh);

        // in plane and out of plane jet-hadron histo's
        if( dEP>0 && dEP<=(TMath::Pi()/6) ){
          // we are IN plane
          if(makeextraCORRhistos) fHistJetHaddPhiINcent[centbin]->Fill(dphijh);
          fHistJetHaddPhiIN->Fill(dphijh);
          if(makeextraCORRhistos) fHistJetHadbindPhiIN[itrackpt]->Fill(dphijh);
        }else if( dEP>(TMath::Pi()/3) && dEP<=(TMath::Pi()/2) ){
          // we are OUT of PLANE
          if(makeextraCORRhistos) fHistJetHaddPhiOUTcent[centbin]->Fill(dphijh);
          fHistJetHaddPhiOUT->Fill(dphijh);
          if(makeextraCORRhistos) fHistJetHadbindPhiOUT[itrackpt]->Fill(dphijh);
        }else if( dEP>(TMath::Pi()/6) && dEP<=(TMath::Pi()/3) ){ 
          // we are in the middle of plane
          if(makeextraCORRhistos) fHistJetHaddPhiMIDcent[centbin]->Fill(dphijh);
          fHistJetHaddPhiMID->Fill(dphijh);
          if(makeextraCORRhistos) fHistJetHadbindPhiMID[itrackpt]->Fill(dphijh);
        }
      } // loop over tracks found in event with highest JET pT > 10.0 GeV (change)
    } // jet pt cut
  } // jet loop

  fHistEventQA->Fill(14); // events right before event mixing

// ***************************************************************************************************************
// ******************************** Event MIXING *****************************************************************
// ***************************************************************************************************************

  // initialize object array of cloned picotracks
  TObjArray* tracksClone = 0x0;
  
  // PbPb collisions - create cloned picotracks
  //if(GetBeamType() == 1) tracksClone = CloneAndReduceTrackList(tracks); // TEST

  //Prepare to do event mixing
  if(fDoEventMixing>0){
    // event mixing

    // 1. First get an event pool corresponding in mult (cent) and
    //    zvertex to the current event. Once initialized, the pool
    //    should contain nMix (reduced) events. This routine does not
    //    pre-scan the chain. The first several events of every chain
    //    will be skipped until the needed pools are filled to the
    //    specified depth. If the pool categories are not too rare, this
    //    should not be a problem. If they are rare, you could lose
    //    statistics.

    // 2. Collect the whole pool's content of tracks into one TObjArray
    //    (bgTracks), which is effectively a single background super-event.

    // 3. The reduced and bgTracks arrays must both be passed into
    //    FillCorrelations(). Also nMix should be passed in, so a weight
    //    of 1./nMix can be applied.

    // mix jets from triggered events with tracks from MB events
    // get the trigger bit, need to change trigger bits between different runs
    UInt_t trigger = ((AliInputEventHandler*)(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler()))->IsEventSelected();
    // if event was not selected (triggered) for any reseason (should never happen) then return
    if (trigger==0)  return kTRUE;

    // initialize event pools
    AliEventPool* pool = 0x0;
    AliEventPool* poolpp = 0x0;
    Double_t Ntrks = -999;

    // pp collisions - get event pool
    if(GetBeamType() == 0) {
      Ntrks=(Double_t)Ntracks*1.0;
      //cout<<"Test.. Ntrks: "<<fPoolMgr->GetEventPool(Ntrks);
      poolpp = fPoolMgr->GetEventPool(Ntrks, zVtx); // for pp
    }

    // PbPb collisions - get event pool 
    if(GetBeamType() == 1) pool = fPoolMgr->GetEventPool(fCent, zVtx); // for PbPb? fcent

    // if we don't have a pool, return
    if (!pool && !poolpp){
      if(GetBeamType() == 1) AliFatal(Form("No pool found for centrality = %f, zVtx = %f", fCent, zVtx));
      if(GetBeamType() == 0) AliFatal(Form("No pool found for multiplicity = %f, zVtx = %f", Ntrks, zVtx));
      return kTRUE;
    }

    fHistEventQA->Fill(15); // mixed events cases that have pool

    // initialize background tracks array
    TObjArray* bgTracks;

    // next line might not apply for PbPb collisions
    // use only jets from EMCal-triggered events (for lhc11a use AliVEvent::kEMC1)
    //check for a trigger jet
    // fmixingtrack/10 ??
  if(GetBeamType() == 1) if(trigger & fTriggerEventType) { //kEMCEJE)) {     
    if (pool->IsReady() || pool->NTracksInPool() > fNMIXtracks || pool->GetCurrentNEvents() >= fNMIXevents) {

      // loop over jets (passing cuts?)
      for (Int_t ijet = 0; ijet < Njets; ijet++) {
        Double_t leadjet=0;
        if (ijet==ijethi) leadjet=1;

        // get jet object
        AliEmcalJet *jet = static_cast<AliEmcalJet*>(jets->At(ijet));
	    if (!jet) continue;

		// (should probably be higher..., but makes a cut on jet pT)
     	if (jet->Pt()<0.1) continue;
    	if (!AcceptMyJet(jet)) continue;

        fHistEventQA->Fill(16); // event mixing jets
		
        // set cut to do event mixing only if we have a jet meeting our pt threshold (bias applied below)
        if (jet->Pt()<fJetPtcut) continue;

        // get number of current events in pool
        Int_t nMix = pool->GetCurrentNEvents();  // how many particles in pool to mix

          // Fill for biased jet triggers only
        if ((jet->MaxTrackPt()>fTrkBias) || (jet->MaxClusterPt()>fClusBias)) {  // && jet->Pt() > fJetPtcut) {
          // Fill mixed-event histos here  
          for (Int_t jMix=0; jMix<nMix; jMix++) {
			fHistEventQA->Fill(17); // event mixing nMix                 

		    // get jMix'th event
			bgTracks = pool->GetEvent(jMix);
            const Int_t Nbgtrks = bgTracks->GetEntries();
            for(Int_t ibg=0; ibg<Nbgtrks; ibg++) {
              AliPicoTrack *part = static_cast<AliPicoTrack*>(bgTracks->At(ibg));
              if(!part) continue;
              if(TMath::Abs(part->Eta())>0.9) continue;
              if(part->Pt()<0.15) continue;

              Double_t DEta = part->Eta()-jet->Eta();                // difference in eta
              Double_t DPhi = RelativePhi(jet->Phi(),part->Phi());   // difference in phi
              Double_t dEP = RelativeEPJET(jet->Phi(),fEPV0);	     // difference between jet and EP
		      Double_t mixcharge = part->Charge();
              //Double_t DR=TMath::Sqrt(DPhi*DPhi+DEta*DEta);      // difference in R
                           
              // create / fill mixed event sparse
              Double_t triggerEntries[9] = {fCent,jet->Pt(),part->Pt(),DEta,DPhi,dEP,zVtx, mixcharge, leadjet}; //array for ME sparse
              fhnMixedEvents->Fill(triggerEntries,1./nMix);   // fill Sparse histo of mixed events

              fHistEventQA->Fill(18); // event mixing - nbgtracks
              if(makeextraCORRhistos) fHistMEphieta->Fill(DPhi,DEta, 1./nMix);
            } // end of background track loop
          } // end of filling mixed-event histo's
        } // end of check for biased jet triggers
      } // end of jet loop
    } // end of check for pool being ready
  } // end EMC triggered loop

//=============================================================================================================

    // use only jets from EMCal-triggered events (for lhc11a use AliVEvent::kEMC1)
///    if (trigger & AliVEvent::kEMC1) {
    // pp collisions
    if(GetBeamType() == 0) if(trigger & fTriggerEventType) { //kEMC1)) {     
      if (poolpp->IsReady() || poolpp->NTracksInPool() > fNMIXtracks || poolpp->GetCurrentNEvents() >= fNMIXevents) {

        // loop over jets (passing cuts?)
        for (Int_t ijet = 0; ijet < Njets; ijet++) {
          Double_t leadjet=0;
          if (ijet==ijethi) leadjet=1;

          // get jet object
          AliEmcalJet *jet = static_cast<AliEmcalJet*>(jets->At(ijet));
		  if (!jet) continue;

		  // (should probably be higher..., but makes a cut on jet pT)
     	  if (jet->Pt()<0.1) continue;
    	  if (!AcceptMyJet(jet)) continue;

          fHistEventQA->Fill(16); // event mixing jets

          // set cut to do event mixing only if we have a jet meeting our pt threshold (bias applied below)
		  if (jet->Pt()<fJetPtcut) continue;

          // get number of current events in pool 
          Int_t nMix = poolpp->GetCurrentNEvents();  // how many particles in pool to mix

          // Fill for biased jet triggers only
          if ((jet->MaxTrackPt()>fTrkBias) || (jet->MaxClusterPt()>fClusBias)) {  // && jet->Pt() > fJetPtcut) {
            // Fill mixed-event histos here  
            for (Int_t jMix=0; jMix<nMix; jMix++) {
			  fHistEventQA->Fill(17); // event mixing nMix                 

		      // get jMix'th event
			  bgTracks = poolpp->GetEvent(jMix);
              const Int_t Nbgtrks = bgTracks->GetEntries();
              for(Int_t ibg=0; ibg<Nbgtrks; ibg++) {
                AliPicoTrack *part = static_cast<AliPicoTrack*>(bgTracks->At(ibg));
                if(!part) continue;
                if(TMath::Abs(part->Eta())>0.9) continue;
                if(part->Pt()<0.15) continue;

                Double_t DEta = part->Eta()-jet->Eta();                // difference in eta
                Double_t DPhi = RelativePhi(jet->Phi(),part->Phi());   // difference in phi
                Double_t dEP = RelativeEPJET(jet->Phi(),fEPV0);	       // difference between jet and EP
                Double_t mixcharge = part->Charge();
                //Double_t DR=TMath::Sqrt(DPhi*DPhi+DEta*DEta);        // difference in R

                // create / fill mixed event sparse
                Double_t triggerEntries[9] = {fCent,jet->Pt(),part->Pt(),DEta,DPhi,dEP,zVtx, mixcharge, leadjet}; //array for ME sparse
                fhnMixedEvents->Fill(triggerEntries,1./nMix);   // fill Sparse histo of mixed events

                fHistEventQA->Fill(18); // event mixing - nbgtracks
                if(makeextraCORRhistos) fHistMEphieta->Fill(DPhi,DEta, 1./nMix);
              } // end of background track loop
            } // end of filling mixed-event histo's
          } // end of check for biased jet triggers
        } // end of jet loop
      } // end of check for pool being ready
    } //end EMC triggered loop

    // pp collisions
    if(GetBeamType() == 0) {

      // use only tracks from MB events (for lhc11a use AliVEvent::kMB) //kAnyINT as test
      if(trigger & fMixingEventType) { //kMB) {

        // create a list of reduced objects. This speeds up processing and reduces memory consumption for the event pool
        tracksClone = CloneAndReduceTrackList(tracks);

        // update pool if jet in event or not
        poolpp->UpdatePool(tracksClone);

      } // check on track from MB events
    }

    // PbPb collisions
    if(GetBeamType() == 1) {
      
      // use only tracks from MB events
      if(trigger & fMixingEventType) { //kMB) {

        // create a list of reduced objects. This speeds up processing and reduces memory consumption for the event pool
        tracksClone = CloneAndReduceTrackList(tracks);

        // update pool if jet in event or not
        pool->UpdatePool(tracksClone);

      } // MB events
    } // PbPb collisions
  } // end of event mixing

  // print some stats on the event
  event++;
  fHistEventQA->Fill(19);  // events making it to end  

  if (doComments) {
    cout<<"Event #: "<<event<<"     Jet Radius: "<<fJetRad<<"     Constituent Pt Cut: "<<fConstituentCut<<endl;
    cout<<"# of jets: "<<Njets<<"      NjetAcc: "<<NjetAcc<<"      Highest jet pt: "<<highestjetpt<<"     leading hadron pt: "<<leadhadronPT<<endl;
    cout<<"# tracks: "<<Ntracks<<"      NtrackAcc: "<<NtrackAcc<<"      Highest track pt: "<<ptmax<<endl;
    cout<<" =============================================== "<<endl;
  }

  return kTRUE;  // used when the function is of type bool
}  // end of RUN

//________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::GetCentBin(Double_t cent) const
{  // Get centrality bin.
  Int_t centbin = -1;
  if (cent>=0 && cent<10)	centbin = 0;
  else if (cent>=10 && cent<20)	centbin = 1;
  else if (cent>=20 && cent<30) centbin = 2;
  else if (cent>=30 && cent<40)	centbin = 3;
  else if (cent>=40 && cent<50) centbin = 4;
  else if (cent>=50 && cent<90)	centbin = 5;
 
  return centbin;
}

//________________________________________________________________________
Double_t AliAnalysisTaskEmcalJetHadEPpid::RelativePhi(Double_t mphi,Double_t vphi) const
{ // function to calculate relative PHI
  double dphi = mphi-vphi;

  // set dphi to operate on adjusted scale
  if(dphi<-0.5*TMath::Pi()) dphi+=2.*TMath::Pi();
  if(dphi>3./2.*TMath::Pi()) dphi-=2.*TMath::Pi();

  // test
  if( dphi < -1.*TMath::Pi()/2 || dphi > 3.*TMath::Pi()/2 )
    AliWarning(Form("%s: dPHI not in range [-0.5*Pi, 1.5*Pi]!", GetName()));

  return dphi; // dphi in [-0.5Pi, 1.5Pi]                                                                                   
}

//_________________________________________________________________________
Double_t AliAnalysisTaskEmcalJetHadEPpid:: RelativeEPJET(Double_t jetAng, Double_t EPAng) const
{ // function to calculate angle between jet and EP in the 1st quadrant (0,Pi/2)
  Double_t dphi = (EPAng - jetAng);
  
  // ran into trouble with a few dEP<-Pi so trying this...
  if( dphi<-1*TMath::Pi() ){
    dphi = dphi + 1*TMath::Pi();
  } // this assumes we are doing full jets currently 
  
  if( (dphi>0) && (dphi<1*TMath::Pi()/2) ){
    // Do nothing! we are in quadrant 1
  }else if( (dphi>1*TMath::Pi()/2) && (dphi<1*TMath::Pi()) ){
    dphi = 1*TMath::Pi() - dphi;
  }else if( (dphi<0) && (dphi>-1*TMath::Pi()/2) ){
    dphi = fabs(dphi);
  }else if( (dphi<-1*TMath::Pi()/2) && (dphi>-1*TMath::Pi()) ){
    dphi = dphi + 1*TMath::Pi();
  } 
  
  // test
  if( dphi < 0 || dphi > TMath::Pi()/2 )
    AliWarning(Form("%s: dPHI not in range [0, 0.5*Pi]!", GetName()));

  return dphi;   // dphi in [0, Pi/2]
}

//Int_t ieta=GetEtaBin(deta);
//________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::GetEtaBin(Double_t eta) const
{
  // Get eta bin for histos.
  Int_t etabin = -1;
  if (TMath::Abs(eta)<=0.4)				                etabin = 0;
  else if (TMath::Abs(eta)>0.4 && TMath::Abs(eta)<0.8)	etabin = 1;
  else if (TMath::Abs(eta)>=0.8)			            etabin = 2;

  return etabin;
} // end of get-eta-bin

//________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::GetpTjetBin(Double_t pt) const
{
  // Get jet pt  bin for histos.
  Int_t ptbin = -1;
  if (pt>=15 && pt<20)		ptbin = 0;
  else if (pt>=20 && pt<25)	ptbin = 1;
  else if (pt>=25 && pt<40)	ptbin = 2;
  else if (pt>=40 && pt<60)	ptbin = 3;
  else if (pt>=60)	     	ptbin = 4;

  return ptbin;
} // end of get-jet-pt-bin

//________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::GetpTtrackBin(Double_t pt) const
{
  // May need to update bins for future runs... (testing locally)

  // Get track pt bin for histos.
  Int_t ptbin = -1;
  if (pt < 0.5)			ptbin = 0;
  else if (pt>=0.5 && pt<1.0)	ptbin = 1;
  else if (pt>=1.0 && pt<1.5)	ptbin = 2;
  else if (pt>=1.5 && pt<2.0)	ptbin = 3;
  else if (pt>=2.0 && pt<2.5)	ptbin = 4;
  else if (pt>=2.5 && pt<3.0)	ptbin = 5;
  else if (pt>=3.0 && pt<4.0)	ptbin = 6;
  else if (pt>=4.0 && pt<5.0)	ptbin = 7;
  else if (pt>=5.0)		ptbin = 8;

  return ptbin;
} // end of get-jet-pt-bin

//___________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::GetzVertexBin(Double_t zVtx) const
{
  // get z-vertex bin for histo.
  int zVbin= -1;
  if (zVtx>=-10 && zVtx<-8)	    zVbin = 0;
  else if (zVtx>=-8 && zVtx<-6)	zVbin = 1;
  else if (zVtx>=-6 && zVtx<-4)	zVbin = 2;
  else if (zVtx>=-4 && zVtx<-2)	zVbin = 3; 
  else if (zVtx>=-2 && zVtx<0)	zVbin = 4;
  else if (zVtx>=0 && zVtx<2)	zVbin = 5;
  else if (zVtx>=2 && zVtx<4)	zVbin = 6;
  else if (zVtx>=4 && zVtx<6)	zVbin = 7;
  else if (zVtx>=6 && zVtx<8)	zVbin = 8;
  else if (zVtx>=8 && zVtx<10)	zVbin = 9;
  else zVbin = 10;
        
  return zVbin;
} // end of get z-vertex bin

//______________________________________________________________________
THnSparse* AliAnalysisTaskEmcalJetHadEPpid::NewTHnSparseF(const char* name, UInt_t entries)
{
   // generate new THnSparseF, axes are defined in GetDimParams()
   Int_t count = 0;
   UInt_t tmp = entries;
   while(tmp!=0){
      count++;
      tmp = tmp &~ -tmp;  // clear lowest bit
   }

   TString hnTitle(name);
   const Int_t dim = count;
   Int_t nbins[dim];
   Double_t xmin[dim];
   Double_t xmax[dim];

   Int_t i=0;
   Int_t c=0;
   while(c<dim && i<32){
      if(entries&(1<<i)){
         TString label("");
         GetDimParams(i, label, nbins[c], xmin[c], xmax[c]);
         hnTitle += Form(";%s",label.Data());
         c++;
      }

      i++;
   }
   hnTitle += ";";

   return new THnSparseF(name, hnTitle.Data(), dim, nbins, xmin, xmax);
} // end of NewTHnSparseF

void AliAnalysisTaskEmcalJetHadEPpid::GetDimParams(Int_t iEntry, TString &label, Int_t &nbins, Double_t &xmin, Double_t &xmax)
{
   // stores label and binning of axis for THnSparse
   const Double_t pi = TMath::Pi();

   switch(iEntry){

   case 0:
      label = "V0 centrality (%)";
      nbins = 10;
      xmin = 0.;
      xmax = 100.;
      break;

   case 1:
      label = "Jet p_{T}";
      nbins = 50; // 216
      xmin = 0.;
      xmax = 250.; // 216
      break;

   case 2:
      label = "Track p_{T}";
      nbins = 80; //300; // 750 pid
      xmin = 0.;
      xmax = 20.; //75.;
      break;

    case 3:
      label = "Relative Eta";
      nbins = 56; // 42
      xmin = -1.4;
      xmax = 1.4;
      break;

   case 4: 
      label = "Relative Phi";
      nbins = 72;
      xmin = -0.5*pi;
      xmax = 1.5*pi;
      break;

  case 5:
      label = "Relative angle of Jet and Reaction Plane";
      nbins = 3; // (12) 72
      xmin = 0;
      xmax = 0.5*pi;
      break;

  case 6:
      label = "z-vertex";
      nbins = 10;
      xmin = -10;
      xmax =  10;
      break;

  case 7:
      label = "track charge";
      nbins = 3;
      xmin = -1.5;
      xmax = 1.5;
      break;

  case 8:
      label = "leading jet";
      nbins = 3;
      xmin = -0.5;
      xmax = 2.5;
      break;

  case 9: // need to update
      label = "leading track";
      nbins = 10;
      xmin = 0;
      xmax = 50;
      break; 

   } // end of switch
} // end of getting dim-params

//_________________________________________________
// From CF event mixing code PhiCorrelations
TObjArray* AliAnalysisTaskEmcalJetHadEPpid::CloneAndReduceTrackList(TObjArray* tracksME)
{
  // clones a track list by using AliPicoTrack which uses much less memory (used for event mixing)
  TObjArray* tracksClone = new TObjArray;
  tracksClone->SetOwner(kTRUE);

// ===============================

//      cout << "RM Hybrid track : " << i << "  " << particle->Pt() << endl;  

  //cout << "nEntries " << tracks->GetEntriesFast() <<endl;
  for (Int_t i=0; i<tracksME->GetEntriesFast(); i++) {         // AOD/general case
    AliVParticle* particle = (AliVParticle*) tracksME->At(i);  // AOD/general case
    if(TMath::Abs(particle->Eta())>fTrkEta) continue;
    if(particle->Pt()<0.15)continue;

/*
// DON'T USE
    Double_t trackpt=particle->Pt();   // track pT

    Int_t trklabel=-1;
    trklabel=particle->GetLabel();
    //cout << "TRACK_LABEL: " << particle->GetLabel()<<endl;

    Int_t hadbin=-1;
    if(trackpt<0.5) hadbin=0;
    else if(trackpt<1) hadbin=1;
    else if(trackpt<2) hadbin=2;
    else if(trackpt<3) hadbin=3;
    else if(trackpt<5) hadbin=4;
    else if(trackpt<8) hadbin=5;
    else if(trackpt<20) hadbin=6;
// end of DON'T USE

//feb10 comment out
    if(hadbin>-1 && trklabel>-1 && trklabel <3) fHistTrackEtaPhi[trklabel][hadbin]->Fill(particle->Eta(),particle->Phi());
    if(hadbin>-1) fHistTrackEtaPhi[3][hadbin]->Fill(particle->Eta(),particle->Phi());

    if(hadbin>-1) fHistTrackEtaPhi[hadbin]->Fill(particle->Eta(),particle->Phi());  // TEST
*/

    tracksClone->Add(new AliPicoTrack(particle->Pt(), particle->Eta(), particle->Phi(), particle->Charge(), 0, 0, 0, 0));
  } // end of looping through tracks

  return tracksClone;
}

//____________________________________________________________________________________________
THnSparse* AliAnalysisTaskEmcalJetHadEPpid::NewTHnSparseFPID(const char* name, UInt_t entries)
{
   // generate new THnSparseF PID, axes are defined in GetDimParams()
   Int_t count = 0;
   UInt_t tmp = entries;
   while(tmp!=0){
      count++;
      tmp = tmp &~ -tmp;  // clear lowest bit
   }

   TString hnTitle(name);
   const Int_t dim = count;
   Int_t nbins[dim];
   Double_t xmin[dim];
   Double_t xmax[dim];

   Int_t i=0;
   Int_t c=0;
   while(c<dim && i<32){
      if(entries&(1<<i)){
         TString label("");
         GetDimParamsPID(i, label, nbins[c], xmin[c], xmax[c]);
         hnTitle += Form(";%s",label.Data());
         c++;
      }

      i++;
   }
   hnTitle += ";";

   return new THnSparseF(name, hnTitle.Data(), dim, nbins, xmin, xmax);
} // end of NewTHnSparseF PID

//________________________________________________________________________________
void AliAnalysisTaskEmcalJetHadEPpid::GetDimParamsPID(Int_t iEntry, TString &label, Int_t &nbins, Double_t &xmin, Double_t &xmax)
{
   // stores label and binning of axis for THnSparse
   const Double_t pi = TMath::Pi();

   switch(iEntry){

   case 0:
      label = "V0 centrality (%)";
      nbins = 10;
      xmin = 0.;
      xmax = 100.;
      break;

   case 1:
      label = "Track p_{T}";
      nbins = 80; //300; // 750 
      xmin = 0.;
      xmax = 20.; //75.; 
      break;

   case 2:
      label = "Charge of Track";
      nbins = 3;
      xmin = -1.5;
      xmax = 1.5;
      break;

   case 3:
      label = "Relative Eta of Track and Jet";
      nbins = 56; // 48
      xmin = -1.4;
      xmax = 1.4;
      break;

   case 4:
      label = "Relative Phi of Track and Jet";
      nbins = 72;
      xmin = -0.5*pi;
      xmax = 1.5*pi;
      break;

   case 5:
      label = "leading jet";
      nbins = 3;
      xmin = -.5;
      xmax = 2.5;
      break;

   case 6:
      label = "Z-vertex";
      nbins = 10;
      xmin = -10.;
      xmax = 10.;
      break;

   case 7: 
      label = "Relative angle: Jet and Reaction Plane";
      nbins = 3; // (12) 48
      xmin = 0.;
      xmax = 0.5*pi;
      break;

   case 8: 
      label = "Jet p_{T}";
      nbins = 50; // 216 
      xmin = 0.;
      xmax = 250.; // 216
      break;

   case 9:
      label = "N-Sigma of pions in TPC";
      nbins = 200;
      xmin = -10.0;
      xmax = 10.0; 
      break;

   case 10:
      label = "N-Sigma of pions in TOF";
      nbins = 200;
      xmin = -10.;
      xmax = 10.; 
      break;

   case 11:
      label = "TPC PID determination";
      nbins = 5;
      xmin = 0.;
      xmax = 5.;
      break;

   case 12:
      label = "ITS PID determination";
      nbins = 5;
      xmin = 0.;
      xmax = 5.;
      break;

   case 13:
      label = "TOF PID determination";
      nbins = 5;
      xmin = 0.;
      xmax = 5.;
      break;

   case 14:
      label = "N-Sigma of protons in TPC";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   case 15:
      label = "N-Sigma of kaons in TPC";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   case 16:
      label = "N-Sigma of pions in ITS";
      nbins = 200;
      xmin = -10.0;
      xmax = 10.0; 
      break;

   case 17:
      label = "N-Sigma of protons in ITS";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   case 18:
      label = "N-Sigma of kaons in ITS";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   case 19:
      label = "N-Sigma of protons in TOF";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   case 20:
      label = "N-Sigma of kaons in TOF";
      nbins = 200;
      xmin = -10.;
      xmax = 10.;
      break;

   } // end of switch
} // end of get dimension parameters PID

void AliAnalysisTaskEmcalJetHadEPpid::Terminate(Option_t *) {
  cout<<"#########################"<<endl;
  cout<<"#### DONE RUNNING!!! ####"<<endl;
  cout<<"#########################"<<endl;
} // end of terminate

//________________________________________________________________________
Int_t AliAnalysisTaskEmcalJetHadEPpid::AcceptMyJet(AliEmcalJet *jet) {
  //applies all jet cuts except pt
  if ((jet->Phi()<fPhimin)||(jet->Phi()>fPhimax)) return 0;
  if ((jet->Eta()<fEtamin)||(jet->Eta()>fEtamax)) return 0;
  if (jet->Area()<fAreacut) return 0;
  // prevents 0 area jets from sneaking by when area cut == 0
  if (jet->Area()==0) return 0;
  //exclude jets with extremely high pt tracks which are likely misreconstructed
  if(jet->MaxTrackPt()>100) return 0;

  //passed all above cuts
  return 1;
}

//void AliAnalysisTaskEmcalJetHadEPpid::FillAnalysisSummaryHistogram() const
void AliAnalysisTaskEmcalJetHadEPpid::SetfHistPIDcounterLabels(TH1* h) const
{
    // fill the analysis summary histrogram, saves all relevant analysis settigns
    h->GetXaxis()->SetBinLabel(1, "TPC: Unidentified");
    h->GetXaxis()->SetBinLabel(2, "TPC: Pion");
    h->GetXaxis()->SetBinLabel(3, "TPC: Kaon");
    h->GetXaxis()->SetBinLabel(4, "TPC: Proton");
    h->GetXaxis()->SetBinLabel(5, "ITS: Unidentified");
    h->GetXaxis()->SetBinLabel(6, "ITS: Pion");
    h->GetXaxis()->SetBinLabel(7, "ITS: Kaon");
    h->GetXaxis()->SetBinLabel(8, "ITS: Proton");
    h->GetXaxis()->SetBinLabel(9, "TOF: Unidentified");
    h->GetXaxis()->SetBinLabel(10, "TOF: Pion"); 
    h->GetXaxis()->SetBinLabel(11, "TOF: Kaon");
    h->GetXaxis()->SetBinLabel(12, "TOF: Proton");
    h->GetXaxis()->SetBinLabel(14, "Unidentified tracks");

    // set x-axis labels vertically
    h->LabelsOption("v");
}

//void AliAnalysisTaskEmcalJetHadEPpid::FillAnalysisSummaryHistogram() const
void AliAnalysisTaskEmcalJetHadEPpid::SetfHistQAcounterLabels(TH1* h) const
{
    // label bins of the analysis event summary
    h->GetXaxis()->SetBinLabel(1, "All events started"); 
    h->GetXaxis()->SetBinLabel(2, "object check"); 
    h->GetXaxis()->SetBinLabel(3, "aod/esd check"); 
    h->GetXaxis()->SetBinLabel(4, "centrality check"); 
    h->GetXaxis()->SetBinLabel(5, "zvertex check"); 
    h->GetXaxis()->SetBinLabel(6, "list check"); 
    h->GetXaxis()->SetBinLabel(7, "track/jet pointer check"); 
    h->GetXaxis()->SetBinLabel(8, "tracks & jets < than 1 check"); 
    h->GetXaxis()->SetBinLabel(9, "after track/jet loop to get highest pt"); 
    h->GetXaxis()->SetBinLabel(10, "accepted jets"); 
    h->GetXaxis()->SetBinLabel(11, "jets meeting pt threshold"); 
    h->GetXaxis()->SetBinLabel(12, "accepted tracks in events w/ trigger jet"); 
    h->GetXaxis()->SetBinLabel(13, "after AliVEvent & fPIDResponse"); 
    h->GetXaxis()->SetBinLabel(14, "events before event mixing"); 
    h->GetXaxis()->SetBinLabel(15, "mixed events w/ pool"); 
    h->GetXaxis()->SetBinLabel(16, "event mixing: jets"); 
    h->GetXaxis()->SetBinLabel(17, "event mixing: nMix"); 
    h->GetXaxis()->SetBinLabel(18, "event mixing: nbackground tracks"); 
    h->GetXaxis()->SetBinLabel(19, "event mixing: THE END"); 

    // set x-axis labels vertically
    h->LabelsOption("v");
}

void AliAnalysisTaskEmcalJetHadEPpid::SetfHistEvtSelQALabels(TH1* h) const
{  
    // label bins of the analysis trigger selection summary
    h->GetXaxis()->SetBinLabel(1, "no trigger");
    h->GetXaxis()->SetBinLabel(2, "kAny");
    h->GetXaxis()->SetBinLabel(3, "kAnyINT");
    h->GetXaxis()->SetBinLabel(4, "kMB");
    h->GetXaxis()->SetBinLabel(5, "kINT7");
    h->GetXaxis()->SetBinLabel(6, "kEMC1");
    h->GetXaxis()->SetBinLabel(7, "kEMC7");
    h->GetXaxis()->SetBinLabel(8, "kEMC8");
    h->GetXaxis()->SetBinLabel(9, "kEMCEJE");
    h->GetXaxis()->SetBinLabel(10, "kEMCEGA");
    h->GetXaxis()->SetBinLabel(11, "kCentral");
    h->GetXaxis()->SetBinLabel(12, "kSemiCentral");
    h->GetXaxis()->SetBinLabel(13, "kINT8");
    h->GetXaxis()->SetBinLabel(14, "kEMCEJE or kMB");
    h->GetXaxis()->SetBinLabel(15, "kEMCEGA or kMB");
    h->GetXaxis()->SetBinLabel(16, "kAnyINT or kMB");
    h->GetXaxis()->SetBinLabel(17, "kEMCEJE & kMB");
    h->GetXaxis()->SetBinLabel(18, "kEMCEGA & kMB");
    h->GetXaxis()->SetBinLabel(19, "kAnyINT & kMB");

    // set x-axis labels vertically
    h->LabelsOption("v");
    //h->LabelsDeflate("X");
}

//______________________________________________________________________
THnSparse* AliAnalysisTaskEmcalJetHadEPpid::NewTHnSparseFCorr(const char* name, UInt_t entries) {
  // generate new THnSparseD, axes are defined in GetDimParamsD()
  Int_t count = 0;
  UInt_t tmp = entries;
  while(tmp!=0){
    count++;
    tmp = tmp &~ -tmp;  // clear lowest bit
  }

  TString hnTitle(name);
  const Int_t dim = count;
  Int_t nbins[dim];
  Double_t xmin[dim];
  Double_t xmax[dim];

  Int_t i=0;
  Int_t c=0;
  while(c<dim && i<32){
    if(entries&(1<<i)){
      TString label("");
      GetDimParamsCorr(i, label, nbins[c], xmin[c], xmax[c]);
      hnTitle += Form(";%s",label.Data());
      c++;
    }

    i++;
  }
  hnTitle += ";";

  return new THnSparseF(name, hnTitle.Data(), dim, nbins, xmin, xmax);
} // end of NewTHnSparseF

//______________________________________________________________________________________________
void AliAnalysisTaskEmcalJetHadEPpid::GetDimParamsCorr(Int_t iEntry, TString &label, Int_t &nbins, Double_t &xmin, Double_t &xmax)
{
   //stores label and binning of axis for THnSparse
   const Double_t pi = TMath::Pi();

   switch(iEntry){

    case 0:
      label = "V0 centrality (%)";
      nbins = 10;
      xmin = 0.;
      xmax = 100.;
      break;

   case 1:
      label = "Jet p_{T}";
      nbins = 50; // 216
      xmin = 0.;
      xmax = 250.; // 216
      break;

   case 2:
      label = "Relative angle: Jet and Reaction Plane";
      nbins = 3; // (12) 48
      xmin = 0.;
      xmax = 0.5*pi;
      break;

   case 3:
	  label = "Z-vertex";
      nbins = 10;
      xmin = -10.;
      xmax = 10.;
	  break;

   case 4:
	  label = "Jet p_{T} corrected with Local Rho";
      nbins = 50; // 250
      xmin = -50.;
      xmax = 200.;
	  break;

   case 5:
	  label = "Jet p_{T} corrected with Global Rho";
      nbins = 50; // 250
      xmin = -50.;
      xmax = 200.;
	  break;

   }// end of switch
} // end of Correction (ME) sparse

//________________________________________________________________________
//Int_t AliAnalysisTaskEmcalJetHadEPpid::AcceptFlavourJet(AliEmcalJet* fljet, Int_t NUM, Int_t NUM2, Int_t NUM3) {
Int_t AliAnalysisTaskEmcalJetHadEPpid::AcceptFlavourJet(AliEmcalJet* fljet, Int_t NUM) {
  // Get jet if accepted for given flavour tag
  // If jet not accepted return 0
  if(!fljet) {
    AliError(Form("%s:Jet not found",GetName()));
    return 0;
  }

  Int_t flavNUM = -99;//, flavNUM2 = -99, flavNUM3 = -99; FIXME commented out to avoid compiler warning
  flavNUM = NUM;
  //flavNUM2 = NUM2;
  //flavNUM3 = NUM3;

/*
  // from the AliEmcalJet class, the tagging enumerator
  enum EFlavourTag{
       kDStar = 1<<0, kD0 = 1<<1, 
	   kSig1 = 1<<2, kSig2 = 1<<3, 
	   kBckgrd1 = 1<<4, kBckgrd2 = 1<<5, kBckgrd3 = 1<<6
  }; 
  // bit 0 = no tag, bit 1 = Dstar, bit 2 = D0 and so forth...
*/   

  // get flavour of jet (if any)
  Int_t flav = -999;
  flav = fljet->GetFlavour();

  // cases (for now..)
  // 3 = electron rich, 5 = hadron (bkgrd) rich
  // if flav < 1, its not tagged, so return kFALSE (0)
  if(flav < 1) return 0;

  // if flav is not equal to what we want then return kFALSE (0)
  //if(flav != flavNUM && flav != flavNUM2 && flav != flavNUM3) return 0;
  if(flav != flavNUM) return 0;  

  // we have the flavour we want, so return kTRUE (1)
  //if(flav == flavNUM || flav == flavNUM2 || flav == flavNUM3) return 1;
  if(flav == flavNUM) return 1;

  // we by default have a flavour tagged jet
  return 1;
}
