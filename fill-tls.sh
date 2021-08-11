#!/bin/bash

# Obtain contents of .tdata and place them in temptdata.bin.
objcopy -O binary --only-section=.tdata build/app-helloworld_kvm-x86_64.dbg tdata.dump

# Update dummy section .tbootdata with the contents of .tdata.
echo "Filling tdata from build/app-helloworld_kvm-x86_64.dbg in build/app-helloworld_kvm-x86_64.dbg.tdata ..."
cp build/app-helloworld_kvm-x86_64.dbg build/app-helloworld_kvm-x86_64.dbg.tdata
objcopy --update-section .tbootdata=tdata.dump build/app-helloworld_kvm-x86_64.dbg.tdata

echo "Filling tdata from build/app-helloworld_kvm-x86_64 in build/app-helloworld_kvm-x86_64.tdata ..."
cp build/app-helloworld_kvm-x86_64 build/app-helloworld_kvm-x86_64.tdata
objcopy --update-section .tbootdata=tdata.dump build/app-helloworld_kvm-x86_64.tdata

# size of .tbss as shown by readelf
TBSS_SIZE_STR=$(readelf -S build/app-helloworld_kvm-x86_64 | \
             grep tbss -A1 | awk '{print $1}' | sed -n 2p | tr -d '\n')

# size of .tbss as hexadecimal
TBSS_SIZE_HEX=$(echo "0x${TBSS_SIZE_STR#"${TBSS_SIZE_STR%%[!0]*}"}")

# size of .tbss as decimal
TBSS_SIZE=$(printf %d ${TBSS_SIZE_HEX})

# Generate what would have been the contents of .tbss
for i in $(seq ${TBSS_SIZE}); do echo -en '\x00'; done >> tbss.dump

# Place .tbss zeroes into out .tbootdata dummy section.
echo "Filling tdata from build/app-helloworld_kvm-x86_64.dbg.tdata in build/app-helloworld_kvm-x86_64.dbg.tboot ..."
cp build/app-helloworld_kvm-x86_64.dbg.tdata build/app-helloworld_kvm-x86_64.dbg.tboot
objcopy --update-section .tbootbss=tbss.dump build/app-helloworld_kvm-x86_64.dbg.tboot

echo "Filling tdata from build/app-helloworld_kvm-x86_64.tdata in build/app-helloworld_kvm-x86_64.tboot ..."
cp build/app-helloworld_kvm-x86_64.tdata build/app-helloworld_kvm-x86_64.tboot
objcopy --update-section .tbootbss=tbss.dump build/app-helloworld_kvm-x86_64.tboot

# Clean temporary files
rm tdata.dump tbss.dump
