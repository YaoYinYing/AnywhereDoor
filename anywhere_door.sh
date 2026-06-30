# ANYWHERE DOOR

# Selfcheck for the installation dir
SHELL_TYPE=$(basename $SHELL)
if [[ "$SHELL_TYPE" == "bash" ]]; then
  core_script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
elif [[ "$SHELL_TYPE" == "zsh" ]]; then
  core_script_dir=${0:A:h}
fi
export ANYWHERE_DOOR_DIR=$core_script_dir

# version info
export ANYWHERE_DOOR_DIR_VERSION=2.0.0-alpha

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

# --- GOST helpers (pure shell) ---
function _gost_precheck {
  if [[ $(anywhere_door show) =~ 'inactive' ]]; then
    echo "Anywheredoor must be active before running gost."
    return 1
  fi
}

function _gost_restore {
  if [[ -n "$ANYWHEREDOOR_CURRENT_GATE" ]]; then
    anywhere_door $ANYWHEREDOOR_CURRENT_GATE
  fi
}

# --- main function ---
function anywhere_door {
  case "$1" in
    version)
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
      ;;

    gost)
      if ! command -v gost; then
        echo "Gost not found. Please see: https://gost.run/en/"
        return
      fi
      case "$2" in
        clean)
          _gost_precheck || return
          echo "Remaining gost process will be cleaned"
          ps aux | grep gost | awk '{print $2; system("kill " $2)}' >/dev/null 2>&1
          echo Done.
          ;;
        off)
          _gost_precheck || return
          anywhere_door gost clean
          _gost_restore
          ;;
        on|"")
          _gost_precheck || return
          GOST_PORT=${3:-63322}
          [[ "$GOST_PORT" =~ ^[0-9]+$ ]] || GOST_PORT=63322
          echo "Using gost port: $GOST_PORT"
          gost -L=:${GOST_PORT} -F=$(anywhere_door show all | cut -b 12-) >/dev/null 2>&1 &
          export GOST_PID=$!
          export http_proxy=http://127.0.0.1:${GOST_PORT}
          export https_proxy=http://127.0.0.1:${GOST_PORT}
          export all_proxy=http://127.0.0.1:${GOST_PORT}
          echo "GOST running at $GOST_PID"
          ;;
        stop)
          _gost_precheck || return
          if [[ -n "$GOST_PID" ]]; then
            kill $GOST_PID >/dev/null 2>&1
            echo "Stop background GOST at ${GOST_PID}"
            unset GOST_PID GOST_PORT
            _gost_restore
          else
            echo "GOST is not running"
          fi
          ;;
        update)
          _gost_precheck || return
          anywhere_door gost clean
          _gost_restore
          anywhere_door gost on
          ;;
        *) echo "Unknown command for anywhere_door gost: $2 $3" ;;
      esac
      ;;

    bench)
      command -v speedtest && speedtest --progress=yes --selection-details \
        || echo "Please install the speedtest command line tool first."
      ;;

    wget)
      command -v wget && wget -O /dev/null 'https://speedtest.yaoyy.moe/' \
        || echo "Please install wget first."
      ;;

    curl)
      command -v curl && curl -o /dev/null 'https://speedtest.yaoyy.moe/' \
        || echo "Please install curl first."
      ;;

    whereami)
      command -v curl && curl 'https://ipinfo.io/' \
        || echo "Please install curl first."
      ;;

    ipq)
      echo "Running in progress..."
      $SHELL ${ANYWHERE_DOOR_DIR}/tools/IPQuality/ip.sh -x $(anywhere_door show http | awk -F= '{print $2}')
      ;;

    netq)
      echo "Running in progress..."
      $SHELL ${ANYWHERE_DOOR_DIR}/tools/NetQuality/net.sh
      ;;

    upgrade)
      pushd $ANYWHERE_DOOR_DIR >/dev/null
        git stash; git fetch --all; git reset --hard origin/main; git pull
      popd >/dev/null
      ;;

    refresh)
      source $ANYWHERE_DOOR_DIR/anywhere_door.sh
      ;;

    # --- C binary commands ---
    *)
      if [[ ! -x "$ANYWHERE_DOOR_BIN" ]]; then
        echo -e "${RED}C binary not found at ${ANYWHERE_DOOR_BIN}${RESET}"
        echo "Build it with: cd ${ANYWHERE_DOOR_DIR}/src/anywheredoor && cmake -B build && cmake --build build"
        return 1
      fi

      case "$1" in
        # Direct: output to terminal
        test|dns|leak|help|\?|show|list|ls|git|docker_daemon|export|tui)
          ${ANYWHERE_DOOR_BIN} "$@"
          ;;

        # Direct when no args, eval when selecting
        use)
          if [[ -z "$2" ]]; then
            ${ANYWHERE_DOOR_BIN} "$@"
          else
            export ANYWHEREDOOR_CURRENT_GATE="$*"
            eval "$(${ANYWHERE_DOOR_BIN} "$@" 2>/dev/null)"
          fi
          ;;

        # Eval: modify shell environment
        on|off|config)
          [[ "$1" == "config" ]] && export ANYWHEREDOOR_CURRENT_GATE="$*"
          eval "$(${ANYWHERE_DOOR_BIN} "$@" 2>/dev/null)"
          if [[ "$1" == "off" ]]; then
            unset ANYWHEREDOOR_CURRENT_GATE
            [[ -n "$GOST_PID" ]] && anywhere_door gost off
          fi
          ;;

        # Default (empty / unknown): same as "on"
        *) eval "$(${ANYWHERE_DOOR_BIN} "$@" 2>/dev/null)" ;;
      esac
      ;;
  esac
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
