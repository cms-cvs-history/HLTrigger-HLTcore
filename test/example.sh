#!/bin/sh

cp $CMSSW_RELEASE_BASE/src/RecoJets/JetProducers/test/CaloTowers_HB_HO.root .

cmsRun -p example.cfg
