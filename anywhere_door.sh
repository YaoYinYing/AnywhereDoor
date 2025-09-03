
# https://stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script

SHELL_TYPE=$(basename  $SHELL)
if [[ "$SHELL_TYPE" == "bash" ]]; then
  core_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
elif [[ "$SHELL_TYPE" == "zsh" ]]; then
  core_script_dir=${0:A:h}
fi
export ANYWHERE_DOOR_DIR=$core_script_dir

PLATFORM=$(uname -s)

# Color escape sequences
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
RESET='\033[0m'


function anywhere_door {
  # direct run without eval
  if [[ "$1" == "test" || "$1" == "dns" ||  "$1" == "leak" ||  "$1" == "help" ||  "$1" == "?" ||  "$1" == "show" ||  "$1" == "list" ||  "$1" == "ls" ||  "$1" == "git" || "$1" == "docker_daemon" || ( "$1" == "use" && "$2" == "" ) ]]; then
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
      wget -O /dev/null 'https://speedtest.yaoyy.moe/';
    fi

  elif [[ "$1" == "curl" ]];then
    if ! command -v curl;then
      echo "Please install the curl first.";
    else 
      curl -o /dev/null 'https://speedtest.yaoyy.moe/';
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



# Completion script for anywhere_door

_anywhere_door_completions()
{
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    
    # Define the options for anywhere_door
    opts="on off show list ls test bench wget curl whereami use upgrade dns leak help ?"

    # Define hints for each option
    declare -A hints=(
        [on]="Activate Anywhere Door proxy"
        [off]="Deactivate Anywhere Door proxy"
        [config]="Configure a new proxy"
        [show]="Show the current proxy configurations"
        [list]="List all predefined proxies"
        [ls]="List all predefined proxies"
        [test]="Perform a connection test to check proxy accessibility"
        [bench]="Perform a speed test using the Ookla speed test tool"
        [wget]="Perform a speed test using wget"
        [curl]="Perform a speed test using curl"
        [whereami]="Check your current IP location"
        [use]="Use a specific proxy from the list"
        [upgrade]="Upgrade to the latest code version"
        [dns]="Perform a DNS leak test"
        [leak]="Perform a DNS leak test"
        [help]="Show help messages for commands"
        [docker_daemon]="Generate HTTP proxy setup for docker daemon"
        [?]="Show help messages for commands"
    )

    # Define hints for help subcommands
    declare -A help_hints=(
        [test]="Help for testing proxy connectivity"
        [use]="Help for using a specific proxy"
        [config]="Help for configuring a proxy"
        [show]="Help for showing current proxy settings"
    )

    # If we're at the base command, suggest all main options
    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )

        # Show hints if the user presses tab with no input
        if [[ -z "${cur}" ]]; then
            echo -e "\nOptions:"
            for opt in ${opts}; do
                printf "  %-10s : %s\n" "${opt}" "${hints[$opt]}"
            done
        fi

        return 0
    fi

    case "${prev}" in
        anywhere_door)
            COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
            return 0
            ;;
        use)
            # Complete with proxy labels or indices
            local proxies="Mihomo_ Private_"
            local indices=$(seq 0 22)
            COMPREPLY=( $(compgen -W "${proxies} ${indices}" -- ${cur}) )
            return 0
            ;;
        show)
            # Complete with proxy types
            local types="http https all"
            COMPREPLY=( $(compgen -W "${types}" -- ${cur}) )
            return 0
            ;;
        test)
            # Complete with test options
            local test_opts="all full"
            COMPREPLY=( $(compgen -W "${test_opts}" -- ${cur}) )
            return 0
            ;;
        ?|help)
            # Complete with specific help topics
            local help_opts="test use config show"
            COMPREPLY=( $(compgen -W "${help_opts}" -- ${cur}) )


            # Show hints for help topics
            if [[ -z "${cur}" ]]; then
                echo -e "\nHelp Topics:"
                for topic in ${help_opts}; do
                    printf "  %-10s : %s\n" "${topic}" "${help_hints[$topic]}"
                done
            fi
            
            return 0
            ;;
        *)
            ;;
    esac
}

complete -F _anywhere_door_completions anywhere_door
