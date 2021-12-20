(return 0 2>/dev/null) && sourced=1 || sourced=0

if [ $sourced -eq 0 ]; then
  echo "This script should be sourced"
  exit 1
fi

tools_kit_path=$(readlink -f $(dirname ${BASH_SOURCE[0]}))

if [ ! -d $tools_kit_path/py3env ]; then
  (cd ${tools_kit_path}; ./create_py3env.sh)
fi

unset PYTHONPATH
source ${tools_kit_path}/py3env/bin/activate
