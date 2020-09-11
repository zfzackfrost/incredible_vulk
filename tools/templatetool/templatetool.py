#!/usr/bin/env python3

"""Python script to process Jinja2 template
files and print to STDOUT.
"""

import argparse
import json
import sys
import os.path

from ttool import get_env

def process_args():
    """Process command line arguments
    """
    parser = argparse.ArgumentParser(
        prog="templatetool.py",
        description="Script to process Jinja2 template files and print them to STDOUT",
    )
    parser.add_argument(
        "input", help="Specify the Jinja2 template file to process", type=str
    )
    parser.add_argument(
        "--output", '-o', help="Specify the output file path", type=str, default=None,
    )
    parser.add_argument(
        "--context", "-c", default="{}", help="The Jinja2 context as JSON code.",
    )
    parser.add_argument(
        "--include",
        "-I",
        action="append",
        help="Add a directory to the template search path",
    )
    return parser.parse_args()


def process_templates(template_path, include_paths, context):
    """Process template file
    """
    env = get_env(include_paths)
    return env.get_template(template_path).render(context)


def main():
    """Main function"""
    args = process_args()
    input_path = os.path.relpath(args.input.rstrip('/'), os.getcwd())
    try:
        context = json.loads(args.context)
    except json.JSONDecodeError as err:
        print(err.msg)
        return 1

    include_paths = list(args.include) + [os.path.dirname(input_path), os.getcwd()]
    include_paths = [os.path.normpath(os.path.abspath(p)) for p in include_paths]

    if not isinstance(context, dict):
        print("Context root must be a dictionary!")
        return 1
    result = process_templates(str(input_path), include_paths, context)
    if args.output is not None:
        os.makedirs(os.path.dirname(args.output), exist_ok=True)
        with open(args.output, 'w') as wfile:
            wfile.write(result)
    else:
        print(result)

    return 0


if __name__ == "__main__":
    sys.exit(main())
