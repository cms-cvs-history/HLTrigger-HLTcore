#!/bin/tcsh

echo
echo "Getting test beam file (big!)"
echo
wget http://cern.ch/jmans/cms/HTB_011609.root

#echo
#echo "Getting simulated file"
#echo
#wget http://cmsdoc.cern.ch/cms/data/Validation/Simulation/pi100GeV_detsim_digi_pileup.50evt.root

echo
echo "Making CaloTowers with cmsRun"
echo
eval `scramv1 runtime -csh`
rehash 
cmsRun -p make_CaloTowers_HB_HO.cfg

echo
echo "Making Jets and running simple jet HLTs"
echo
eval `scramv1 runtime -csh`
rehash 
cmsRun -p example.cfg
