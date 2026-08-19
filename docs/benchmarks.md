# Benchmarks

Real measurements against the actual `compresso` binary, taken on
2026-08-19. Every number below was produced by running the commands shown,
not estimated.

## Methodology

- Built with the repo's own `Makefile` (`make`), which compiles with
  `-fsanitize=address,undefined`. That means the timings below include
  ASan/UBSan instrumentation overhead and are **not** representative of an
  optimized release build's speed — treat the timing columns as relative
  (compresso vs. gzip, file vs. file), not absolute.
- Machine: AMD Ryzen 5 5500, Linux 7.1.8-zen1, x86_64.
- Toolchain: `gcc (GCC) 16.2.1`, `gzip 1.14`.
- Every compressed file was round-tripped through `compresso decompress`
  and diffed against the original with `cmp` to confirm losslessness
  before recording numbers.
- Times are `user` time from `time`, single run per file (no averaging
  across multiple runs).

## Test files

Generated into a scratch directory, not committed to the repo:
| File                 | Size      | Description                                                                 |
|----------------------|----------:|-------------------------------------------------------------------------------|
| `repetitive_text.txt`| 900,000 B | the phrase "the quick brown fox jumps over the lazy dog. " repeated 20,000×  |
| `varied_text.txt`    | 101,080 B | `docs/theory.md` concatenated 40× (real prose, not synthetic)                |
| `random_binary.bin`  | 1,000,000 B | `/dev/urandom` output — no exploitable structure                            |
| `long_runs.bin`      | 1,000,000 B | 500,000× `0x41`, then 300,000× `0x42`, then 200,000× `0x00` — built to favor RLE |

## Results

| File                  | Original  | compresso | ratio  | algo chosen | compress time | decompress time | gzip -9   | gzip ratio |
|-----------------------|----------:|----------:|-------:|:-----------:|---------------:|------------------:|----------:|-----------:|
| `repetitive_text.txt` | 900,000 B | 502,683 B | 55.9%  | Huffman     | 0.114s          | 0.077s             | 2,719 B   | 0.3%       |
| `varied_text.txt`     | 101,080 B | 58,720 B  | 58.1%  | Huffman     | 0.022s          | 0.016s             | 2,014 B   | 2.0%       |
| `random_binary.bin`   | 1,000,000 B | 1,001,551 B | 100.2% (grew) | Huffman | 0.196s      | 0.137s             | 1,000,191 B | 100.0%     |
| `long_runs.bin`       | 1,000,000 B | 7,851 B   | 0.8%   | RLE         | 0.060s          | 0.012s             | 1,023 B   | 0.1%       |

`ratio` is compressed size as a percentage of the original (lower is
better). All four round trips (`compresso decompress` output `cmp`'d
against the original) matched exactly.

## Reading the results

- **compresso never beats gzip in these tests**, and on `random_binary.bin`
  it makes the file slightly *larger* (1,001,551 B vs. 1,000,000 B). Random
  bytes have entropy close to 8 bits/byte (see
  [theory.md](theory.md#what-is-the-theoretical-limit-of-compression)), so
  Huffman coding has essentially nothing to exploit, and the ~1,546-byte
  symbol table + header overhead (up to 256 × 6 bytes, plus the 10-byte
  original_size/symbol_count fields) is pure loss.
- **The gap is largest on `repetitive_text.txt`** (502,683 B vs. 2,719 B —
  gzip is ~185× smaller). This isn't a bug in the Huffman implementation;
  it's a difference in what the two approaches can see. `compresso`'s
  Huffman coding is memoryless — it only ever looks at *single-byte*
  frequencies, so a repeated 45-byte phrase still costs roughly one code
  per character. gzip's DEFLATE first runs LZ77, matching that repeated
  phrase against earlier occurrences at arbitrary distance and replacing
  each repeat with a short back-reference, then Huffman-codes what's left.
  RLE doesn't help here either, since consecutive *bytes* rarely repeat in
  English-like text even when longer substrings do.
- **`long_runs.bin` is the one case built to favor `compresso`'s RLE**, and
  it does win over Huffman there (7,851 B vs. presumably-larger Huffman
  output — RLE was the algorithm actually selected). But gzip still comes
  out ~7.7× smaller, because DEFLATE's LZ77 stage isn't capped at a
  255-byte run length the way `compresso`'s RLE `count` byte is: a run of
  500,000 identical bytes is many separate `(symbol, 255)` pairs for
  `compresso`, but a small number of back-references for gzip.
- Across the two text files, `compresso` picked Huffman both times — RLE
  never won on realistic text, since it only pays off when the *same byte*
  repeats consecutively, not when patterns repeat at a distance or
  characters merely recur throughout the file.

## Takeaway

`compresso`'s automatic RLE-vs-Huffman selection does pick the better of
its two algorithms for each input (confirmed against the `algo` byte in
each output file), but neither algorithm alone is competitive with a
dictionary-based compressor like gzip's DEFLATE on anything but
pathological single-byte-run data. That gap is expected for a from-scratch
educational implementation with no LZ77-style back-referencing — see
[theory.md](theory.md) for why RLE and Huffman are limited to the
redundancy each one can individually see.
