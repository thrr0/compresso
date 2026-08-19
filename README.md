![logo](./assets/logo.png)

---

`compresso` is a small lossless file compressor written in C, built as a
learning project. It implements Run-Length Encoding and Huffman coding from
scratch, with no external compression libraries.

## Building

```
make
```

This produces a `compresso` binary in the repo root (built with
`-fsanitize=address,undefined` per the `Makefile`, so it's a debug build).

## Usage

```
./compresso compress <input> <output>
./compresso decompress <input> <output>
```

Example:

```
./compresso compress report.txt report.sso
./compresso decompress report.sso report.restored.txt
```

## How it picks an algorithm

There is no flag to choose RLE or Huffman. On every `compress`, both
encoders run against the input in parallel (each buffered into its own
`tmpfile()`), the two output sizes are compared, and whichever is smaller
is written to `<output>`, prefixed with a small header that records which
algorithm was used. Decompression reads that header and dispatches to the
matching decoder automatically — you never need to know which one was
picked.

For the reasoning behind RLE and Huffman, how the codes are built, and why
this is lossless, see [docs/theory.md](docs/theory.md). For the exact byte
layout of compressed files, see [docs/format-spec.md](docs/format-spec.md).

## Known limitations

- **Return values of `fread`/`fwrite` are not checked.** If a compressed
  file is corrupted or truncated, the decoder can read short or garbage
  header fields and keep going — it fails silently instead of reporting an
  error, and the decompressed output may be wrong without any indication
  that something went wrong.
- **Compressing a file onto itself destroys the input.** `compress` opens
  the output path with `fopen(..., "wb")`, which truncates it, before the
  encoders read from the input. If `<input>` and `<output>` are the same
  path, the input is emptied before it's ever read, so the "compressed"
  result is just an encoding of nothing.

## To-do


- [ ] **Folder compression.** Right now `compress`/`decompress` only take a
  single file. 
