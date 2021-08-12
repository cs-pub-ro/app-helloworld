# Boot TLS Implementation

The aim is to add TLS sections (equivalent to `.tdata` and `.tbss`) in Unikraft as early in the boot process as possible.
Some components (such as GCOV) require early TLS support.
We call this boot TLS, and the sections are refered to as `.tbootdata` and `.tbootbss`.

The current approach is to enhance the resulting image with these two new sections (`.tbootdata` and `.tbootbss`).
When the image is loaded, the sections become available in the runtime; by setting the proper registers and pointers, they would be used.
`.tbootdata` is pre-populated with the contents of the `.tdata` section, whereas `.tbootbss` is filled with zeroes.

Currently, only KVM is supported.

## Implementation

In order to add two new `ELF` sections to be used for boot TLS, there were a few changes that needed to be done, so that we may have the `.tdata` and `.tbss` sections duplicated before runtime:
1. add the new sections inside the Unikraft platform linker script
1. in order for these two new sections to be successfully mapped to a loadable segment, we use `__attribute__((section("section-name")))` through two dummy placeholders that are to be placed anywhere inside the codebase and they can be of any size
1. manually update the resulted sections in the final binary to match the contents of the `.tdata` and `.tbss`.

### Unikraft Platform Linker Script

The following changes have been made to the Unikraft platform linker script:
```
--- a/plat/common/include/uk/plat/common/common.lds.h
+++ b/plat/common/include/uk/plat/common/common.lds.h
@@ -105,6 +105,7 @@
 #define TLS_SECTIONS							\
 	. = ALIGN(0x8);							\
 	_tls_start = .;							\
+        _stdata = .;                                                   \
 	.tdata :							\
 	{								\
 		*(.tdata)						\
@@ -112,13 +113,33 @@
 		*(.gnu.linkonce.td.*)					\
 	}								\
 	_etdata = .;							\
-	.tbss :								\
+        _stbss = .;                                                    \
+       	.tbss :							\
 	{								\
 		*(.tbss)						\
 		*(.tbss.*)						\
 		*(.gnu.linkonce.tb.*)					\
 		. = ALIGN(0x8);						\
+	}                                                               \
+        _etbss = . + SIZEOF(.tbss);                                    \
+	_tls_end = . + SIZEOF(.tbss);                                   \
+        . += SIZEOF(.tbss);                                            \
+        _stbootdata = .;                                               \
+        .tbootdata :							\
+	{								\
+		*(.tbootdata)						\
+		*(.tbootdata.*)						\
 	}								\
-	_tls_end = . + SIZEOF(.tbss);
+	_etbootdata = .;                                                \
+        . = ALIGN(0x8);						\
+        . += SIZEOF(.tdata);                                           \
+        _stbootbss = .;                                                \
+	.tbootbss :							\
+	{								\
+		*(.tbootbss)						\
+		*(.tbootbss.*)						\
+	}                                                               \
+        _etbootbss = .; \
+        . += SIZEOF(.tbss);
```

Notice the added padding (`. += SIZEOF`).
This is to ensure that whatever section would normally follow the two new sections (`.tbootdata` and `.tbootbss`) would not overlap.
The exported symbols have been added to help with debugging.

### Unikraft Placeholders

For the two new sections to be properly mapped to a loadable segment, the following additions were made:
```
--- a/lib/ukboot/boot.c
+++ b/lib/ukboot/boot.c
@@ -72,6 +72,11 @@

 int main(int argc, char *argv[]) __weak;

+#if 1
+__thread char tbootbss_placeholder __attribute__((section(".tbootbss"))) = 0;
+__thread char tbootdata_placeholder __attribute__((section(".tbootdata"))) = 1;
+#endif
+
 static void main_thread_func(void *arg) __noreturn;

 struct thread_main_arg {
```

These will act as placeholders for the two sections in the final `ELF` binary.

These changes are available in the [Unikraft cs-pub-ro fork](https://github.com/cs-pub-ro/unikraft), the `boot-TLS-attr-buffer` branch.
See [this commit](https://github.com/cs-pub-ro/unikraft/commit/7abdc867d6c2ca9aeb032d5346b98f81e767fd77).

### Manual Content Filling

Now, for the final step, we will manually copy the contents of `.tdata` and what would have been `.tbss` after load time by using `objcopy`.
An example of how we could do this can be seen in this directory's `fill-tls.sh`.
We basically extract the contents of `.tdata` and update the `.tbootdata` section with them.
Next, we generate what would have been the contents of `.tbss` after load time (a number of zero bytes equal to the size of `.tbss`) and update the `.tbootbss` section with those.

## Building and Running

To build a boot TLS-enabled Unikraft image, do the following
* Use [this repository](https://github.com/cs-pub-ro/app-helloworld/tree/test-boot-tls), the `test-boot-tls` branch.
* Clone the [Unikraft cs-pub-ro fork](https://github.com/cs-pub-ro/unikraft), the `boot-TLS-attr-buffer` branch.
* Update the `Makefile` accordingly: the `UK_ROOT` or `UK_LIBS` variable may require updating.
* Use `make menuconfig` and just save the configuration.
  Configuration is loaded from the `Config.uk` file.
* Use `make` to build the vanilla images in `build/`.
* Use `fill-tls.sh` to fill the boot TLS sections (`.tbootdata` and `.tbootbss`).
* Run the resulting images using:

  ```
  ./qemu-guest -k build/app-helloworld_kvm-x86_64.tboot
  ```

  If you're lucky, the running command works.
  If not, it hangs.
  This issue (where it hangs is detailed below).

## Issue

Let's consider this small random example, where in our `main.c` we would have:
```
__thread volatile char test_var_tdata[10] = {1,2,3,4,5,6,7,8,9,10};
__thread volatile char test_var_tbss[20];

```

After we build, this is what `readelf -S build/app-helloworld_kvm-x86_64.dbg` shows us:
```
  [ 8] .tdata            PROGBITS         000000000013d450  0003e450
       000000000000000a  0000000000000000 WAT       0     0     16
  [ 9] .tbss             NOBITS           000000000013d460  0003e45a
       0000000000000148  0000000000000000 WAT       0     0     16
  [10] .tbootdata        PROGBITS         000000000013d5a8  0003e5a8
       0000000000000001  0000000000000000 WAT       0     0     1
  [11] .tbootbss         PROGBITS         000000000013d5ba  0003e5ba
       0000000000000001  0000000000000000 WAT       0     0     1
  [12] .data             PROGBITS         000000000013e000  0003f000
       0000000000002a30  0000000000000000  WA       0     0     32
```

Now, we will update the `.tbootdata` and `.tbootbss` to be the duplicates of `.tdata` and `.tbss` respectively, by using `./fill-tls.sh`.

This is what `readelf -S build/app-helloworld_kvm-x86_64.dbg.tboot` shows us (keep in mind that the updated binary's name is different):
```
  [ 8] .tdata            PROGBITS         000000000013d450  0003e450
       000000000000000a  0000000000000000 WAT       0     0     16
  [ 9] .tbss             NOBITS           000000000013d460  0003e45a
       0000000000000148  0000000000000000 WAT       0     0     16
  [10] .tbootdata        PROGBITS         000000000013d5a8  0003e5a8
       000000000000000a  0000000000000000 WAT       0     0     1
  [11] .tbootbss         PROGBITS         000000000013d5ba  0003e5ba
       0000000000000148  0000000000000000 WAT       0     0     1
  [12] .data             PROGBITS         000000000013e000  0003f000
       0000000000002a30  0000000000000000  WA       0     0     32
```

Besides the alignment, which we wouldn't need anyway after obtaining the final binary, this seems to work.
The runtime confirms as well that nothing is broken (we use `./qemu-guest -k build/app-helloworld_kvm-x86_64.dbg.tboot` to run the binary).

However, what would happen if we were to increase the size of `.tdata` or `.tbss` just enough to force the padding to push the alignment of `.data` to the next page-aligned address?
To find out, we need to increase the cumulated size of the padding and the TLS sections accordingly.

We are going to do this through an additional buffer whose size we could easily control:
```
__thread volatile char test_var_tdata[10] = {1,2,3,4,5,6,7,8,9,10};
__thread volatile char test_var_tbss[20];
__thread volatile char push_data_tbss[1136];  // additional buffer
```

After some testing, we find this limit of the additional buffer to be around `0x470` (or `1136`).
Anything above this size and `.data` will be forced to the next page-aligned address (for the sake of this example we increased only `.tbss`, but this can also be replicated for `.tdata` with the size of the buffer being anything above `0x478` or `1144`).

After building and running `./fill-tls.sh` we get the following output in the case of `readelf -S build/app-helloworld_kvm-x86_64.dbg.tboot`:
```
  [ 8] .tdata            PROGBITS         000000000013d450  0003e450
       000000000000000a  0000000000000000 WAT       0     0     16
  [ 9] .tbss             NOBITS           000000000013d460  0003e45a
       00000000000005b8  0000000000000000 WAT       0     0     16
  [10] .tbootdata        PROGBITS         000000000013da18  0003ea18
       000000000000000a  0000000000000000 WAT       0     0     1
  [11] .tbootbss         PROGBITS         000000000013da2a  0003ea2a
       00000000000005b8  0000000000000000 WAT       0     0     1
  [12] .data             PROGBITS         000000000013e000  0003f000
       0000000000002a30  0000000000000000  WA       0     0     32
```

From what we can see, `.data` is at the same address. What is more, running the final binary yields nothing unusual.
Now, we will increase the size of the buffer by just 1 byte, thus forcing `.data` to be aligned at the next address:
```
__thread volatile char test_var_tdata[10] = {1,2,3,4,5,6,7,8,9,10};
__thread volatile char test_var_tbss[20];
__thread volatile char push_data_tbss[1137];  // this was previously 1136
```

Now `readelf` confirms the address of `.data` has changed:
```
  [ 8] .tdata            PROGBITS         000000000013d450  0003e450
       000000000000000a  0000000000000000 WAT       0     0     16
  [ 9] .tbss             NOBITS           000000000013d460  0003e45a
       00000000000005c8  0000000000000000 WAT       0     0     16
  [10] .tbootdata        PROGBITS         000000000013da28  0003ea28
       000000000000000a  0000000000000000 WAT       0     0     1
  [11] .tbootbss         PROGBITS         000000000013da3a  0003ea3a
       00000000000005c8  0000000000000000 WAT       0     0     1
  [12] .data             PROGBITS         000000000013f000  0003f020
       0000000000002a30  0000000000000000  WA       0     0     32
```

And if we were to run the binary, we would be stuck with this output:
```
SeaBIOS (version rel-1.14.0-0-g155821a1990b-prebuilt.qemu.org)
Booting from ROM...

```
This seems to be caused by the fact that the `.data` section has been tampered with.

By comparing the runtime dump of `.data` from the previous working example (`data.dump.works`) with the one from the broken example (`data.dump.broken`), we can clearly see that they do not match.
However, what is noticeable is that some of the human-readable bytes that `xxd` could output for us seem to be the exact same, but their offset is different.
For example: In the broken example we have this at the offset `0x1950` inside the `.data` section:
```
00001950: 60ce 1200 0000 0000 30cf 1200 0000 0000  `.......0.......
00001960: f01a 1200 0000 0000 001b 1200 0000 0000  ................
00001970: f01a 1200 0000 0000 90d1 1200 0000 0000  ................
00001980: 00d6 1200 0000 0000 f0db 1200 0000 0000  ................
00001990: f0e1 1200 0000 0000 30e0 1200 0000 0000  ........0.......
000019a0: 70dc 1200 0000 0000 20e0 1200 0000 0000  p....... .......
000019b0: f0ca 1200 0000 0000 60ca 1200 0000 0000  ........`.......
000019c0: f01a 1200 0000 0000 50d4 1200 0000 0000  ........P.......
```

In the working example we have the exact same bytes but at a different offset, that of `0x2930`.
```
00002930: 60ce 1200 0000 0000 30cf 1200 0000 0000  `.......0.......
00002940: f01a 1200 0000 0000 001b 1200 0000 0000  ................
00002950: f01a 1200 0000 0000 90d1 1200 0000 0000  ................
00002960: 00d6 1200 0000 0000 f0db 1200 0000 0000  ................
00002970: f0e1 1200 0000 0000 30e0 1200 0000 0000  ........0.......
00002980: 70dc 1200 0000 0000 20e0 1200 0000 0000  p....... .......
00002990: f0ca 1200 0000 0000 60ca 1200 0000 0000  ........`.......
000029a0: f01a 1200 0000 0000 50d4 1200 0000 0000  ........P.......
```

If we check again during runtime on the broken example with an offset difference of `-4064` (`expected_data_addr + (0x1950 - 0x2930)`) from where we would expect `.data` to start, we will see that, indeed, that is where our `.data` is placed.
For some reason the `.data` section is not loaded at the address that is specified inside the `ELF`, which leads to issues when certain pointers or values from `.data` are accessed and they are not what they are expected to be.
