# ED, the standard editor, now UTF-8/Unicode-aware.

This is a port of the Research Unix (v10) ED, with UTF-8/Unicode
support. In particular, regexes and subs are fully Unicode-aware.

The sources that this port began with were scavenged from the
Research Unix sources available from
[The Unix Heritage Society](https://tuhs.org/).

The original manpage is in the `doc` subdirectory.
Where the behaviour of this `ed` differs significantly from
that described in the manpage is noted below.

The `Makefile` provides a simple way to build and install
an `ed` binary.
`make clean && make` will get you a standalone binary `ed` that
can be installed anywhere. `make install` will install ED as `ed`
and `e`, with corresponding manpages. Edit the `Makefile` to change
where all this goes.

## Differences

This `ed` differs from the ED described in the manpage as follows:

+ This `ed` consumes and outputs UTF-8 Unicode. Internally, all
  "characters" are stored and manipulated as Unicode code-points
  represented as `int`-s. This means that even
  _emojis_ can appear in regexes and character classes.

+ In *list mode*, this `ed` displays non-printing codepoints in the
  ASCII range, and all Unicode codepoints above the printing ASCII
  range as a sequence `\ddd` of octal triplets, one for each byte
  in the UTF-8 encoding of the codepoint. Octal is the traditional
  ED list mode format, and it is a surprisingly good way of reading
  raw UTF-8, as all multi-byte sequences begin with byte `\3..`, and all
  subsequent sequence bytes begin `\2..`. (Non-printing) ASCII bytes
  are all of the form `\0..`, or `\1..`.

+ The original manpage states that ED discards all text in a file that
  appears between the last newline and the end-of-file. The
  original `ed.c` source code does not do that. Instead it
  supplies a newline at the end of the file, and notifies the
  user with a: `'\n' appended` message. This behaviour has been
  retained.

