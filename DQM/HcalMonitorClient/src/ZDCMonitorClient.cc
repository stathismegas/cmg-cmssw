#include "DQM/HcalMonitorClient/interface/ZDCMonitorClient.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DataFormats/DetId/interface/DetId.h"

#include "TROOT.h"
#include "TTree.h"
#include "TGaxis.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/time.h>

#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"

#include "DQM/HcalMonitorClient/interface/HcalDQMDbInterface.h"
// Use to hold/get channel status
#include "CondFormats/HcalObjects/interface/HcalChannelQuality.h"
#include "CondFormats/HcalObjects/interface/HcalCondObjectContainer.h"
#include "CalibCalorimetry/HcalAlgos/interface/HcalDbASCIIIO.h"
#include "CondFormats/HcalObjects/interface/HcalChannelStatus.h"

#include "DQM/HcalMonitorClient/interface/HcalClientUtils.h"
#include "DQM/HcalMonitorClient/interface/HcalHistoUtils.h"
#include "DQM/HcalMonitorTasks/interface/HcalEtaPhiHists.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMStore.h"


//--------------------------------------------------------
ZDCMonitorClient::ZDCMonitorClient(std::string myname, const edm::ParameterSet& ps){
  name_=myname;
  
  inputFile_ = ps.getUntrackedParameter<std::string>("inputFile","");
  mergeRuns_ = ps.getUntrackedParameter<bool>("mergeRuns", false);
  cloneME_ = ps.getUntrackedParameter<bool>("cloneME", true);
  prescaleFactor_ = ps.getUntrackedParameter<int>("prescaleFactor", -1);
  prefixME_              = ps.getUntrackedParameter<std::string>("subSystemFolder","Hcal/");
  if (prefixME_.substr(prefixME_.size()-1,prefixME_.size())!="/")
    prefixME_.append("/");
  enableCleanup_ = ps.getUntrackedParameter<bool>("enableCleanup", false);
  
  updateTime_ = ps.getUntrackedParameter<int>("UpdateTime",0);
  baseHtmlDir_ = ps.getUntrackedParameter<std::string>("baseHtmlDir", "");
  htmlUpdateTime_ = ps.getUntrackedParameter<int>("htmlUpdateTime", 0);
  htmlFirstUpdate_ = ps.getUntrackedParameter<int>("htmlFirstUpdate",20);
  databasedir_   = ps.getUntrackedParameter<std::string>("databaseDir","");
  databaseUpdateTime_ = ps.getUntrackedParameter<int>("databaseUpdateTime",0);
  databaseFirstUpdate_ = ps.getUntrackedParameter<int>("databaseFirstUpdate",10);

  saveByLumiSection_  = ps.getUntrackedParameter<bool>("saveByLumiSection",false);
  Online_             = ps.getUntrackedParameter<bool>("online",false);
 
  subdir_                = ps.getUntrackedParameter<std::string>("ZDCFolder","ZDCMonitor_Hcal/"); // DeadCellMonitor_Hcal  
  if (subdir_.size()>0 && subdir_.substr(subdir_.size()-1,subdir_.size())!="/")
    subdir_.append("/");
  subdir_=prefixME_+subdir_;

  debug_              = ps.getUntrackedParameter<int>("debug",0);
  ZDCGoodLumi_        = ps.getUntrackedParameter<std::vector<double> > ("ZDC_QIValueForGoodLS");

}


//--------------------------------------------------------
ZDCMonitorClient::~ZDCMonitorClient(){

  if (debug_>0) std::cout << "ZDCMonitorClient: Exit ..." << std::endl;
}


//--------------------------------------------------------
void ZDCMonitorClient::beginJob()
{
  dqmStore_ = edm::Service<DQMStore>().operator->();
  if (debug_>0) 
    {
      std::cout <<"<ZDCMonitorClient::beginJob()>  Displaying dqmStore directory structure:"<<std::endl;
      dqmStore_->showDirStructure();
    }
}

//--------------------------------------------------------
void ZDCMonitorClient::beginRun(void) {

  begin_run_ = true;
  end_run_   = false;

  evt_=0;
  jevt_=0;
  htmlcounter_=0;
  /*  if (!dqmStore_) 
    {
      if (debug_>0) std::cout <<"<ZDCMonitorClient::beginRun> dqmStore does not exist!"<<std::endl;
      return;
      }*/
 //subdir_="Hcal/";
  dqmStore_->setCurrentFolder(subdir_); // what is Hcal/ZDCMonitor/EventInfoDUMMY folder
  
  // Add new histograms; remove those created in previous runs
  // prefixMe = Hcal/

  ZDCChannelSummary_=dqmStore_->get(subdir_ + "ZDC_Channel_Summary");
  if (ZDCChannelSummary_) dqmStore_->removeElement(ZDCChannelSummary_->getName());
  ZDCChannelSummary_= dqmStore_->book2D("ZDC_Channel_Summary", "Fraction of Events where ZDC Channels had no Errors" , 2, 0, 2, 9, 0, 9); //This is the histo which will show the health of each ZDC Channel
  for (int i=0;i<18;++i)
    {
      ZDCChannelSummary_->setBinContent(i/9,i%9,-1);
    }
  (ZDCChannelSummary_->getTH2F())->GetXaxis()->SetBinLabel(1,"ZDC+");
  (ZDCChannelSummary_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(1,"EM1");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(2,"EM2");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(3,"EM3");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(4,"EM4");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(5,"EM5");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(6,"HAD1");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(7,"HAD2");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(8,"HAD3");
  (ZDCChannelSummary_->getTH2F())->GetYaxis()->SetBinLabel(9,"HAD4");
  (ZDCChannelSummary_->getTH2F())->SetOption("textcolz");  
  (ZDCChannelSummary_->getTH2F())->SetMinimum(-1);
  (ZDCChannelSummary_->getTH2F())->SetMaximum(1);


  
  ZDCReportSummary_ = dqmStore_->get(subdir_ + "ZDC_ReportSummary");
  if (ZDCReportSummary_) dqmStore_->removeElement(ZDCReportSummary_->getName());
  ZDCReportSummary_= dqmStore_->book2D("ZDC_ReportSummary","Fraction of Good Lumis for either ZDC",2,0,2,1,0,1);
	  for (int i=0;i<3;++i)
    {
      ZDCReportSummary_->setBinContent(i,1,-1);
    }
  (ZDCReportSummary_->getTH2F())->GetXaxis()->SetBinLabel(1,"ZDC+");
  (ZDCReportSummary_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCReportSummary_->getTH2F())->SetOption("textcolz");
  (ZDCReportSummary_->getTH2F())->SetMinimum(-1);
  (ZDCReportSummary_->getTH2F())->SetMaximum(1);

  ZDCHotChannelFraction_ = dqmStore_->get(subdir_+"Errors/HotChannel/ZDC_Hot_Channel_Fraction");
  if (ZDCHotChannelFraction_) dqmStore_->removeElement(ZDCHotChannelFraction_->getName());
  dqmStore_->setCurrentFolder(subdir_ + "Errors/HotChannel");
  ZDCHotChannelFraction_ = dqmStore_->book2D("ZDC_Hot_Channel_Fraction", "Hot Channel Rates in the ZDC Channels", 2, 0, 2, 9, 0, 9); //Hot channel checker for ZDC
   for (int i=0;i<18;++i)
    {
      ZDCHotChannelFraction_->setBinContent(i/9,i%9,-1);
    }
 (ZDCHotChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(1,"ZDC+");
  (ZDCHotChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(1,"EM1");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(2,"EM2");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(3,"EM3");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(4,"EM4");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(5,"EM5");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(6,"HAD1");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(7,"HAD2");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(8,"HAD3");
  (ZDCHotChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(9,"HAD4");
  (ZDCHotChannelFraction_->getTH2F())->SetOption("textcolz");
  (ZDCHotChannelFraction_->getTH2F())->SetMinimum(-1);
  (ZDCHotChannelFraction_->getTH2F())->SetMaximum(1);
 
  ZDCColdChannelFraction_ = dqmStore_->get(subdir_ + "Errors/ColdChannel/ZDC_Cold_Channel_Fraction");
  if (ZDCColdChannelFraction_) dqmStore_->removeElement(ZDCColdChannelFraction_->getName());
  dqmStore_->setCurrentFolder(subdir_ + "Errors/ColdChannel");
  ZDCColdChannelFraction_=dqmStore_->book2D("ZDC_Cold_Channel_Fraction", "Cold Channel Rates in the ZDC Channels", 2, 0, 2,9, 0, 9); //Cold channel checker for ZDC                    
  for (int i=0;i<18;++i)
    {
      ZDCColdChannelFraction_->setBinContent(i/9,i%9,-1);
    }
  (ZDCColdChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(1,"ZDC+");
  (ZDCColdChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(1,"EM1");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(2,"EM2");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(3,"EM3");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(4,"EM4");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(5,"EM5");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(6,"HAD1");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(7,"HAD2");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(8,"HAD3");
  (ZDCColdChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(9,"HAD4");
  (ZDCColdChannelFraction_->getTH2F())->SetOption("textcolz");
  (ZDCColdChannelFraction_->getTH2F())->SetMinimum(-1);
  (ZDCColdChannelFraction_->getTH2F())->SetMaximum(1);

 
  ZDCDeadChannelFraction_ = dqmStore_->get(subdir_ + "Errors/DeadChannel/ZDC_Dead_Channel_Fraction");
  if ( ZDCDeadChannelFraction_) dqmStore_->removeElement(ZDCDeadChannelFraction_->getName());
  dqmStore_->setCurrentFolder(subdir_+ "Errors/DeadChannel");
  ZDCDeadChannelFraction_=dqmStore_->book2D("ZDC_Dead_Channel_Fraction","Dead Channel Rates in the ZDC Channels",2,0,2,9,0,9);
  for (int i=0;i<18;++i)
    {
      ZDCDeadChannelFraction_->setBinContent(i/9,i%9,-1);
    }
  (ZDCDeadChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(1,"ZDC+");
  (ZDCDeadChannelFraction_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(1,"EM1");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(2,"EM2");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(3,"EM3");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(4,"EM4");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(5,"EM5");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(6,"HAD1");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(7,"HAD2");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(8,"HAD3");
  (ZDCDeadChannelFraction_->getTH2F())->GetYaxis()->SetBinLabel(9,"HAD4");
  (ZDCDeadChannelFraction_->getTH2F())->SetOption("textcolz");
  (ZDCDeadChannelFraction_->getTH2F())->SetMinimum(-1);
  (ZDCDeadChannelFraction_->getTH2F())->SetMaximum(1);

  ZDCDigiErrorFraction_ = dqmStore_->get(subdir_ + "Errors/Digis/ZDC_Digi_Error_Fraction");
  if (ZDCDigiErrorFraction_) dqmStore_->removeElement(ZDCDigiErrorFraction_->getName());
  dqmStore_->setCurrentFolder(subdir_ + "Errors/Digis");
  ZDCDigiErrorFraction_=dqmStore_->book2D("ZDC_Digi_Error_Fraction", "Digi Error Rates in the ZDC Channels", 2, 0, 2,9, 0, 9); //Hot channel checker for ZDC                    
  for (int i=0;i<18;++i)
    {
      ZDCDigiErrorFraction_->setBinContent(i/9,i%9,-1);
    }
  (ZDCDigiErrorFraction_->getTH2F())->GetXaxis()->SetBinLabel(2,"ZDC-");
  (ZDCDigiErrorFraction_->getTH2F())->GetXaxis()->SetBinLabel(1,"EM1");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(2,"EM2");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(3,"EM3");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(4,"EM4");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(5,"EM5");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(6,"HAD1");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(7,"HAD2");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(8,"HAD3");
  (ZDCDigiErrorFraction_->getTH2F())->GetYaxis()->SetBinLabel(9,"HAD4");
  (ZDCDigiErrorFraction_->getTH2F())->SetOption("textcolz");
  (ZDCDigiErrorFraction_->getTH2F())->SetMinimum(-1);
  (ZDCDigiErrorFraction_->getTH2F())->SetMaximum(1);  
}


//--------------------------------------------------------
void ZDCMonitorClient::endJob(void) {

  if( debug_>0 ) 
    std::cout << "ZDCMonitorClient: endJob, ievt = " << ievt_ << std::endl;

  return;
}

//--------------------------------------------------------
void ZDCMonitorClient::endRun(void){analyze();}

void ZDCMonitorClient::setup(void){}
void ZDCMonitorClient::cleanup(void){}

void ZDCMonitorClient::analyze()
{
  if (debug_>0) 
    std::cout <<"<ZDCMonitorClient> Entered ZDCMonitorClient::analyze()"<<std::endl;
  if(debug_>1) std::cout<<"\nZDC Monitor Client heartbeat...."<<std::endl;
  
  float ChannelRatio[18]={0};
   //dqmStore_->runQTests();
  
  // Make a rate plot, by first getting plots from tasks
  MonitorElement* me;
  std::string s;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////  1)   DIGI ERROR RATE PLOT     /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  s=subdir_+"Errors/Digis/ZDC_Digi_Errors";  // prefixME_ = "Hcal/"
  me=dqmStore_->get(s.c_str());

  TH2F* ZdcDigiErrors=0;
  if (me!=0)
    {
      ZdcDigiErrors=HcalUtilsClient::getHisto<TH2F*>(me,false,0,0);
      if (ZdcDigiErrors!=0)
	{
	  int num_events_digis = ZdcDigiErrors->GetBinContent(-1,-1);
	  if(num_events_digis != 0)
	    {
	      for (int i=0;i<18;++i)
		{
		  ZDCDigiErrorFraction_->setBinContent((i/9)+1,(i%9)+1,(ZdcDigiErrors->GetBinContent((i/9)+1,(i%9)+1)*1./num_events_digis));
		}
	    }
	}
    }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// 2)  HOT CHANNEL RATE PLOT     /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Now get Hot Channel plot, used normalized values (num hot/event) in ZDCHotChannelFraction_
  s=subdir_+"Errors/HotChannel/ZDC_Hot_Channel_Errors";
  me=dqmStore_->get(s.c_str());
  TH2F* ZdcHotChannel=0;
  if (me!=0)
    {
      ZdcHotChannel=HcalUtilsClient::getHisto<TH2F*>(me,false,0,0);
      if (ZdcHotChannel!=0)
	{
	  int num_events_hot = ZdcHotChannel->GetBinContent(-1,-1);
	  if(num_events_hot!=0)
	    {
	      for (int i=0;i<18;++i)
		{
		  ZDCHotChannelFraction_->setBinContent((i/9)+1,(i%9)+1,((ZdcHotChannel->GetBinContent((i/9)+1,(i%9)+1))*1./num_events_hot));
		}
	    }			 
	}     
    } // if (me!=0)

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// 3)  Cold CHANNEL RATE PLOT     /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  //Now get Cold Channel plot, used normalized values (num Cold/event) in ZDCColdChannelFraction_
  s=subdir_+"Errors/ColdChannel/ZDC_Cold_Channel_Errors";
  me=dqmStore_->get(s.c_str());
  TH2F* ZdcColdChannel=0;
  if (me!=0)
    {
      ZdcColdChannel=HcalUtilsClient::getHisto<TH2F*>(me,false,0,0);
      if ((ZdcColdChannel)!=0)
	{
	  int num_events_cold = ZdcColdChannel->GetBinContent(-1,-1);
	  if(num_events_cold!=0)
	    {
	      for (int i=0;i<18;++i)
		{
		  ZDCColdChannelFraction_->setBinContent((i/9)+1,(i%9)+1,((ZdcColdChannel->GetBinContent((i/9)+1,(i%9)+1))*1./num_events_cold));
		}
	    }
	}
    }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// 4)  Dead CHANNEL RATE PLOT     /////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  //Now get Cold Channel plot, used normalized values (num Cold/event) in ZDCHotChannelFraction_
  s=subdir_+"Errors/DeadChannel/ZDC_Dead_Channel_Errors";
  me=dqmStore_->get(s.c_str());
  TH2F* ZdcDeadChannel=0;
  if (me!=0)
    {
      ZdcDeadChannel=HcalUtilsClient::getHisto<TH2F*>(me,false,0,0);
      if ((ZdcDeadChannel)!=0)
	{
	  int num_events_dead = ZdcDeadChannel->GetBinContent(-1,-1);
	  if(num_events_dead!=0)
	    {
	      for (int i=0;i<18;++i)
		{
		  ZDCDeadChannelFraction_->setBinContent((i/9)+1,(i%9)+1,(ZdcDeadChannel->GetBinContent((i/9)+1,(i%9)+1))*1./num_events_dead);
		}
	    }
	}
    }





  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// 5)  CHANNEL SUMMARY PLOT     /////////////////////////////////////////
  //     This simply takes the total errors that each channel got (Cold,hot, digi error) sums them up from the 
  //  totalchannelerrors plot and turns them into a rate. 1-errorrate be the number displayed.            //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////


  ///now we will make the channel summary map
  s=subdir_+"Errors/ZDC_TotalChannelErrors";
  me=dqmStore_->get(s.c_str());
  TH2F* ZdcTotalErrors=0;
  if (me!=0)
    {
      ZdcTotalErrors=HcalUtilsClient::getHisto<TH2F*>(me,false,0,0);
      int num_events_errors = ZdcTotalErrors->GetBinContent(-1,-1);
      if(num_events_errors!=0)
	{
	  for (int i=0;i<18;++i)
	    {
	      ChannelRatio[i]=(ZdcTotalErrors->GetBinContent((i/9)+1,(i%9)+1))*1./num_events_errors;
	      ZDCChannelSummary_->setBinContent((i/9)+1,(i%9)+1,1-ChannelRatio[i]);
	    }
	}
    }


  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// 6)  ZDC REPORT SUMMARY PLOT     /////////////////////////////////////////
  //     This is a ratio of GoodLumis/TotalLumis. The determination of which is made by the Quality Index plots.            //
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////

  LumiCounter=0;
  PZDC_GoodLumiCounter=0;
  PZDC_LumiRatio=0.;
  NZDC_GoodLumiCounter=0;
  NZDC_LumiRatio=0.;
  s=subdir_+"EventsVsLS";
  me=dqmStore_->get(s.c_str());
  TH1F* EventsvsLs=0;
  if (me!=0)
    EventsvsLs=HcalUtilsClient::getHisto<TH1F*>(me,false,0,0);
  s=subdir_+"PZDC_QualityIndexVSLB";
  me=dqmStore_->get(s.c_str());
  TH1F* Pzdc_QI=0;
  if (me!=0)
    Pzdc_QI=HcalUtilsClient::getHisto<TH1F*>(me,false,0,0);
  s=subdir_+"NZDC_QualityIndexVSLB";
  me=dqmStore_->get(s.c_str());
  TH1F* Nzdc_QI=0;
  if (me!=0)
    Nzdc_QI=HcalUtilsClient::getHisto<TH1F*>(me,false,0,0);
  
  if(EventsvsLs!=0 && Pzdc_QI!=0 && Nzdc_QI!=0)
    {
      for (int i=1;i<=EventsvsLs->GetNbinsX();++i)
	{
	  if (EventsvsLs->GetBinContent(i)==0)
	    continue;
	  LumiCounter+=1;

	  if(Pzdc_QI!=0)
	    if(Pzdc_QI->GetBinContent(i)>ZDCGoodLumi_[0])
	      PZDC_GoodLumiCounter+=1;

	  if(Nzdc_QI!=0)
	    if(Nzdc_QI->GetBinContent(i)>ZDCGoodLumi_[1])
	      NZDC_GoodLumiCounter+=1;

	  PZDC_LumiRatio=PZDC_GoodLumiCounter*(1./LumiCounter);
	  NZDC_LumiRatio=NZDC_GoodLumiCounter*(1./LumiCounter);
	}
    }

  ZDCReportSummary_->setBinContent(1,1,PZDC_LumiRatio);
  ZDCReportSummary_->setBinContent(2,1,NZDC_LumiRatio);

  return;
}
