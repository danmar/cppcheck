@echo off

pushd %~dp0

if exist online-help.qhc del online-help.qhc
if exist online-help.qch del online-help.qch

qcollectiongenerator online-help.qhcp -o online-help.qhc

popd
