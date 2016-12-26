#!/bin/bash

for i in `ls originals`; do
  echo Elaboro $i
  python tools/converter.py originals/$i $i.graphml
  python tools/converter.py originals/$i $i.edl edl
done

