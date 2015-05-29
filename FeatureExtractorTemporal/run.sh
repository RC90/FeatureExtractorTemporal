#!/bin/bash

#### Cluster ####

# ROOTPATH=/panfs/panasas01.panfs.cluster/psxta4/Documents/Databases
# XMLPATH=/panfs/panasas01.panfs.cluster/psxta4/Documents/Databases/CK-Faces.xml
# OUTPUTPATH=/panfs/panasas01.panfs.cluster/psxta4/Documents/Databases/CK-Faces-Features-LGBPTOP

#### Cluster ####

#### Local ####

export DYLD_LIBRARY_PATH="/Users/timur/Documents/Boost/boost_1_57_0/stage/lib":$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH="/Users/timur/Documents/OpenCV/lib":$DYLD_LIBRARY_PATH

ROOTPATH=/Users/timur/Documents/DatabasesReloaded
XMLPATH=/Users/timur/Documents/Research/ResearchReloaded/Code/Metadata/Collections/CK-Faces.xml
OUTPUTPATH=/Users/timur/Documents/DatabasesReloaded/CK-Faces-Features-LGBPTOP

#### Local ####

rm -rf $OUTPUTPATH
mkdir $OUTPUTPATH
# clear

./FeatureExtractorTemporal $ROOTPATH $XMLPATH $OUTPUTPATH