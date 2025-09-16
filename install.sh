

RETRY_COUNT=10

ensure_deps(){
    echo "Ensuring Python Depts ..."
    pip install PyYAML pysocks5; install_code=$?
    if [ $install_code==130 ];then 
        echo 'Reinstall with opt `--break-system-packages`' ; 
        pip install PyYAML pysocks5 --break-system-packages
    else
        echo -e "Unknown Error occurred!"
        exit 1
    fi

}

# fetch install dir
if [[ ! -z "$1" ]];then install_dir=$1; else read -p "Please enter your path to AnywhereDoor: " install_dir; fi
echo Anywhere Door will be installed at ${install_dir}

# ensure dir
ensure_dir(){
if [[ ! -d ${install_dir} ]];then { echo Creating ${install_dir}; mkdir -p ${install_dir}; }; else echo Using exiting dir: ${install_dir};fi;
install_dir=$(realpath ${install_dir})
echo Installation path fixed: ${install_dir}
}


# ask for current shell apply
ask_apply(){
    read -p "Would you like to apply now?[Y/N] " do_apply
    do_apply="${do_apply^^}" # to upper
    if [[ "$do_apply" == "Y" ]];then 
        source ${source_file}
    elif [[ "$do_apply" == "N" ]];then
        echo 'Okay skipped'
    else 
        echo Not a valid answer!
        ask_apply
    fi
}

ask_append_shell_profile(){
    read -p "Would you like to append AnywhereDoor to shell profile now?[Y/N] " do_append
    do_append="${do_append^^}" # to upper
    SHELL_TYPE=$(basename  $SHELL)
    SHELL_PROFILE_RC=$(readlink -f "$HOME/.${SHELL_TYPE}rc")


    if [[ "$do_append" == "Y" ]];then 
        if [[ ! -f ${SHELL_PROFILE_RC} ]];then
            echo "Creating a new shell profile at \`${SHELL_PROFILE_RC}\` "
            touch ${SHELL_PROFILE_RC}
        fi
        echo -e "\n# Anywhere Door installation \nsource ${install_dir}/AnywhereDoor/anywhere_door.sh\n" >> ${SHELL_PROFILE_RC}
    elif [[ "$do_append" == "N" ]];then 
        echo 'Okay skipped'; 
    else
        echo Not a valid answer! 
        ask_append_shell_profile
    fi
}

run_procedure(){
    ensure_deps;
    ensure_dir;
    source_file="${install_dir}/AnywhereDoor/anywhere_door.sh"
    
    for retry_id in $(seq 1 $RETRY_COUNT);do
        # git fetch and show 
        pushd ${install_dir};
            git clone https://github.com/YaoYinYing/AnywhereDoor.git; install_code=$?
        popd
        echo install_code=$install_code
        if [[ "$install_code" == "0" ]];then
            echo Success - Now enjoy the free internet!
            echo "To apply the Anywhere door to your ssh session, use \`source ${install_dir}/AnywhereDoor/anywhere_door.sh \`"
            echo "Done with installing! Congrats!!!"
            break
        else
            echo "Retrying (${retry_id}/${RETRY_COUNT}) ..."
            if [[ -d "${install_dir}/AnywhereDoor" ]]; then 
                echo "Removing: ${install_dir}/AnywhereDoor"
                rm -rf "${install_dir}/AnywhereDoor"
            fi
        fi
        # finial exit if still fails on auto trials
        exit 1
    done
    ask_apply;
    ask_append_shell_profile;
}

run_procedure