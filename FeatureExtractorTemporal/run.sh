#!/bin/bash

#### Cluster ####

# DATAPATH=/home/psxta4/Documents/DatabasesReloaded/MMI-Faces-Aligned
# XMLPATH=/home/psxta4/Documents/DatabasesReloaded/MMI.xml
# OUTPUTPATH=/home/psxta4/Documents/DatabasesReloaded/MMI-Features-LGBPTOP-Aligned
# EXTENSION=""

#### Cluster ####

#### Local ####

DATAPATH=/Users/timur/Documents/Databases/McMaster-Faces
XMLPATH=/Users/timur/Documents/MatlabTools/Timur/Metadata/Collections/McMaster.xml
OUTPUTPATH=/Users/timur/Documents/Databases/McMaster-Features-LBP-TOP
FTYPE="LBP-TOP"
EXTENSION=".bmp"

#### Local ####

rm -rf $OUTPUTPATH
mkdir $OUTPUTPATH
clear

./FeatureExtractor $DATAPATH $XMLPATH $OUTPUTPATH $FTYPE $EXTENSION