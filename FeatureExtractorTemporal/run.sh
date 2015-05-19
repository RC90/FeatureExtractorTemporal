!#/bin/bash

INPUT  := ~/Documents/Databases/CK-InputPerSession-Aligned
OUTPUT := ~/Documents/Databases/CK-LGBPTOP-Features-Aligned

./FeatureExtractorTemporal $(INPUT) $(OUTPUT)