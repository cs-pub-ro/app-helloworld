sudo qemu-system-aarch64 -fsdev local,id=myid,path=$(pwd)/fs0,security_model=none \
                         -device virtio-9p-pci,fsdev=myid,mount_tag=rootfs,disable-modern=on,disable-legacy=off \
                         -kernel "build/app-helloworld_kvm-arm64" \
                         -machine virt \
                         -cpu cortex-a57 \
                         -nographic