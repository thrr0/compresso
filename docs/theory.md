# What is a file?
In simple terms, all files are a finite (and large) succession of 1's and 0's that represents information.
# Why do we even need compression?
Compressing a file is essentially trying to represent the same information with less bits (1's and 0's).
Files typically feature patterns and symbols that appear more often than others.
There are several ways to go around these patterns:
``` 
// -- RLE (repetition-based): --
000110000
// can be represented as
{3}0 {2}1 {4}0

// -- Huffman (frequency-based):
Simbols are represented based on their frequency
Given the text: "AAAAABBC":
'A' appears 5 times -> code: 0
'B' appears 2 times -> code: 10
'C' appears 1 time -> code: 11

```

But not all files are *redundant*. Some feature more patterns than others.

Compression can be done to the extent that there is redundancy in a file (i.e lossless compression) or can be done losing certain information undistinguishable from human sight/hearing (lossy compression; used in images/audio).

# How are the codes generated?
Huffman builds a binary tree from the bottom up. It repeatedly takes the two symbols (or partial trees) with the lowest frequency, merges them under a new node, and repeats until only one node is left (the root).

Each symbol's code is just the path from the root to its leaf: left branch is 0, right branch is 1.

# Why can this be decoded without separators?
No code is ever the start of another one. This is called a prefix-free code, and it comes straight from how the tree is built: a leaf can never sit above another leaf, so no complete code can be the beginning of a longer one.
That's why the decoder doesn't need separators between symbols. It reads bit by bit, walks the tree from the root, and knows it hit a complete symbol the moment it reaches a leaf.

# What is the theoretical limit of compression?
Files can't be compressed forever. There's a lower bound called entropy: the minimum average number of bits needed per symbol, based on how often each symbol shows up (its probability distribution).

A truly random file (every byte equally likely, no patterns) has entropy close to 8 bits per byte, so it's basically incompressible. A skewed file (some symbols way more common than others, like English text) has lower entropy, so there's room to compress.

No lossless algorithm beats this bound on average, Huffman included. Huffman gets close, but not exact, since it only assigns whole-bit codes (1 bit, 2 bits...) while true entropy can require fractional bits per symbol.
