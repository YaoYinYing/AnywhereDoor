
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
  time_executable_path=$(which time)
fi

# Color escape sequences
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'


function anywhere_door {
  if [[ "$1" == "test" ]]; then
    pass_test=()
    if [[ "$2" == "all" ]]; then 
      max_index=$(anywhere_door use |grep  '[[:digit:]]. ' |wc -l)
      pass_test=()
      for i in $(seq 1 $max_index); do 
        echo "Testing $i ..."
        anywhere_door use $i;
        proxy_test fast $i
      done

      echo -e "Testing Passed: ${GREEN}${pass_test[*]}${RESET}"

    else
      proxy_test 
    fi
    
  elif [[ "$1" == "bench" ]]; then
    if ! command speedtest; then echo "Please install the speedtest command line tool first."; exit 1; fi
    
    speedtest --progress=yes --selection-details
  else
    eval $(python3 ${ANYWHERE_DOOR_DIR}/anywhere_door_core.py "$1" "$2" "$3" "$4" "$5" "$6")
  fi
}

function proxy_test {
  local mode=$1
  local proxy_index=$2
  if [[ ! $mode == "fast" ]];then
    local test_urls=("https://www.facebook.com" "https://www.google.com" "https://www.twitter.com" "https://www.instagram.com")
  else
    local test_urls=("https://www.google.com")
  fi
  local failed_tests=0
  local passed=true

  for url in "${test_urls[@]}"; do
    if result=$( ($time_executable_path -p curl -sS --proxy "$https_proxy" --max-time 10 "$url") 2>&1 ); then
      timing=$(echo "$result" | awk '/^real/{print $2}')
      echo -e "Connection test to ${GREEN}$url: Success${RESET}  Time taken: $timing seconds"
    else
      echo -e "Connection test to ${RED}$url: Failed${RESET}"
      ((failed_tests++))
      local passed=false
    fi
    
  done

  if $passed;then 
    pass_test+=($proxy_index)
  fi
  

  if [[ ! $mode == "fast" ]]; then
    if [[ $failed_tests -gt 0 ]]; then
      echo -e "\n${YELLOW}WARNING: Some test connections failed. It appears that the tested websites are blocked in your location.${RESET}"
      echo -e "\n${YELLOW}Consider configuring an upstream proxy server to bypass the restrictions.${RESET}"
    else
      echo -e "\n${GREEN}All test connections passed successfully. The tested websites are accessible.${RESET}"
    fi
  fi

  
}

