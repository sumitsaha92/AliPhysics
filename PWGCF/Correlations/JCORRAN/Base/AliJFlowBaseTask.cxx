/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice     */
//______________________________________________________________________________
// Analysis task for providing various flow informations
// author: D.J. Kim(dong.jo.kim@cern.ch)
// ALICE Group University of Jyvaskyla
// Finland
//
// Fill the analysis containers for ESD or AOD
// Note: Adapted for AliAnalysisTaskSE
//////////////////////////////////////////////////////////////////////////////
#include <AliAnalysisManager.h>
#include "AliJBaseTrack.h"
#include "AliJFlowBaseTask.h" 
static double decAcc[AliJFlowBaseTask::D_COUNT][2] = {
	{-0.8,0.8},
	{-1.5,-0.8},
	{0.8,1.5},
	{2.8,5.1},
	{-3.7,-1.7},
	{2.19,5.08}
};

//______________________________________________________________________________
AliJFlowBaseTask::AliJFlowBaseTask() :   
	AliAnalysisTaskSE("JFlowBaseTask"),
	fJCatalystTask(NULL),
	fJCatalystTaskName("JCatalystTask"),
	fOutput(NULL)
{
}

//______________________________________________________________________________
AliJFlowBaseTask::AliJFlowBaseTask(const char *name, TString inputformat):
	AliAnalysisTaskSE(name), 
	fJCatalystTask(NULL),
	fJCatalystTaskName("JCatalystTask"),
	fOutput(NULL)
{
	// Constructor
	AliInfo("---- AliJFlowBaseTask Constructor ----");
	DefineOutput (1, TDirectory::Class());
}

//____________________________________________________________________________
AliJFlowBaseTask::AliJFlowBaseTask(const AliJFlowBaseTask& ap) :
	AliAnalysisTaskSE(ap.GetName()), 
	fJCatalystTask(ap.fJCatalystTask),
	fJCatalystTaskName(ap.fJCatalystTaskName),
	fOutput( ap.fOutput )
{ 

	AliInfo("----DEBUG AliJFlowBaseTask COPY ----");

}

//_____________________________________________________________________________
AliJFlowBaseTask& AliJFlowBaseTask::operator = (const AliJFlowBaseTask& ap)
{
	// assignment operator

	AliInfo("----DEBUG AliJFlowBaseTask operator= ----");
	this->~AliJFlowBaseTask();
	new(this) AliJFlowBaseTask(ap);
	return *this;
}

//______________________________________________________________________________
AliJFlowBaseTask::~AliJFlowBaseTask()
{
	// destructor 
	delete fOutput;

}

//________________________________________________________________________

void AliJFlowBaseTask::UserCreateOutputObjects()
{  
	//=== create the jcorran outputs objects
	if(fDebug > 1) printf("AliJFlowBaseTask::UserCreateOutPutData() \n");
	//=== Get AnalysisManager
	AliAnalysisManager *man = AliAnalysisManager::GetAnalysisManager();

	fJCatalystTask = (AliJCatalystTask*)(man->GetTask( fJCatalystTaskName ));

	OpenFile(1);
	fOutput = gDirectory;
	fOutput->cd();

	PostData(1, fOutput);

}

//______________________________________________________________________________
void AliJFlowBaseTask::UserExec(Option_t* /*option*/) 
{

	// Processing of one event
	if(fDebug > 5) cout << "------- AliJFlowBaseTask Exec-------"<<endl;
	if(!((Entry()-1)%100))  AliInfo(Form(" Processing event # %lld",  Entry())); 
	if( fJCatalystTask->GetJCatalystEntry() != fEntry ) return;
	TClonesArray *fInputList = (TClonesArray*)fJCatalystTask->GetInputList();
	CalculateEventPlane(fInputList);
	if(fDebug > 5) cout << "\t------- End UserExec "<<endl;
}

//______________________________________________________________________________
void AliJFlowBaseTask::Init()
{
	// Intialisation of parameters
	AliInfo("Doing initialization") ; 

}

//______________________________________________________________________________
void AliJFlowBaseTask::Terminate(Option_t *)
{
	// Processing when the event loop is ended
	cout<<"AliJFlowBaseTask Analysis DONE !!"<<endl; 
}

//______________________________________________________________________________
void AliJFlowBaseTask::CalculateEventPlane(TClonesArray *inList) {

	int NtracksDEC[AliJFlowBaseTask::D_COUNT]; // Num of particle in each dector
	int noTracks = inList->GetEntries();
	for(int is = 0; is < AliJFlowBaseTask::D_COUNT; is++)  { 
		NtracksDEC[is] = 0;
		for(int iH=2;iH<=3;iH++) { QvectorsEP[is][iH-2] = TComplex(0,0); }
	}

	for(int itrack=0;itrack<noTracks; itrack++){
		AliJBaseTrack *trk = (AliJBaseTrack*)inList->At(itrack);
		double phi = trk->Phi();
		double eta = trk->Eta();
		for(int is = 0; is < AliJFlowBaseTask::D_COUNT; is++){
			if(decAcc[is][0]<eta && decAcc[is][1]>eta) {
				for(int iH=2;iH<=3;iH++) { QvectorsEP[is][iH-2] += TComplex(TMath::Cos(iH*phi),TMath::Sin(iH*phi));}
				NtracksDEC[is]++;
			}
		}
	}

	for(int is = 0; is < AliJFlowBaseTask::D_COUNT; is++){
		for(int iH=2;iH<=3;iH++) {		
			QvectorsEP[is][iH-2] /= (double)NtracksDEC[is];
			QvectorsEP[is][iH-2] = QvectorsEP[is][iH-2]/TComplex::Abs(QvectorsEP[is][iH-2]);
		}
	}
	//for(int is = 0; is < AliJFlowBaseTask::D_COUNT; is++){
	//	for(int iH=2;iH<=2;iH++) {		
	//		cout << decAcc[is][0]<<"<eta<"<<decAcc[is][1]<<" Nch = "<< NtracksDEC[is] << "\t"<< iH <<"th "<< QvectorsEP[is][iH-2].Theta() << endl;
	//	}
	//}
}
