# arcdir_tool

Yet another C tool for packing and extracting paired `.arc` / `.dir` archives (Pikmin 1 format).

## Build

```bash
make
````

## Usage

```bash
arcdir_tool EXTRACT <archive.arc> <index.dir> [<src> <dst>]...
arcdir_tool PACK <archive.arc> <index.dir> [path...]
arcdir_tool PACK_BIN <archive.arc> <index.dir> [path...]
```

## Examples

```bash
# Extract all files
arcdir_tool EXTRACT data.arc data.dir

# Extract single file to custom path
arcdir_tool EXTRACT data.arc data.dir a/b.bin out.bin

# Pack directory
arcdir_tool PACK data.arc data.dir assets/

# Pack only .bin files from directories
arcdir_tool PACK_BIN data.arc data.dir assets/
```

## Documentation

See `docs/arcdir_format.tex` for full format description and implementation details.