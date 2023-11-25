# blocks
Source code for tetris tables and beyond

Requires:
libasound2-dev
libfftw3-dev

# Cross Compile reminders

## Create a SYSROOT

```
rsync -lrvz --copy-unsafe-links --exclude=/lib/modules --exclude=/lib/firmware user@host.local:/lib newsysroot
rsync -lrvz --copy-unsafe-links user@host.local:/usr/lib newsysroot/usr/
rsync -lrvz --copy-unsafe-links user@host.local:/usr/include newsysroot/usr/
```

## Compiler flags to find stuff

GCC Flags:
--sysroot=${SYSROOT} -iprefix=${SYSROOT}

## Linker flags to find stuff

Linker Flags:
--sysroot=${SYSROOT} -iprefix=${SYSROOT}
-B${SYSROOT} -B${SYSROOT}/lib/arm-linux-gnueabihf -B${SYSROOT}/usr/lib/arm-linux-gnueabihf
-Xlinker -rpath=${SYSROOT}/lib/arm-linux-gnueabihf/