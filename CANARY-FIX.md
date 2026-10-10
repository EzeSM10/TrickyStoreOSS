# Canary libcore_bundle compatibility patch

Based on Tricky Store OSS v3.1.0 (41383f5). Experimental ARM64 build.

On the Pixel 8 Canary build ZP11.260918.007, `/system/lib64/libbinder.so`
and `libutils.so` are symlinks to `libcore_bundle.so`. The dynamic linker
reports the real shared object in `dl_iterate_phdr`, so the original module
cannot find its Binder hook target.

The patch retains `libbinder.so` as the first choice and falls back to
`libcore_bundle.so`. Both expose the same imported `ioctl` symbol. It does
not change the Java daemon, attestation configuration, keybox or target list.

The flashable ZIP reuses the official v3.1.0 release and replaces only its
ARM64 interceptor, labels the build as experimental, disables its upstream
update feed and restricts installation to ARM64. Existing configuration is
preserved by the original installer.

## Validation performed

- Built the native interceptor with Android NDK 29.0.14206865 and the
  matching `org.lsposed.libcxx:libcxx` dependency.
- Confirmed the loaded core bundle has an `ioctl` PLT relocation and the
  Binder/RefBase symbols needed by the interceptor.
- Ran `tests/canary_plt_probe.c` in a disposable process on the Pixel: the
  fallback selected the core bundle, installed an ioctl hook and removed it.
- Ran `tests/canary_entry_probe.c` with the complete patched library in a
  separate process on the Pixel: its entry point initialized successfully.
- Checked ARM64 ELF format, 16 KiB segment alignment and ZIP CRCs.

The isolated probes did not inject into `keystore2`. After those checks, the
experimental ZIP was installed on the Pixel 8 and the device was rebooted.
Live logs showed Binder calls being intercepted. The device owner then reported
passing BASIC, DEVICE and STRONG with the existing keybox, configuration and
other installed modules. This is one reported device result; it does not
guarantee those verdicts on other setups or after future updates.

## Build native library

Place the matching libcxx AAR's `prefab/modules/cxx` directory under
`toolchain/libcxx/prefab/modules/cxx`, then run `./build-canary-native.sh`.
The script uses the NDK installed under `$HOME/android-sdk` by default;
override `TS_SDK_ROOT` if needed. The CMake imported-target configuration is
generated from the local libcxx dependency.

Keep the official v3.1.0 ZIP and the device's existing configuration before
testing the experimental replacement. No module installation or reboot is
performed by the build script.
