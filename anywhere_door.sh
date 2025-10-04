# ANYWHERE DOOR

# version info
export ANYWHERE_DOOR_DIR_VERSION=1.1.0

# Selfcheck for the installation dir
# See: https://stackoverflow.com/questions/59895/how-do-i-get-the-directory-where-a-bash-script-is-located-from-within-the-script

SHELL_TYPE=$(basename  $SHELL)
if [[ "$SHELL_TYPE" == "bash" ]]; then
  core_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
elif [[ "$SHELL_TYPE" == "zsh" ]]; then
  core_script_dir=${0:A:h}
fi
export ANYWHERE_DOOR_DIR=$core_script_dir

function _fetch_anywhere_door_version {
  CUR_PWD=$PWD
  cd $ANYWHERE_DOOR_DIR
  git log -1 |grep commit
  cd $CUR_PWD
}

export ANYWHERE_DOOR_DIR_COMMIT_ID_SESSION=$(_fetch_anywhere_door_version)
# read color escapes
eval $(cat ${ANYWHERE_DOOR_DIR}/color_escape.py |tr -d ' ')



# main function
function anywhere_door {
  # direct run without eval
  if [[ "$1" == "test" || "$1" == "dns" ||  "$1" == "leak" ||  "$1" == "help" ||  "$1" == "?" ||  "$1" == "show" ||  "$1" == "list" ||  "$1" == "ls" ||  "$1" == "git" || "$1" == "docker_daemon" || "$1" == "export" || ( "$1" == "use" && "$2" == "" ) ]]; then
      python3 ${ANYWHERE_DOOR_DIR}/anywhere_door_core.py "$1" "$2" "$3" "$4" "$5" "$6"
  
  # other shell commands
  elif [[ "$1" == "version" ]]; then
    echo ""
    echo -e "${RED_BG}${YELLOW}${BOLD}Anywhere Door${RESET} (${ITALIC}${MAGENTA_BG}${ANYWHERE_DOOR_DIR_COMMIT_ID_SESSION}${RESET}) v.${CYAN_BG}${LIGHT_GREEN}${BLINK}${ANYWHERE_DOOR_DIR_VERSION}${RESET}"
    echo "Copyright (C) 2025 YINYING YAO."
    echo "The right of final interpretation is reserved."
    echo ""
    echo -e "Installed At: \$ANYWHERE_DOOR_DIR=${BLUE}${RED_BG}${UNDERLINE}${ANYWHERE_DOOR_DIR}${RESET}"
    echo ""

  elif [[ "$1" == "gost" ]]; then
    if ! command -v gost; then
      echo "Gost not found. Please see: https://gost.run/en/"
    elif [[ "$2" == "clean" ]]; then
      echo "Remaining gost process will be cleaned"
      ps aux |grep gost | awk '{print $2; system("kill " $2)}' >/dev/null 2>&1
      echo Done.
    else
      if [[ ! -z "$GOST_PID" ]]; then 
        kill $GOST_PID >/dev/null 2>&1; 
        echo "Stop background GOST at ${GOST_PID}"; 
        unset GOST_PID; 
        unset GOST_PORT; 
        # recover to anywhere door settings
        anywhere_door $ANYWHEREDOOR_CURRENT_GATE; 
      fi

      

      if [[ "$2" == "off" ]]; then
        anywhere_door gost clean
        anywhere_door $ANYWHEREDOOR_CURRENT_GATE
      else
        # on / port-id
        if [[ $(anywhere_door show) =~ 'inactive' ]];then
          echo Anywheredoor must be active befor run gost.
        else
          if [[ ! -z "$2" && "$2" =~ ^[0-9]+$ ]]; then GOST_PORT=$2; else GOST_PORT="63322" ; fi

          gost -L=:${GOST_PORT} -F=$(anywhere_door show all | cut -b 12-) >/dev/null 2>&1 &
          export GOST_PID=$!
          export http_proxy=http://127.0.0.1:${GOST_PORT}
          export https_proxy=http://127.0.0.1:${GOST_PORT}
          export all_proxy=http://127.0.0.1:${GOST_PORT}
          echo GOST running at $GOST_PID 
        fi
      fi
    fi 
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
  
  elif [[ "$1" == "ipq" ]];then
    echo "Running in progress..."
    $SHELL ${ANYWHERE_DOOR_DIR}/tools/IPQuality/ip.sh -x $(anywhere_door show http |awk -F= '{print $2}')

  elif [[ "$1" == "netq" ]];then
     echo "Running in progress..."
    $SHELL ${ANYWHERE_DOOR_DIR}/tools/NetQuality/net.sh
  
  elif [[ "$1" == "upgrade" ]]; then
    pushd $ANYWHERE_DOOR_DIR;
      git stash;git fetch --all; git reset --hard origin/main; git pull;
    popd
  
  elif [[ "$1" == "refresh" ]]; then
    source $ANYWHERE_DOOR_DIR/anywhere_door.sh;

  # eval for the python script, for proxy injection to shell env.
  else
    if [[ "$1" == "use" || "$1" == "config" ]];then export ANYWHEREDOOR_CURRENT_GATE="$1 $2 $3 $4 $5 $6"; fi
    eval $(python3 ${ANYWHERE_DOOR_DIR}/anywhere_door_core.py "$1" "$2" "$3" "$4" "$5" "$6")
    if [[ "$1" == "off" ]];then 
      unset ANYWHEREDOOR_CURRENT_GATE; 
      if [[ ! -z "$GOST_PID" ]]; then anywhere_door gost off; fi
    fi
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
    _opts=$(anywhere_door help |grep '^ ' |awk -F: '{split($1,args, "/"); for(arg in args){_arg=args[arg];gsub(/[ \t]+/,"",_arg);printf "%s ",_arg;}}')
    opts=$_opts

    # Primary help completions
    _hints=$(anywhere_door help |grep '^ ' |awk -F: '{split($1,args, "/"); for(arg in args){_arg=args[arg];gsub(/[ \t]+/,"",_arg);_exp=$2;gsub(/^[ \t]+/,"", _exp);printf "[%s]=\"%s\"\n",_arg, _exp}}')
    eval "declare -A hints=(${_hints})"


    # Secondary help completions
    declare -A help_hints=(
        [test]="Help for testing proxy connectivity"
        [use]="Help for using a specific proxy"
        [config]="Help for configuring a proxy"
        [show]="Help for showing current proxy settings"
        [list]="List all predefined proxies"
        [ls]="List all predefined proxies"
        [gost]='Wrap Socks to HTTP by GOST'
        [docker_daemon]='Generate Docker daemon proxy based on current door.'

    )

    # Primary level of completions 
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
        # Complete primary level
        anywhere_door)
            COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
            return 0
            ;;

        # Complete the secondary levels
        use)
            # Complete with proxy labels or indices
            local proxies="$(anywhere_door list all |grep '^[[:digit:]]' |awk '{print substr($2,6, length($2)-10)}')"
            local indices=$(seq 0 $(echo ${proxies} | wc --words))
            COMPREPLY=( $(compgen -W "${proxies} ${indices}" -- ${cur}) )
            return 0
            ;;
        show)
            # Complete with proxy types
            local types="http https all"
            COMPREPLY=( $(compgen -W "${types}" -- ${cur}) )
            return 0
            ;;
        list|ls)
            local types="all"
            COMPREPLY=( $(compgen -W "${types}" -- ${cur}) )
            return 0
            ;;
        gost)
            # Complete with test options
            local test_opts="on off clean"
            COMPREPLY=( $(compgen -W "${test_opts}" -- ${cur}) )
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
            local help_opts="test use config show gost docker_daemon"
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
