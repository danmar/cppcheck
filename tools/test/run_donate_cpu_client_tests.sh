#!/bin/bash
#
# Script for verifying that the donate-cpu.py script works under different circumstances (Python
# version, parameters, ...).
# Good for checking if everything still works before committing changes to donate-cpu.py.

# Detect and report errors
errors_occurred=0

error_occurred() {
  echo "#######################################################################"
  echo "ERROR: On line $(caller), errorcode: $?" >&2
  echo "#######################################################################"
  errors_occurred=1
}

trap error_occurred ERR

# Run tests
client_script=../donate-cpu.py
test_packages=(
ftp://ftp.se.debian.org/debian/pool/main/0/0xffff/0xffff_0.8.orig.tar.gz
ftp://ftp.se.debian.org/debian/pool/main/a/actionaz/actionaz_3.8.0.orig.tar.gz
)

for python_exec in "python3"
do
    for test_package in "${test_packages[@]}"
    do
        echo "Testing package ${test_package} with ${python_exec} ..."
        ${python_exec} ${client_script} --package=${test_package}
        ${python_exec} ${client_script} --package=${test_package} -j1
        ${python_exec} ${client_script} --package=${test_package} -j2
        ${python_exec} ${client_script} --package=${test_package} --bandwidth-limit=250k
        ${python_exec} ${client_script} --package=${test_package} -j2 --bandwidth-limit=0.5M
        ${python_exec} ${client_script} --package=${test_package} --max-packages=1
    done
done

# Report result and exit accordingly
if [ $errors_occurred -eq 0 ]; then
  echo "All tests successfully finished."
  exit 0
else
  echo "#######################################################################"
  echo "ERRORS OCCURRED! See error messages above for details."
  echo "#######################################################################"
  exit 1
fi
