
# https://stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script

SHELL_TYPE=$(basename  $SHELL)
if [[ "$SHELL_TYPE" == "bash" ]]; then
  core_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
elif [[ "$SHELL_TYPE" == "zsh" ]]; then
  core_script_dir=${0:A:h}
fi
export ANYWHERE_DOOR_DIR=$core_script_dir

PLATFORM=$(uname -s)

if [[ "$PLATFORM" == "Darwin" ]];then
  time_executable_path='/usr/bin/time'
else
  time_executable_path=time
fi

# Color escape sequences
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'


function anywhere_door {
  # direct run without eval
  if [[ "$1" == "test" || "$1" == "dns" ||  "$1" == "leak" ||  "$1" == "help" ||  "$1" == "?" ||  "$1" == "show" ||  "$1" == "list" ||  "$1" == "ls" ||  "$1" == "git"|| ( "$1" == "use" && "$2" == "" ) ]]; then
      python3 ${ANYWHERE_DOOR_DIR}/anywhere_door_core.py "$1" "$2" "$3" "$4" "$5" "$6"
  
  # other shell commands
  elif [[ "$1" == "bench" ]]; then
    if ! command -v speedtest; then
      echo "Please install the speedtest command line tool first.";
    else
      speedtest --progress=yes --selection-details
    fi

  elif [[ "$1" == "wget" ]];then
    if ! command -v wget;then
      echo "Please install the wget first.";
    else 
      wget -O /dev/null 'https://speedtest.yaoyy-hi.workers.dev/';
    fi

  elif [[ "$1" == "curl" ]];then
    if ! command -v curl;then
      echo "Please install the curl first.";
    else 
      curl -o /dev/null 'https://speedtest.yaoyy-hi.workers.dev/';
    fi

  elif [[ "$1" == "whereami" ]];then
    if ! command -v curl;then
      echo "Please install the curl first.";
    else 
      curl 'https://ipinfo.io/';
    fi
  
  elif [[ "$1" == "upgrade" ]]; then
    pushd $ANYWHERE_DOOR_DIR;
      git stash; git pull;
    popd

  # eval for the python script, for proxy injection to shell env.
  else
    eval $(python3 ${ANYWHERE_DOOR_DIR}/anywhere_door_core.py "$1" "$2" "$3" "$4" "$5" "$6")
  fi
}

