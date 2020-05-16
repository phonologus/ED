# ED, the standard editor, now UTF8/Unicode-aware.

This is a port of the Research Unix (v10) ED, with full UTF8/Unicode
support. In particular, regexes and subs are fully Unicode-aware.

The port was developed on a modern Linux (Fedora 32). It
depends on just a few low-level kernel calls, so should be reasonably
portable accross any Unix-like OS (*BSD, MacOS).

The sources that this port began with were scavenged from the
Research Unix sources available from
[The Unix Heritage Society](https://tuhs.org/).

The manpage is in the `doc` subdirectory.

The `Makefile` provides a simple way to build an `ed` binary.

The more elaborate `mkfile` can build, install, and archive.
It can also run off a `pdf` version of the manpage. However, it
depends on you having a working [plan9port](https://github.com/9fans/plan9port).

