#!/bin/bash

if test $# -ne 1; then
    echo "Usage: $0 <kvm_image>" 1>&2
    exit 1
fi

kvm_image="$1"
qemu_script="./qemu-guest"
debug_port="12345"

"$qemu_script" -k "$kvm_image" -g "$debug_port" -P
