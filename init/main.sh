source help.sh
source udk.sh
source git.sh

function print_success
{
    echo -e "\033[32m$1\033[0m"
}

function print_error
{
    echo -e "\033[31m$1\033[0m"
}

function highlight
{
    echo -e "\033[31m$1\033[0m"
}

export _callback=() # 回调函数, 修改重要文件时的补救措施

clear

logo
description

while true; do
    read -p "Press your password : " PASSWORD

    sudo -k
    if ! echo ${PASSWORD} | sudo -S -p "" echo 2>/dev/null;then
        print_error "Password is incorrect!!!"
    else
        echo "PASSWORD:=${PASSWORD}" > ../src/config/private.mk
        print_success "\033[1ARecorded the password (${PASSWORD}) and you can change it in src/config/private.mk"

        break
    fi
done

# while true ;do
#     read -p "Do you want to install udkdebugger ? [y/n] : " UDK_ENABLE
#     if [[ ${UDK_ENABLE} == "y" ]]; then
#         udk_setup
#         break
#     elif [[ ${UDK_ENABLE} == "n" ]]; then
#         break
#     fi
# done

highlight "========================"
highlight "Please install UdkDebugger step by step later"
highlight "========================"

if ! repo_setup; then
    print_error "Unable to clone submodule"
    
    exit 1
fi

echo -e "Starting to compile baseTools..."
! make -s -C ../src/boot/Edk2/BaseTools/Source/C && exit 1

for func in ${_callback[@]}; do
    `echo $func`
done

