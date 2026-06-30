# ANYWHERE DOOR

# version info
export ANYWHERE_DOOR_DIR_VERSION=2.0.0-alpha

# Selfcheck for the installation dir
SHELL_TYPE=$(basename $SHELL)
if [[ "$SHELL_TYPE" == "bash" ]]; then
  core_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
elif [[ "$SHELL_TYPE" == "zsh" ]]; then
  core_script_dir=${0:A:h}
fi
export ANYWHERE_DOOR_DIR=$core_script_dir

function _fetch_anywhere_door_version {
  CUR_PWD=$PWD
  cd $ANYWHERE_DOOR_DIR
  git log -1 | grep commit 2>/dev/null || echo "commit unknown"
  cd $CUR_PWD
}

export ANYWHERE_DOOR_DIR_COMMIT_ID_SESSION=$(_fetch_anywhere_door_version)

# --- ANSI color escapes (inlined from color_escape.py, for shell-only commands) ---
BLACK="\033[0;30m"
GREEN="\033[0;32m"
BROWN="\033[0;33m"
BLUE="\033[0;34m"
PURPLE="\033[0;35m"
LIGHT_GRAY="\033[0;37m"
DARK_GRAY="\033[1;30m"
LIGHT_RED="\033[1;31m"
LIGHT_GREEN="\033[1;32m"
YELLOW="\033[1;33m"
LIGHT_BLUE="\033[1;34m"
LIGHT_PURPLE="\033[1;35m"
LIGHT_CYAN="\033[1;36m"
LIGHT_WHITE="\033[1;37m"
BOLD="\033[1m"
FAINT="\033[2m"
ITALIC="\033[3m"
UNDERLINE="\033[4m"
BLINK="\033[5m"
NEGATIVE="\033[7m"
CROSSED="\033[9m"
RESET="\033[0m"
CYAN="\033[0;36m"
CYAN_BG="\033[0;44m"
RED="\033[0;31m"
RED_BG="\033[0;41m"
MAGENTA="\033[0;35m"
MAGENTA_BG="\033[0;45m"

# --- Locate the C binary ---
function _find_anywhere_door_bin {
  # Search locations in priority order
  local locations=(
    "${ANYWHERE_DOOR_DIR}/src/anywheredoor/build/anywhere_door"
    "${ANYWHERE_DOOR_DIR}/build/anywhere_door"
    "/usr/local/bin/anywhere_door"
  )

  # Also check PATH
  if command -v anywhere_door_bin &>/dev/null; then
    echo "anywhere_door_bin"
    return
  fi

  for loc in "${locations[@]}"; do
    if [[ -x "$loc" ]]; then
      echo "$loc"
      return
    fi
  done

  # Fallback: try to find it relative to the script
  echo "${ANYWHERE_DOOR_DIR}/src/anywheredoor/build/anywhere_door"
}

ANYWHERE_DOOR_BIN=$(_find_anywhere_door_bin)

# --- main function ---
function anywhere_door {
  # Commands handled entirely in shell (no C binary needed)
  if [[ "$1" == "version" ]]; then
    echo ""
    echo -e "${RED_BG}${YELLOW}${BOLD}Anywhere Door${RESET} (${ITALIC}${MAGENTA_BG}${ANYWHERE_DOOR_DIR_COMMIT_ID_SESSION}${RESET}) v.${CYAN_BG}${LIGHT_GREEN}${BLINK}${ANYWHERE_DOOR_DIR_VERSION}${RESET}"
    echo "Copyright (C) 2025 YINYING YAO."
    echo "The right of final interpretation is reserved."
    echo ""
    echo -e "Installed At: \$ANYWHERE_DOOR_DIR=${BLUE}${RED_BG}${UNDERLINE}${ANYWHERE_DOOR_DIR}${RESET}"
    if [[ -x "$ANYWHERE_DOOR_BIN" ]]; then
      echo -e "Binary: ${ANYWHERE_DOOR_BIN} ($(${ANYWHERE_DOOR_BIN} version 2>/dev/null))"
    fi
    echo ""
    return
  fi

  # GOST management (pure shell)
  if [[ "$1" == "gost" ]]; then
    if ! command -v gost; then
      echo "Gost not found. Please see: https://gost.run/en/"
      return
    fi

    function gost_precheck() {
      if [[ $(anywhere_door show) =~ 'inactive' ]];then
        echo Anywheredoor must be active before running gost.
        return
      fi
    }

    function gost_restore() {
      if [[ ! -z "$ANYWHEREDOOR_CURRENT_GATE" ]];then
          anywhere_door $ANYWHEREDOOR_CURRENT_GATE;
      fi
    }

    if [[ "$2" == "clean" ]]; then
      gost_precheck
      echo "Remaining gost process will be cleaned"
      ps aux | grep gost | awk '{print $2; system("kill " $2)}' >/dev/null 2>&1
      echo Done.
    elif [[ "$2" == "off" ]]; then
        gost_precheck
        anywhere_door gost clean
        gost_restore
    elif [[ "$2" == "on" || "$2" == '' ]]; then
      gost_precheck
      if [[ ! -z "$3" && "$3" =~ ^[0-9]+$ ]]; then GOST_PORT=$3; else GOST_PORT="63322"; fi
      echo Using gost port: $GOST_PORT
      gost -L=:${GOST_PORT} -F=$(anywhere_door show all | cut -b 12-) >/dev/null 2>&1 &
      export GOST_PID=$!
      export http_proxy=http://127.0.0.1:${GOST_PORT}
      export https_proxy=http://127.0.0.1:${GOST_PORT}
      export all_proxy=http://127.0.0.1:${GOST_PORT}
      echo GOST running at $GOST_PID
    elif [[ "$2" == "stop" ]]; then
      gost_precheck
      if [[ ! -z "$GOST_PID" ]]; then
        kill $GOST_PID >/dev/null 2>&1;
        echo "Stop background GOST at ${GOST_PID}";
        unset GOST_PID;
        unset GOST_PORT;
        gost_restore
      else
        echo "GOST is not running"
      fi
    elif [[ "$2" == "update" ]]; then
      gost_precheck
      anywhere_door gost clean
      gost_restore
      anywhere_door gost on
    else
      echo Unknown command for anywhere_door gost: $2 $3
    fi
    return
  fi

  # Other shell-only commands
  if [[ "$1" == "bench" ]]; then
    if ! command -v speedtest; then
      echo "Please install the speedtest command line tool first.";
    else
      speedtest --progress=yes --selection-details
    fi
    return
  fi

  if [[ "$1" == "wget" ]];then
    if ! command -v wget;then
      echo "Please install wget first.";
    else
      wget -O /dev/null 'https://speedtest.yaoyy.moe/';
    fi
    return
  fi

  if [[ "$1" == "curl" ]];then
    if ! command -v curl;then
      echo "Please install curl first.";
    else
      curl -o /dev/null 'https://speedtest.yaoyy.moe/';
    fi
    return
  fi

  if [[ "$1" == "whereami" ]];then
    if ! command -v curl;then
      echo "Please install curl first.";
    else
      curl 'https://ipinfo.io/';
    fi
    return
  fi

  if [[ "$1" == "ipq" ]];then
    echo "Running in progress..."
    $SHELL ${ANYWHERE_DOOR_DIR}/tools/IPQuality/ip.sh -x $(anywhere_door show http | awk -F= '{print $2}')
    return
  fi

  if [[ "$1" == "netq" ]];then
    echo "Running in progress..."
    $SHELL ${ANYWHERE_DOOR_DIR}/tools/NetQuality/net.sh
    return
  fi

  if [[ "$1" == "upgrade" ]]; then
    pushd $ANYWHERE_DOOR_DIR;
      git stash; git fetch --all; git reset --hard origin/main; git pull;
    popd
    return
  fi

  if [[ "$1" == "refresh" ]]; then
    source $ANYWHERE_DOOR_DIR/anywhere_door.sh;
    return
  fi

  # --- Commands dispatched to C binary ---

  # Ensure binary exists
  if [[ ! -x "$ANYWHERE_DOOR_BIN" ]]; then
    echo -e "${RED}C binary not found at ${ANYWHERE_DOOR_BIN}${RESET}"
    echo "Build it with: cd ${ANYWHERE_DOOR_DIR}/src/anywheredoor && cmake -B build && cmake --build build"
    return 1
  fi

  # Direct output commands (no eval needed)
  if [[ "$1" == "test" || "$1" == "dns" || "$1" == "leak" || "$1" == "help" || "$1" == "?" || "$1" == "show" || "$1" == "list" || "$1" == "ls" || "$1" == "git" || "$1" == "docker_daemon" || "$1" == "export" || ("$1" == "use" && "$2" == "") || "$1" == "tui" ]]; then
      ${ANYWHERE_DOOR_BIN} "$1" "$2" "$3" "$4" "$5" "$6"
      local rc=$?
      # If tui returns 1 or 2, it printed export/unset — eval them
      if [[ "$1" == "tui" && ( $rc -eq 1 || $rc -eq 2 ) ]]; then
        # TUI output captured via eval — re-run with eval wrapper
        return
      fi
      return $rc
  fi

  # Eval commands (modify shell environment)
  # For "use" with args or "config", track state for gost restore
  if [[ "$1" == "use" || "$1" == "config" ]];then
    export ANYWHEREDOOR_CURRENT_GATE="$1 $2 $3 $4 $5 $6"
  fi

  eval "$(${ANYWHERE_DOOR_BIN} "$1" "$2" "$3" "$4" "$5" "$6" 2>/dev/null)"

  if [[ "$1" == "off" ]];then
    unset ANYWHEREDOOR_CURRENT_GATE;
    if [[ ! -z "$GOST_PID" ]]; then anywhere_door gost off; fi
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
    _opts=$(anywhere_door help | grep '^ ' | awk -F: '{split($1,args, "/"); for(arg in args){_arg=args[arg];gsub(/[ \t]+/,"",_arg);printf "%s ",_arg;}}')
    opts=$_opts

    # Primary help completions
    _hints=$(anywhere_door help | grep '^ ' | awk -F: '{split($1,args, "/"); for(arg in args){_arg=args[arg];gsub(/[ \t]+/,"",_arg);_exp=$2;gsub(/^[ \t]+/,"", _exp);printf "[%s]=\"%s\"\n",_arg, _exp}}')
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
    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )

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
            local proxies="$(anywhere_door list all 2>/dev/null | grep '^[[:digit:]]' | awk '{print substr($2,6, length($2)-10)}')"
            local indices=$(seq 0 $(echo ${proxies} | wc --words) 2>/dev/null)
            COMPREPLY=( $(compgen -W "${proxies} ${indices}" -- ${cur}) )
            return 0
            ;;
        show)
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
            local test_opts="on off clean update"
            COMPREPLY=( $(compgen -W "${test_opts}" -- ${cur}) )
            return 0
            ;;
        test)
            local test_opts="all full"
            COMPREPLY=( $(compgen -W "${test_opts}" -- ${cur}) )
            return 0
            ;;
        ?|help)
            local help_opts="test use config show gost docker_daemon"
            COMPREPLY=( $(compgen -W "${help_opts}" -- ${cur}) )

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
