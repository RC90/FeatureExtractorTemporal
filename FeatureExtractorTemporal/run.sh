#!/bin/bash

#### Cluster ####

# DATAPATH=/home/psxta4/Documents/DatabasesReloaded/MMI-Faces-Aligned
# XMLPATH=/home/psxta4/Documents/DatabasesReloaded/MMI.xml
# OUTPUTPATH=/home/psxta4/Documents/DatabasesReloaded/MMI-Features-LGBPTOP-Aligned
# EXTENSION=""

#### Cluster ####

#### Local ####

export DYLD_LIBRARY_PATH="/Users/timur/Documents/Boost/boost_1_57_0/stage/lib":$DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH="/Users/timur/Documents/OpenCV/lib":$DYLD_LIBRARY_PATH

DATAPATH=/Users/timur/Documents/DatabasesReloaded/MMI-Faces-Aligned
XMLPATH=/Users/timur/Documents/Research/ResearchReloaded/Code/Metadata/Collections/MMI.xml
OUTPUTPATH=/Users/timur/Documents/DatabasesReloaded/MMI-Features-LGBPTOP-Aligned
EXTENSION=""

#### Local ####

rm -rf $OUTPUTPATH
mkdir $OUTPUTPATH
# clear

./FeatureExtractorTemporal $DATAPATH $XMLPATH $OUTPUTPATH $EXTENSION