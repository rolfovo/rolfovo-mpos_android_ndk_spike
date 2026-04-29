import argparse
import subprocess
import sys
from pathlib import Path


def quote_qstr_lines(text):
    return "".join(quote_qstr_line(line) for line in text.splitlines(True))


def unquote_qstr_lines(text):
    return "".join(unquote_qstr_line(line) for line in text.splitlines(True))


def quote_qstr_line(line):
    text, newline = split_line(line)
    if text.startswith("Q("):
        text = f'"{text}"'
    return text + newline


def unquote_qstr_line(line):
    text, newline = split_line(line)
    if text.startswith('"Q('):
        end_quote = text.find('"', 1)
        if end_quote != -1:
            text = text[1:end_quote] + text[end_quote + 1:]
    return text + newline


def split_line(line):
    if line.endswith("\r\n"):
        return line[:-2], "\r\n"
    if line.endswith("\n"):
        return line[:-1], "\n"
    return line, ""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--input", action="append", required=True)
    parser.add_argument("flags", nargs=argparse.REMAINDER)
    args = parser.parse_args()

    flags = args.flags
    if flags and flags[0] == "--":
        flags = flags[1:]

    combined = ""
    for path in args.input:
        combined += Path(path).read_text()

    proc = subprocess.run(
        [args.compiler, "-E", *flags, "-"],
        input=quote_qstr_lines(combined),
        text=True,
        capture_output=True,
    )
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        sys.exit(proc.returncode)

    Path(args.output).write_text(unquote_qstr_lines(proc.stdout))


if __name__ == "__main__":
    main()
