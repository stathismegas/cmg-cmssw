#ifndef CFWriter_h
#define CFWriter_h

/** \class CFWriter
 *
 * CFWriter is the EDProducer subclass 
 * It copy the transient objects CrossingFrame
 * into the persistent objects PCrossingFrame and
 * write them into the root file 
 *
 * \author Emilia Becheva, LLR Palaiseau
 *
 * \version   1st Version April 2009
 *
 ************************************************************/
 
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
 
#include "DataFormats/Common/interface/Handle.h"
 
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/CrossingFrame/interface/PCrossingFrame.h"

namespace edm
{
class CFWriter : public edm::EDProducer
{
 public:
 
  explicit CFWriter(const edm::ParameterSet& conf);
  
  virtual ~CFWriter();
  
  void beginJob() {}
  void beginRun(const edm::Run& run, const edm::EventSetup& es);
  virtual void produce(edm::Event& e, const edm::EventSetup& c);
  virtual void put(edm::Event &e) {;}

 private:
  inline PCrossingFrame<SimTrack> fctTest (PCrossingFrame<SimTrack> p){ return p; std::cout << " call PCrossingFrame object" << std::endl;};

  virtual void branchesActivate(const std::string &friendlyName, std::string subdet,InputTag &tag,std::string &label);
  std::vector<std::string> wantedBranches_;
  bool useCurrentProcessOnly_;
  bool flagSimTrack_;
  bool flagSimVertex_;
  bool flagHepMCProduct_;
  bool flagPCaloHit_;
  bool flagPSimHit_;
  
  typedef std::vector<edm::HepMCProduct> HepMCProductContainer;
  std::vector<std::string> labSimHit;
  std::vector<std::string> labCaloHit;

};
}//edm
#endif
