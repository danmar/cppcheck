#!/bin/sh

# TODO: use temporary folder
# TODO: print the name of the directory used for data
# TODO: pass all parameters on to the script execution

set -ex

echo 'Thank you for providing resources and supporting the Cppcheck project!'

if [ ! -d cppcheck ]; then
  # clone the 'main' branch of the official repository
  git clone -b main --depth 1 https://github.com/danmar/cppcheck.git
else
  # update to the latest source to make sure we run the latest client
  git pull
fi

cd cppcheck

# create a virtual environment to install the required Python dependencies (or update it)
python -m venv --upgrade --upgrade-deps .env

# install or upgrade the Python dependencies
.env/bin/pip install -r tools/donate-cpu-requirements.txt --upgrade

# run the client for a limited amount of packages (adjust -j to the amount of cores to use)
.env/bin/python tools/donate-cpu.py --max-packages=100 -j1

# continue with the latest script
bash -c "$(wget -O - https://raw.githubusercontent.com/danmar/cppcheck/refs/heads/main/tools/donate.sh)"