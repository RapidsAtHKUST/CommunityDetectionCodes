#!/bin/bash
#PBS -l walltime=24:00:00 
#PBS -l mem=60Gb 
#PBS -l ncpus=4 
#PBS -m abe
# Script to run linegraphcreator (C++) on ALTIX
# 
# compile using
#module load intel-suite
#icpc -mcmodel=large -i-dynamic -o linegraphcreator TseGraph.cpp main.cpp 

module load intel-suite
export NAMEROOT=IC090729stempt
export NAMEEND=inputEL.dat
#export OUTPUTNAME=$NAMEROOT`eval date +%y%m%d`".tgz"
#export ZIPNAME=$NAMEROOT`eval date +%y%m%d`".tgz"
#mkdir -v ${TMPDIR}/input
#mkdir -v ${TMPDIR}/output
cp -v ${HOME}/linegraphcreator/input/${NAMEROOT}${NAMEEND} ${TMPDIR}
echo Running ${NAMEROOT}
${HOME}/linegraphcreator/linegraphcreator  -i ${TMPDIR}/${NAMEROOT}${NAMEEND} -o ${TMPDIR}/${NAMEROOT}WLGoutputEL.dat -t 2
#gzip -vc ${TMPDIR}/output/* 
#tar -zcvf ${ZIPNAME} ${TMPDIR}/output/* 
#echo unpack using tar -xzvf ${ZIPNAME}.tgz
cp -v ${TMPDIR}/${NAMEROOT}WLG* ${HOME}/linegraphcreator/output/
