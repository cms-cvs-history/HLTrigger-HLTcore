/** \class HLTAnalCand
 *
 * See header file for documentation
 *
 *  $Date: 2006/06/16 18:55:56 $
 *  $Revision: 1.3 $
 *
 *  \author Martin Grunewald
 *
 */

#include "HLTrigger/HLTcore/interface/HLTAnalCand.h"

#include "FWCore/Framework/interface/Handle.h"

#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"

#include "DataFormats/Common/interface/RefToBase.h"

#include "DataFormats/HLTReco/interface/HLTFilterObject.h"

#include<typeinfo>

//
// constructors and destructor
//
 
HLTAnalCand::HLTAnalCand(const edm::ParameterSet& iConfig)
{
   using namespace reco;

   src_ = iConfig.getParameter< std::string > ("src");

   // should use message logger instead of cout!
   std::cout << "HLTAnalCand created: " << src_ << std::endl;

}

HLTAnalCand::~HLTAnalCand()
{
   // should use message logger instead of cout!
   std::cout << "HLTAnalCand destroyed: " << src_ << std::endl;
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void
HLTAnalCand::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace std;
   using namespace reco;

   cout << "HLTAnalCand::filter start: " << src_ << endl;

   // get hold of products from Event

   edm::Handle<HLTFilterObjectWithRefs> ref;
   iEvent.getByLabel(src_,ref);

   HLTParticle particle;
   const Candidate* candidate;

   const unsigned int n(ref->numberParticles());
   std::cout << "HLTAnalCand: Size = " << n << std::endl;
   for (unsigned int i=0; i!=n; i++) {
     particle=ref->getParticle(i);
     candidate=(ref->getParticleRef(i)).get();
     std::cout << "HLTAnalCand: " << i << " E: " 
               << particle.energy() << " " << candidate->energy() << " "  
               << typeid(*candidate).name() << std::endl;
   }
   std::cout << "HLTAnalCand::filter stop: " << src_ << std::endl;
}
