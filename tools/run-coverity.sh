#!/bin/bash
PATH=$PATH:/home/danielmarjamaki/cov-analysis-linux64-2021.12.1/bin
cd /home/danielmarjamaki/cppcheck-devinfo || exit 1
git pull

echo Analyze
make clean
nice cov-build --dir cov-int make

echo Compressing
tar czvf cppcheck.tgz cov-int

echo Upload
curl --insecure \
  --form token=e74RRnWR6BVsn5LKdclfcA \
  --form email=daniel.marjamaki@gmail.com \
  --form file=@cppcheck.tgz \
  --form version=$(git log -1 --format=oneline | sed -r 's/([a-f0-9]{7}).*/\1/') \
  --form description="Development" \
  https://scan.coverity.com/builds?project=cppcheck

echo Done
rm -rf cov-int

