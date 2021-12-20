#!/bin/bash

unset PYTHONPATH
PATH=$PATH:/usr/bin

if [ ! -f py3env/bin/activate ]; then
  echo "INFO: creating python3 venv..."
  python3 -m venv py3env
fi

if [ -f py3env/bin/activate ]; then
  source py3env/bin/activate
  if [ ! -f py3env/bin/sphinx-build ]; then
    echo "INFO: pip installing in venv..."
    pip install -U pip
    pip install wheel
    pip install -r requirements.txt
  fi
else
  echo "ERROR: venv activate script does not exist. Skipping pip install"
fi
