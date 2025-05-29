

![Finn McMissile](https://github.com/Sujal-Ladde/Finn_McMissile-The_LSB_staegnographer/blob/main/Assets/Finn_Mcmissile.png)
### Finn McMissile:
"I never properly introduced myself. Finn McMissile, British Intelligence."

### Mater:
"Tow Mater, average intelligence."


# LSB Steganography in C

A command‑line utility for hiding (“encoding”) any file within a 24‑bit BMP image by manipulating its least significant bits—and later recovering (“decoding”) it without visibly altering the carrier.

---

## Table of Contents

1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Building](#building)
4. [Usage](#usage)

   * [Encoding](#encoding)
   * [Decoding](#decoding)
5. [How It Works](#how-it-works)
6. [Project Structure](#project-structure)
7. [Error Handling](#error-handling)
8. [License](#license)
9. [Author](#author)

---

## Features

* **Universal payload**: Embed any file type (text, binary, image, etc.) into a 24‑bit uncompressed BMP.
* **Magic‑string protection**: Require a user‑supplied password to decode the hidden data.
* **Automatic metadata**: Store file extension and exact size for faithful recovery.
* **Capacity check**: Prevent encoding if the carrier image lacks sufficient LSB capacity.
* **Modular codebase**: Separate encode/decode logic and utility functions for easy extension.

---

## Prerequisites

* A C99‑compatible compiler (e.g., `gcc`)
* GNU Make
* A 24‑bit uncompressed BMP image to use as carrier

---

## Building

```bash
# Compile sources and produce executable `a.out`
make

# Clean up object files and executables
make clean
```

---

## Usage

### Encoding

```bash
./a.out -e <SOURCE_IMAGE.bmp> <SECRET_FILE> [OUTPUT_STEGO_IMAGE.bmp]
```

* `<SOURCE_IMAGE.bmp>`
  Path to the 24‑bit BMP carrier image.
* `<SECRET_FILE>`
  File to embed (any type).
* `[OUTPUT_STEGO_IMAGE.bmp]` (optional)
  Name of the generated stego image. Defaults to `stego.bmp`.

**Example**

```bash
./a.out -e lena.bmp secret.txt lena_stego.bmp
```

### Decoding

```bash
./a.out -d <STEGO_IMAGE.bmp> [OUTPUT_BASE_NAME]
```

* `<STEGO_IMAGE.bmp>`
  BMP containing hidden data.
* `[OUTPUT_BASE_NAME]` (optional)
  Base name for the recovered file; original extension is appended. Defaults to `secret`.

**Example**

```bash
./a.out -d lena_stego.bmp recovered
# Prompts for magic password, then writes `recovered.txt`
```

---

## How It Works

1. **Capacity Check**

   * Read image dimensions (width & height) from BMP header.
   * Compute available bits: `width × height × 3` bytes → bits.
   * Ensure space for header (54 bytes), metadata (magic string, extension, size), and payload.

2. **Header Copy**

   * Copy first 54 bytes of the BMP header unchanged so the stego image remains valid.

3. **Embed Metadata**

   * **Magic string**: prompt user → store length (1 byte) + each character’s bits in LSBs.
   * **Extension**: extract secret file’s extension → store length (1 byte) + characters.
   * **File size**: store as 4‑byte little‑endian integer → embed 32 bits.

4. **Embed Payload**

   * Read secret file byte‑by‑byte → for each byte, embed its 8 bits into 8 image bytes’ LSBs.

5. **Finalize**

   * Copy any remaining image data (padding, metadata) unchanged.

6. **Decoding**

   * Reverse process: skip header → extract and verify magic string → read extension & size → reconstruct payload.

---

## Project Structure

```
.
├── data/
│   ├── encode_input/       # Sample BMPs to encode
│   ├── encode_output/      # Generated stego images
│   ├── decode_input/       # Stego images to decode
│   └── decode_output/      # Recovered secret files
├── include/                # Public headers
│   ├── common.h            # Utility function prototypes
│   ├── encode.h            # EncodeInfo struct & prototypes
│   ├── decode.h            # DecodeInfo struct & prototypes
│   └── types.h             # Shared enums and typedefs
├── main/                   # Entry point and CLI parsing
│   └── main.c
├── src/                    # Core implementation
│   ├── common.c            # Utility functions
│   ├── encode.c            # Encoding routines
│   └── decode.c            # Decoding routines
├── Makefile                # Build and clean rules
└── README.md               # This documentation
```

---

## Error Handling

* Exits with clear messages if:

  * Files cannot be opened.
  * Carrier capacity is insufficient.
  * Magic string mismatch during decode.
* Ensures all open file handles are closed on error.

---

## Author

**Sujal Ladde** • [laddesujal273@gmail.com](mailto:laddesujal273@gmail.com)

Happy hiding! 🚀


