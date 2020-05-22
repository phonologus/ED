# ED, the standard editor, now UTF8/Unicode-aware.

This is a port of the Research Unix (v10) ED, with full UTF8/Unicode
support. In particular, regexes and subs are fully Unicode-aware.

The port was developed on a modern Linux (Fedora 32). It
depends on just a few low-level kernel calls, so should be reasonably
portable accross any Unix-like OS.
I have successfully built it on Linux, macOS and OpneBSD, with both
`clang` and `gcc`.

The sources that this port began with were scavenged from the
Research Unix sources available from
[The Unix Heritage Society](https://tuhs.org/).

The original manpage is in the `doc` subdirectory.
Where the behaviour of this `ed` differs significantly from
that described in the manpage is noted below.

The `Makefile` provides a simple way to build an `ed` binary.
`make clean && make` will get you a standalone binary `ed` that
can be installed anywhere.

The more elaborate `mkfile` can build, install, and archive.
It can also run off a `pdf` version of the manpage. However, it
depends on you having a working [plan9port](https://github.com/9fans/plan9port).

## Differences

This `ed` differs from the ED described in the manpage as follows:

+ This `ed` consumes and outputs UTF8 Unicode. Internally, all
  "characters" are stored and manipulated as Unicode code-points
  represented as `int`-s. This means that even
  _emojis_ can appear in regexes. Shudder.

+ In *list mode*, this `ed` displays non-printing codepoints in the
  ASCII range as `\xhh`. Additionally, it displays non-ASCII
  code-points in the Basic Multilingual Plane as `\uhhhh`, and
  code-points beyond the BMP as `\Uhhhhhh`.
