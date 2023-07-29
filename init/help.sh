#!/usr/bin/env bash

function logo
{
    awk '{ print "\033[32m" $0 "\033[0m" }' < logo.txt
}

function description
{
    echo -e "这是一个 Uefi 引导的操作系统项目."
    echo -e
    echo -e "- Boot : SigmaBoot"
    echo -e "- Opearting System : TextOS"
    echo -e
    echo -e "- \033[91m[Gitee]\033[0m (\033[94m\033[4mhttps://gitee.com/canyan233\033[0m)"
    echo -e "- \033[91m[GitHub]\033[0m (\033[94m\033[4mhttps://github.com/ljQAQ233\033[0m)"
    echo -e "- \033[91m[Bilibili]\033[0m (\033[94m\033[4mhttps://space.bilibili.com/503518259\033[0m)"
}

