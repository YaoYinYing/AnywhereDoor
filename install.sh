#!/usr/bin/env bash
# AnywhereDoor v2.0.0 — Installation Script
#
# Usage:
#   curl -o install.sh https://raw.githubusercontent.com/YaoYinYing/AnywhereDoor/main/install.sh
#   chmod +x install.sh
#   ./install.sh
#
# Or to specify an install prefix:
#   ./install.sh /usr/local
#
# What this script does:
#   1. Detects OS and architecture
#   2. Downloads a prebuilt binary from GitHub Releases, OR builds from source
#   3. Installs the binary and shell wrapper
#   4. Prints instructions for shell setup

set -euo pipefail

VERSION="2.0.0"
REPO="YaoYinYing/AnywhereDoor"
RETRY_COUNT=5

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
RESET='\033[0m'

banner() {
    echo ''
    echo -e "${GREEN}${BOLD}=== AnywhereDoor v${VERSION} Installer ===${RESET}"
    echo ''
}

# --- Platform detection ---
detect_platform() {
    local os
    os=$(uname -s | tr '[:upper:]' '[:lower:]')
    local arch
    arch=$(uname -m)

    case "$arch" in
        x86_64|amd64)   arch="x86_64" ;;
        i386|i686)      arch="i386" ;;
        arm64)
            [[ "$os" == "darwin" ]] && arch="arm64" || arch="aarch64"
            ;;
        aarch64)        arch="aarch64" ;;
        armv7l|armv7|armv8l) arch="armv7" ;;
        riscv64|riscv)  arch="riscv64" ;;
        loongarch64|loong64) arch="loong64" ;;
        *)
            echo -e "${RED}Unsupported architecture: $arch${RESET}"
            exit 1
            ;;
    esac

    case "$os" in
        linux)  PLATFORM="linux-${arch}" ;;
        darwin) PLATFORM="darwin-${arch}" ;;
        *)
            echo -e "${RED}Unsupported OS: $os${RESET}"
            exit 1
            ;;
    esac

    echo -e "Detected: ${GREEN}${PLATFORM}${RESET}"
}

# --- Install prefix ---
resolve_prefix() {
    if [[ -n "${1:-}" ]]; then
        PREFIX="$1"
    elif [[ "$(id -u)" == "0" ]]; then
        PREFIX="/usr/local"
    else
        PREFIX="${HOME}/.local"
    fi

    mkdir -p "${PREFIX}/bin"
    echo -e "Install prefix: ${GREEN}${PREFIX}${RESET}"
}

# --- Download prebuilt binary ---
install_prebuilt() {
    local bin_name="anywhere_door-${PLATFORM}"
    local release_url="https://github.com/${REPO}/releases/latest/download/${bin_name}"
    local dest="${PREFIX}/bin/anywhere_door"

    echo "Trying prebuilt binary: ${bin_name}..."
    if curl -fsSL -o "${dest}" "${release_url}"; then
        chmod +x "${dest}"
        echo -e "${GREEN}Installed prebuilt binary: ${dest}${RESET}"
        return 0
    fi
    echo -e "${YELLOW}Prebuilt binary not available for ${PLATFORM}.${RESET}"
    return 1
}

# --- Build from source ---
build_from_source() {
    local dest="${PREFIX}/bin/anywhere_door"
    local tmpdir
    tmpdir=$(mktemp -d)
    trap "rm -rf ${tmpdir}" EXIT

    echo "Building from source..."

    # Clone
    echo "Cloning ${REPO}..."
    git clone --depth 1 "https://github.com/${REPO}.git" "${tmpdir}/repo"

    local srcdir="${tmpdir}/repo/src/anywheredoor"

    # Install build dependencies
    if command -v brew &>/dev/null; then
        echo "Installing build deps via Homebrew..."
        brew install libyaml curl ncurses cmake 2>/dev/null || true
    elif command -v apt-get &>/dev/null; then
        echo "Installing build deps via apt..."
        sudo apt-get update -qq
        sudo apt-get install -y -qq libyaml-dev libcurl4-openssl-dev libncurses-dev cmake gcc make
    elif command -v dnf &>/dev/null; then
        echo "Installing build deps via dnf..."
        sudo dnf install -y libyaml-devel libcurl-devel ncurses-devel cmake gcc make
    elif command -v pacman &>/dev/null; then
        echo "Installing build deps via pacman..."
        sudo pacman -S --noconfirm libyaml curl ncurses cmake gcc make
    else
        echo -e "${RED}Cannot install build dependencies. Please install cmake, gcc, libyaml-dev, libcurl-dev, ncurses-dev manually.${RESET}"
        exit 1
    fi

    # Build
    cd "${srcdir}"
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build

    # Install
    cp build/anywhere_door "${dest}"
    chmod +x "${dest}"
    echo -e "${GREEN}Built and installed: ${dest}${RESET}"
}

# --- Install shell wrapper ---
install_shell_wrapper() {
    local wrapper_dest="${PREFIX}/share/anywheredoor/anywhere_door.sh"
    local config_dest="${PREFIX}/share/anywheredoor/.anywheredoorrc"

    if [[ -f "${tmpdir:-/nonexistent}/repo/anywhere_door.sh" ]]; then
        mkdir -p "$(dirname "${wrapper_dest}")"
        cp "${tmpdir}/repo/anywhere_door.sh" "${wrapper_dest}"
        cp "${tmpdir}/repo/.anywheredoorrc" "${config_dest}"
    else
        # Download from GitHub
        mkdir -p "$(dirname "${wrapper_dest}")"
        curl -fsSL "https://raw.githubusercontent.com/${REPO}/main/anywhere_door.sh" -o "${wrapper_dest}"
        curl -fsSL "https://raw.githubusercontent.com/${REPO}/main/.anywheredoorrc" -o "${config_dest}"
    fi

    # Fix binary path in shell wrapper if installing to non-standard location
    if [[ "${PREFIX}" != "${HOME}/.local" ]]; then
        sed -i.bak "s|/usr/local/bin/anywhere_door|${PREFIX}/bin/anywhere_door|g" "${wrapper_dest}"
        rm -f "${wrapper_dest}.bak"
    fi

    echo -e "${GREEN}Shell wrapper installed: ${wrapper_dest}${RESET}"
}

# --- Print setup instructions ---
print_setup_info() {
    local wrapper="${PREFIX}/share/anywheredoor/anywhere_door.sh"
    local binary="${PREFIX}/bin/anywhere_door"

    echo ''
    echo -e "${BOLD}=== Installation Complete ===${RESET}"
    echo ''
    echo -e "Binary:   ${GREEN}${binary}${RESET}"
    echo -e "Wrapper:  ${GREEN}${wrapper}${RESET}"
    echo ''
    echo 'To start using AnywhereDoor, add this line to your shell profile'
    echo "(~/.bashrc, ~/.zshrc, or ~/.config/fish/config.fish):"
    echo ''
    echo -e "  ${BOLD}source ${wrapper}${RESET}"
    echo ''
    echo 'Then restart your shell or run:'
    echo ''
    echo -e "  ${BOLD}source ${wrapper}${RESET}"
    echo ''
    echo 'Quick start:'
    echo '  anywhere_door help       # Show available commands'
    echo '  anywhere_door list       # List configured proxies'
    echo '  anywhere_door tui        # Interactive terminal UI'
    echo ''
}

# --- Main ---
main() {
    banner
    detect_platform
    resolve_prefix "${1:-}"

    if ! install_prebuilt; then
        build_from_source
    fi

    print_setup_info
}

main "$@"
