# Tted - Terminal Text Editor

A sed-inspired command-line utility for viewing and manipulating text files with a safety-first approach.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![GitHub repo](https://img.shields.io/badge/repo-Tted-blue.svg)](https://github.com/yourusername/Tted)

## Features

- 📖 **View** specific lines or line ranges
- ✂️ **Delete** lines from files
- ➕ **Append** text after specific lines
- 🔄 **Replace** text throughout files
- 🛡️ **Safe preview mode** (default)
- ⚡ **In-place editing** with `-i` flag
- 🎯 **Sed-inspired** syntax for familiar workflow

## Table of Contents

- [Installation](#installation)
- [Operating Modes](#operating-modes)
- [Usage](#usage)
  - [Reading Operations](#reading-operations)
  - [Modification Operations](#modification-operations)
- [Examples](#examples)
- [Error Handling](#error-handling)
- [Technical Details](#technical-details)
- [Best Practices](#best-practices)
- [Contributing](#contributing)
- [License](#license)

## Installation

### Compilation

```bash
gcc -o tted rtt.c -Wall -Wextra
```

### Recommended Flags

```bash
gcc -o tted rtt.c -Wall -Wextra -O2 -std=c99
```

## Operating Modes

### 🔍 Terminal Mode (Default)

**Safe preview mode** - Shows what would happen without modifying files.

```bash
./tted --delete 5,10 file.txt  # Preview only
```

> ⚠️ **Safety First**: All modification commands run in preview mode by default!

### ✏️ In-Place Edit Mode

**File modification mode** - Actually modifies files using the `-i` flag.

```bash
./tted -i --delete 5,10 file.txt  # Modifies file
```

## Usage

### Reading Operations

These operations always work in terminal mode (read-only).

#### View Single Line

```bash
./tted --line <line_number> <file>
```

**Example:**
```bash
./tted --line 5 myfile.txt
```

#### View Line Range

```bash
./tted --lines <start>,<end> <file>
```

**Example:**
```bash
./tted --lines 10,20 myfile.txt
```

**Output:**
```
 10:This is line 10
 11:This is line 11
 12:This is line 12
 ...
```

### Modification Operations

#### Delete Lines

| Mode | Command |
|------|---------|
| **Preview** | `./tted --delete <start>,<end> <file>` |
| **Execute** | `./tted -i --delete <start>,<end> <file>` |

**Examples:**
```bash
# Preview deletion
./tted --delete 5,10 document.txt

# Actually delete lines 5-10
./tted -i --delete 5,10 document.txt
```

#### Append Text

| Mode | Command |
|------|---------|
| **Preview** | `./tted --append <line> <file> "text"` |
| **Execute** | `./tted -i --append <line> <file> "text"` |

**Examples:**
```bash
# Preview append
./tted --append 3 notes.txt "This is a new line"

# Actually append
./tted -i --append 3 notes.txt "This is a new line"
```

#### Replace Text

| Mode | Command |
|------|---------|
| **Preview** | `./tted --replace "old/new" <file>` |
| **Execute** | `./tted -i --replace "old/new" <file>` |

**Examples:**
```bash
# Preview replacement
./tted --replace "hello/goodbye" letter.txt

# Actually replace
./tted -i --replace "hello/goodbye" letter.txt
```

> 📝 **Note**: Replace operates on the first occurrence per line.

## Examples

### Basic Workflow

```bash
# 1. View the current state
./tted --lines 1,50 config.txt

# 2. Preview a deletion
./tted --delete 10,15 config.txt

# 3. If satisfied, apply the change
./tted -i --delete 10,15 config.txt

# 4. Verify the result
./tted --lines 1,50 config.txt
```

### Batch Operations

```bash
# Preview all changes first
./tted --delete 5,10 data.txt
./tted --append 20 data.txt "New section header"
./tted --replace "v1.0/v2.0" data.txt

# Then apply them
./tted -i --delete 5,10 data.txt
./tted -i --append 20 data.txt "New section header"
./tted -i --replace "v1.0/v2.0" data.txt
```

### Real-World Example

```bash
# Remove old configuration section
./tted -i --delete 25,40 app.conf

# Add new configuration
./tted -i --append 24 app.conf "# New Configuration Section"
./tted -i --append 25 app.conf "debug_mode=true"

# Update version string
./tted -i --replace "version=1.0/version=2.0" app.conf
```

## Error Handling

### Common Errors

| Error | Description |
|-------|-------------|
| `invalid line number` | Line number exceeds file length |
| `invalid line range` | Start > end, or range exceeds file |
| `Invalid range: Operation is not permitted` | Line number is 0 or out of bounds |
| `Error: Cannot open file` | File doesn't exist or lacks permissions |
| `Error: Cannot create temporary file` | Insufficient permissions or disk space |
| `Error: Text to append is required` | Missing text argument for append |
| `Error: Replace format should be "OldText/NewText"` | Missing or incorrect separator |

## Technical Details

### Line Numbering

- Line numbers **start at 1** (not 0)
- Ranges are **inclusive**: `5,10` includes both lines 5 and 10
- Maximum line number: 999,999,999 (unsigned int limit)

### Buffer Limitations

- **Maximum line length**: 512 characters (including newline)
- **Line number length in commands**: 8 digits max
- Lines exceeding buffer will be processed in chunks

### In-Place Mode Process

1. Read from original file
2. Write modified content to `temp_file.tmp`
3. Close both files
4. Remove original file
5. Rename temporary file to original filename

> ⚠️ **Warning**: Interrupted operations may leave `temp_file.tmp` in the current directory.

## Best Practices

### Safety Guidelines

- ✅ Always preview changes in terminal mode first
- ✅ Keep backups of important files before using `-i` mode
- ✅ Test operations on sample files before production use
- ✅ Use version control systems for critical files
- ✅ Verify file permissions before in-place editing

### Performance Tips

- For large files, specify exact line ranges rather than reading entire files
- Batch multiple preview operations before applying changes
- Consider file size when performing replace operations (operates line-by-line)

## Quick Reference

| Command | Mode | Effect |
|---------|------|--------|
| `--line N file` | Terminal | View line N |
| `--lines N,M file` | Terminal | View lines N-M |
| `--delete N,M file` | Terminal | Preview deletion |
| `-i --delete N,M file` | In-place | Delete lines |
| `--append N file "text"` | Terminal | Preview append |
| `-i --append N file "text"` | In-place | Append text |
| `--replace "a/b" file` | Terminal | Preview replace |
| `-i --replace "a/b" file` | In-place | Replace text |

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development

```bash
# Clone the repository
git clone https://github.com/yourusername/Tted.git

# Compile with debug symbols
gcc -g -o tted read.c -Wall -Wextra

# Run tests
./test.sh
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the classic Unix `sed` (stream editor) tool
- Built with safety and usability in mind
- Designed for developers who value preview-before-execute workflows

---

**Made with ❤️ for developers who value safety and simplicity**

*Tted - A sed-inspired Terminal Text Editor*
