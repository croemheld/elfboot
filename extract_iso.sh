#!/bin/bash
# Description:
#   Extracts all contents of an ISO to the given output directory
# Requires:
#   Packages: genisoimage

ISO=$1
OUTDIR=${2:-"./out"}

usage() {
  echo "$0 <iso> [outdir:./out]"
}

[ -z $ISO ] && { usage; exit 1; }
[ -r $ISO ] || { echo "Cannot read ISO ${ISO}"; exit 1; }
[ -d $OUTDIR ] || mkdir -p $OUTDIR

isoinfo -f -R -i $ISO | while read line; do
  d=$(dirname $line)
  od=${OUTDIR}${d}
  [ -f $od ] && rm -f $od
  [ -d $od ] || mkdir -p $od
  isoinfo -R -i $ISO -x $line > ${OUTDIR}${line}
done

echo "Extracted ${ISO} to ${OUTDIR}"