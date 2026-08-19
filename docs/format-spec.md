# Binary Format Specification

This document describes the on-disk layout produced by `compresso`, as
implemented in `src/main.c`, `src/rle.c`, and `src/huffman.c`. It reflects
the actual code, not a general description of RLE or Huffman coding — for
the conceptual "how and why," see [theory.md](theory.md).

All multi-byte integers are written with a raw `fwrite` of the in-memory
representation (`uint16_t`, `uint32_t`, `uint64_t`). There is no explicit
byte-order handling, so the format is **host-endian**: a file produced on a
little-endian machine is only guaranteed to decode correctly on a
little-endian machine.

## 1. Shared header

Every compressed file starts with a 5-byte header, written by `main.c`
before it delegates to either the RLE or the Huffman encoder:

| Offset | Size | Field   | Value                                  |
|-------:|-----:|---------|-----------------------------------------|
| 0      | 4    | magic   | ASCII `"CMP1"` (no trailing NUL on disk) |
| 4      | 1    | algo    | `0` = RLE, `1` = Huffman                 |

`main.c` decides the algorithm at compression time: it runs `rle_encode`
and `huffman_encode` against the same input in parallel, buffering each
into its own `tmpfile()`, compares the two resulting sizes, and copies
whichever is smaller after the 5-byte header. **The choice is per-file and
automatic** — there is no CLI flag to force one algorithm, and the header's
`algo` byte is the only record of which one was used.

On decompress, `main.c` reads and validates the 4-byte magic, reads the
`algo` byte, and dispatches to `rle_decode` or `huffman_decode` on the
remainder of the stream. An unrecognized magic or algo value is a hard
error (no output is produced).

Everything below this point is specific to the chosen algorithm and starts
immediately at offset 5.

## 2. RLE payload (`algo = 0`)

A flat sequence of `(symbol, count)` pairs, 2 bytes each, with no
additional header:

| Field  | Size | Notes                                  |
|--------|-----:|-----------------------------------------|
| symbol | 1    | the repeated byte value                 |
| count  | 1    | `uint8_t`, run length, range `[1, 255]`  |

Encoding (`rle_encode`) scans the input and merges consecutive identical
bytes into one pair, capping `count` at 255. **A run longer than 255 bytes
is split across multiple consecutive pairs** with the same `symbol` — e.g.
300 consecutive `0x41` bytes become `(0x41, 255)` followed by
`(0x41, 45)`. The cap exists because `count` is a single byte; without it,
`fputc` would silently truncate any larger value to its low 8 bits.

Decoding (`rle_decode`) reads pairs until EOF and writes `symbol` repeated
`count` times for each one. Pairs are always written in twos by the
encoder, so a well-formed stream's length is always even.

An empty input produces zero pairs — the RLE payload is empty and the
output file is just the 5-byte shared header.

## 3. Huffman payload (`algo = 1`)

### 3.1 Header

| Field         | Size          | Notes                                            |
|---------------|--------------:|---------------------------------------------------|
| original_size | 8 (`uint64_t`)| decompressed size in bytes                        |
| symbol_count  | 2 (`uint16_t`)| number of distinct byte values in the input        |

Followed by exactly `symbol_count` entries, one per distinct symbol that
appeared in the input:

| Field  | Size          | Notes                                    |
|--------|--------------:|--------------------------------------------|
| symbol | 1 (`uint8_t`) | the byte value                            |
| length | 1 (`uint8_t`) | code length in bits                       |
| code   | 4 (`uint32_t`)| code value, right-justified in the 32 bits |

This is enough to reconstruct the Huffman tree on decode: `huffman_decode`
rebuilds it by inserting each `(symbol, code, length)` entry as a root-to-leaf
path (`insert_code`), without ever serializing the tree shape itself.

Note the implied limit: `length` is a single byte, and `code` is 32 bits,
so a code longer than 32 bits cannot be represented. With at most 256
distinct symbols this is not reachable by a balanced merge in practice, but
a pathological, heavily skewed frequency distribution (Fibonacci-like
counts) can in principle produce a tree deeper than 32 levels, which this
format cannot encode correctly.

### 3.2 Bitstream

After the header, the entries are followed directly by the encoded data:
one Huffman code per original byte, back to back, with no padding between
codes.

Each code is written **most-significant-bit first** — i.e. in root-to-leaf
order, since `build_table` grows the code by shifting left as it descends
the tree. This is independent of how `bitio` packs those individual bits
into bytes: `bitwriter_write_bit` fills each byte **LSB-first** (bit 0 of
the byte is the first bit written). So the bit *order within a code* and
the bit *order within a packed byte* follow different conventions — one is
a property of the Huffman code itself, the other is an artifact of the
bit-packing implementation. A decoder must read bits with the matching
LSB-first byte convention (`bitreader_read_bit`) and feed them one at a
time into a root-to-leaf tree walk; it does not need to know code lengths
up front, since it stops descending the moment it reaches a leaf.

The final byte of the bitstream is padded with zero bits up to a full byte
(`bitwriter_flush`). The decoder never over-reads past the padding because
it stops after producing `original_size` symbols, not when the bitstream
runs out.

### 3.3 Edge cases

- **Empty input**: `original_size = 0`, `symbol_count = 0`, no symbol table
  entries, and no bitstream. The Huffman payload is exactly 10 bytes
  (8 + 2), following the 5-byte shared header.
- **Single distinct symbol**: normally a leaf's code is the root-to-leaf
  path, but a tree with only one symbol has no internal nodes to descend
  through, so the walk that assigns codes never runs. This case is
  special-cased to force a 1-bit code (`length = 1`, `code = 0`) for that
  symbol. The bitstream is then one `0` bit per occurrence of the symbol —
  e.g. a 1000-byte input containing only `0x41` compresses to a 10-byte
  header plus 125 bytes of bitstream (1000 bits, padded to a byte
  boundary).
