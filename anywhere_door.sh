core_script_dir='/repo/RosettaWorkshop/4._Tools/maintaining'
# Color escape sequences
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'


function anywhere_door {
  if [[ "$1" == "test" ]]; then
    proxy_test 
  elif [[ "$1" == "bench" ]]; then
    speedtest --progress=yes --selection-details
  else
    eval $(python ${core_script_dir}/anywhere_door_core "$1" "$2" "$3" "$4" "$5" "$6")
  fi
}

function proxy_test {
  local test_urls=("https://www.facebook.com" "https://www.google.com" "https://www.twitter.com" "https://www.instagram.com")
  local failed_tests=0

  for url in "${test_urls[@]}"; do
    #echo -e "Testing connection to: $url"
    if result=$( (time -p curl -sS --proxy "$https_proxy" --max-time 10 "$url") 2>&1 ); then
      timing=$(echo "$result" | awk '/^real/{print $2}')
      echo -e "Connection test to ${GREEN}$url: Success${RESET}  Time taken: $timing seconds"
    else
      echo -e "Connection test to ${RED}$url: Failed${RESET}"
      ((failed_tests++))
    fi
    echo  
  done
  

  if [[ $failed_tests -gt 0 ]]; then
    echo -e "\n${YELLOW}WARNING: Some test connections failed. It appears that the tested websites are blocked in your location.${RESET}"
    echo -e "\n${YELLOW}Consider configuring an upstream proxy server to bypass the restrictions.${RESET}"
  else
    echo -e "\n${GREEN}All test connections passed successfully. The tested websites are accessible.${RESET}"
  fi
}

