#!/usr/bin/env bash

ACTION_CHOICE="$1"

TAB=$'\t'
BOLD="\033[1m"
RED="\033[1;31m"
RESET="\033[0m"

yes_arg() {
    (( $# == 1 ))
}

first_arg_check() {
    case "$1" in
        build|testing|package|setup|test) return 0 ;;
        *) echo -e "${RED}Error:${RESET} Invalid Action '$1'"; return 1 ;;
    esac
}

no_arg() {
    (( $# == 0 ))
}

if no_arg "$@"; then
    echo -e "${BOLD}Verteilen Helper${RESET}"
    echo "bash helper.sh [Action] [Arguments...]"
    echo ""
    echo "Action:"
    echo "${TAB}build - Build the library and executable"
    echo "${TAB}copy - Copy the resources"
    echo "${TAB}package - Packing the resources"
    echo "${TAB}setup - Setup project files"
    echo "${TAB}test - Start the testing"
    echo ""
    echo "Arguments:"
    exit 0
fi

if ! yes_arg "$@"; then
    echo -e "${RED}Error:${RESET} Missing required arguments. Expected at least 3."
    echo "Usage: bash helper.sh [Language] [Module] [Action] [Arguments...]"
    exit 1
fi

if ! first_arg_check "$1"; then
    exit 1
fi

shift 1
EXTRA_ARGS="$@"

cd "scripts"

bash "${ACTION_CHOICE}.sh" ${EXTRA_ARGS}