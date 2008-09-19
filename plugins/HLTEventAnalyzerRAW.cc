/** \class HLTEventAnalyzerRAW
 *
 * See header file for documentation
 *
 *  $Date: 2008/09/19 07:18:45 $
 *  $Revision: 1.5 $
 *
 *  \author Martin Grunewald
 *
 */

#include "HLTrigger/HLTcore/interface/HLTEventAnalyzerRAW.h"

// need access to class objects being referenced to get their content!
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidate.h"
#include "DataFormats/EgammaCandidates/interface/Electron.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/Candidate/interface/CompositeCandidate.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/METReco/interface/MET.h"
#include "DataFormats/HcalIsolatedTrack/interface/IsolatedPixelTrackCandidate.h"
#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticle.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"

#include <cassert>

//
// constructors and destructor
//
HLTEventAnalyzerRAW::HLTEventAnalyzerRAW(const edm::ParameterSet& ps) : 
  processName_(ps.getParameter<std::string>("processName")),
  triggerName_(ps.getParameter<std::string>("triggerName")),
  triggerResultsTag_(ps.getParameter<edm::InputTag>("triggerResults")),
  triggerEventWithRefsTag_(ps.getParameter<edm::InputTag>("triggerEventWithRefs"))
{
  using namespace std;
  using namespace edm;

  cout << "HLTEventAnalyzerRAW configuration: " << endl
       << "   ProcessName = " << processName_ << endl
       << "   TriggerName = " << triggerName_ << endl
       << "   TriggerResultsTag = " << triggerResultsTag_.encode() << endl
       << "   TriggerEventWithRefsTag = " << triggerEventWithRefsTag_.encode() << endl;

}

HLTEventAnalyzerRAW::~HLTEventAnalyzerRAW()
{
}

//
// member functions
//
void
HLTEventAnalyzerRAW::beginRun(edm::Run const &, edm::EventSetup const&)
{
  using namespace std;
  using namespace edm;
  
  // HLT config does not change within runs!
  if (hltConfig_.init(processName_)) {
    // check if trigger name in (new) config
    if (triggerName_!="@") { // "@" means: analyze all triggers in config
      const unsigned int n(hltConfig_.size());
      const unsigned int triggerIndex(hltConfig_.triggerIndex(triggerName_));
      if (triggerIndex>=n) {
	cout << "HLTEventAnalyzerRAW::beginRun:"
	     << " TriggerName " << triggerName_ 
	     << " not available in (new) config!" << endl;
	cout << "Available TriggerNames are: " << endl;
	hltConfig_.dump("Triggers");
      }
    }
  } else {
    cout << "HLTEventAnalyzerRAW::beginRun:"
	 << " config extraction failure with process name "
	 << processName_ << endl;
  }
  return;

}

// ------------ method called to produce the data  ------------
void
HLTEventAnalyzerRAW::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace std;
  using namespace edm;
  
  cout << endl;

  // get event products
  iEvent.getByLabel(triggerResultsTag_,triggerResultsHandle_);
  if (!triggerResultsHandle_.isValid()) {
    cout << "HLTEventAnalyzerRAW::analyze: Error in getting TriggerResults product from Event!" << endl;
    return;
  }
  iEvent.getByLabel(triggerEventWithRefsTag_,triggerEventWithRefsHandle_);
  if (!triggerEventWithRefsHandle_.isValid()) {
    cout << "HLTEventAnalyzerRAW::analyze: Error in getting TriggerEventWithRefs product from Event!" << endl;
    return;
  }
  // sanity check
  assert(triggerResultsHandle_->size()==hltConfig_.size());
  
  // analyze this event for the triggers requested
  if (triggerName_=="@") {
    const unsigned int n(hltConfig_.size());
    for (unsigned int i=0; i!=n; ++i) {
      analyzeTrigger(hltConfig_.triggerName(i));
    }
  } else {
    analyzeTrigger(triggerName_);
  }

  cout << endl;
  
  return;
  
}

void HLTEventAnalyzerRAW::analyzeTrigger(const std::string& triggerName) {
  
  using namespace std;
  using namespace edm;
  using namespace reco;
  using namespace trigger;
  
  const unsigned int n(hltConfig_.size());
  const unsigned int triggerIndex(hltConfig_.triggerIndex(triggerName));
  
  // abort on invalid trigger name
  if (triggerIndex>=n) {
    cout << "HLTEventAnalyzerRAW::analyzeTrigger: path "
	 << triggerName << " - not found!" << endl;
    return;
  }
  
  cout << "HLTEventAnalyzerRAW::analyzeTrigger: path "
       << triggerName << " [" << triggerIndex << "]" << endl;
  // modules on this trigger path
  const unsigned int m(hltConfig_.size(triggerIndex));
  const vector<string>& moduleLabels(hltConfig_.moduleLabels(triggerIndex));

  // Results from TriggerResults product
  cout << " Trigger path status:"
       << " WasRun=" << triggerResultsHandle_->wasrun(triggerIndex)
       << " Accept=" << triggerResultsHandle_->accept(triggerIndex)
       << " Error =" << triggerResultsHandle_->error(triggerIndex)
       << endl;
  const unsigned int moduleIndex(triggerResultsHandle_->index(triggerIndex));
  cout << " Last active module - label/type: "
       << moduleLabels[moduleIndex] << "/" << hltConfig_.moduleType(moduleLabels[moduleIndex])
       << " [" << moduleIndex << " out of 0-" << (m-1) << " on this path]"
       << endl;
  assert (moduleIndex<m);

  // Results from TriggerEventWithRefs product
  photonIds_.clear();
  photonRefs_.clear();
  electronIds_.clear();
  electronRefs_.clear();
  muonIds_.clear();
  muonRefs_.clear();
  jetIds_.clear();
  jetRefs_.clear();
  compositeIds_.clear();
  compositeRefs_.clear();
  metIds_.clear();
  metRefs_.clear();
  htIds_.clear();
  htRefs_.clear();
  pixtrackIds_.clear();
  pixtrackRefs_.clear();
  l1emIds_.clear();
  l1emRefs_.clear();
  l1muonIds_.clear();
  l1muonRefs_.clear();
  l1jetIds_.clear();
  l1jetRefs_.clear();
  l1etmissIds_.clear();
  l1etmissRefs_.clear();

  // Attention: must look only for modules actually run in this path
  // for this event!
  for (unsigned int j=0; j<=moduleIndex; ++j) {
    const string& moduleLabel(moduleLabels[j]);
    const string  moduleType(hltConfig_.moduleType(moduleLabel));
    // check whether the module is packed up in TriggerEventWithRef product
    const unsigned int filterIndex(triggerEventWithRefsHandle_->filterIndex(InputTag(moduleLabel,"",processName_)));
    if (filterIndex<triggerEventWithRefsHandle_->size()) {
      cout << " Filter in slot " << j << " - label/type " << moduleLabel << "/" << moduleType << endl;
      cout << "  Accepted objects:" << endl;

      triggerEventWithRefsHandle_->getObjects(filterIndex,photonIds_,photonRefs_);
      const unsigned int nPhotons(photonIds_.size());
      if (nPhotons>0) {
	cout << "   Photons: " << nPhotons << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nPhotons; ++i) {
	  cout << "   " << i << " " << photonIds_[i]
	       << " " << photonRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,electronIds_,electronRefs_);
      const unsigned int nElectrons(electronIds_.size());
      if (nElectrons>0) {
	cout << "   Electrons: " << nElectrons << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nElectrons; ++i) {
	  cout << "   " << i << " " << electronIds_[i]
	       << " " << electronRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,muonIds_,muonRefs_);
      const unsigned int nMuons(muonIds_.size());
      if (nMuons>0) {
	cout << "   Muons: " << nMuons << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nMuons; ++i) {
	  cout << "   " << i << " " << muonIds_[i]
	       << " " << muonRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,jetIds_,jetRefs_);
      const unsigned int nJets(jetIds_.size());
      if (nJets>0) {
	cout << "   Jets: " << nJets << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nJets; ++i) {
	  cout << "   " << i << " " << jetIds_[i]
	       << " " << jetRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,compositeIds_,compositeRefs_);
      const unsigned int nComposites(compositeIds_.size());
      if (nComposites>0) {
	cout << "   Composites: " << nComposites << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nComposites; ++i) {
	  cout << "   " << i << " " << compositeIds_[i]
	       << " " << compositeRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,metIds_,metRefs_);
      const unsigned int nMETs(metIds_.size());
      if (nMETs>0) {
	cout << "   METs: " << nMETs << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nMETs; ++i) {
	  cout << "   " << i << " " << metIds_[i]
	       << " " << metRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,htIds_,htRefs_);
      const unsigned int nHTs(htIds_.size());
      if (nHTs>0) {
	cout << "   HTs: " << nHTs << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nHTs; ++i) {
	  cout << "   " << i << " " << htIds_[i]
	       << " " << htRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,pixtrackIds_,pixtrackRefs_);
      const unsigned int nPixTracks(pixtrackIds_.size());
      if (nPixTracks>0) {
	cout << "   PixTracks: " << nPixTracks << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nPixTracks; ++i) {
	  cout << "   " << i << " " << pixtrackIds_[i]
	       << " " << pixtrackRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,l1emIds_,l1emRefs_);
      const unsigned int nL1EM(l1emIds_.size());
      if (nL1EM>0) {
	cout << "   L1EM: " << nL1EM << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nL1EM; ++i) {
	  cout << "   " << i << " " << l1emIds_[i]
	       << " " << l1emRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,l1muonIds_,l1muonRefs_);
      const unsigned int nL1Muon(l1muonIds_.size());
      if (nL1Muon>0) {
	cout << "   L1Muon: " << nL1Muon << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nL1Muon; ++i) {
	  cout << "   " << i << " " << l1muonIds_[i]
	       << " " << l1muonRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,l1jetIds_,l1jetRefs_);
      const unsigned int nL1Jet(l1jetIds_.size());
      if (nL1Jet>0) {
	cout << "   L1Jet: " << nL1Jet << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nL1Jet; ++i) {
	  cout << "   " << i << " " << l1jetIds_[i]
	       << " " << l1jetRefs_[i]->pt()
	       << endl;
	}
      }

      triggerEventWithRefsHandle_->getObjects(filterIndex,l1etmissIds_,l1etmissRefs_);
      const unsigned int nL1EtMiss(l1etmissIds_.size());
      if (nL1EtMiss>0) {
	cout << "   L1EtMiss: " << nL1EtMiss << "  - the objects: # id pt" << endl;
	for (unsigned int i=0; i!=nL1EtMiss; ++i) {
	  cout << "   " << i << " " << l1etmissIds_[i]
	       << " " << l1etmissRefs_[i]->pt()
	       << endl;
	}
      }
    }
  }

  return;
}
