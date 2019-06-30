#!/bin/bash

# Package all projects into timestamped .maxzip.
# Run in the directory containing this script.

now=`date +%Y%m%d-%H%M%S`

for f in $(find . -name '*.maxproj'); do
    stem=${f%/*}
    stem=${stem#./}
    echo $stem ...
    rm -f ${stem}_*.maxtest.maxzip
    zip -r ${stem}_${now}.maxtest.maxzip ${stem}
done
