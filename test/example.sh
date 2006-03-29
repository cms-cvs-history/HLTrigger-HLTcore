#!/bin/sh

echo "Getting test beam file (big!)"
#wget http://cern.ch/jmans/cms/HTB_011609.root

#echo "Getting simulated file"
#wget http://cmsdoc.cern.ch/cms/data/Validation/Simulation/pi100GeV_detsim_digi_pileup.50evt.root

echo "Making CaloTowers with cmsRun"

cmsRun -p make_CaloTowers_HB_HO.cfg

echo "Making Jets and running simple jet HLTs"

cmsRun -p example.cfg
