# libjpeg-turbo - JPEG image codec C library

This is a `build2` package for the
[`libjpeg-turbo`](https://github.com/libjpeg-turbo/libjpeg-turbo) C library.
It provides a JPEG image codec that uses SIMD instructions to accelerate
baseline and progressive JPEG compression and decompression on systems with
common CPU architectures. This package builds the traditional libjpeg API
(emulating libjpeg v6b), not the TurboJPEG API.


## Usage

To start using `libjpeg-turbo` in your project, add the following `depends`
value to your `manifest`, adjusting the version constraint as appropriate:

```
depends: libjpeg-turbo ^3.2.1
```

Then import the library in your `buildfile`:

```
import libs = libjpeg-turbo%lib{jpeg}
```


## Importable targets

This package provides the following importable targets:

```
lib{jpeg}
```

The libjpeg API library. Its public headers are included as `<jpeglib.h>`
(which includes `<jconfig.h>` and `<jmorecfg.h>`) and `<jerror.h>`.


## Configuration variables

This package provides the following configuration variables:

```
[bool] config.libjpeg_turbo.simd ?= true
```

Build the SIMD extensions, which this package currently supports on x86-64
only. There they require the `nasm` assembler, which becomes a build-time
dependency, and the library selects SSE2 or AVX2 at run time. Set it to
`false` to build only the C code paths.
