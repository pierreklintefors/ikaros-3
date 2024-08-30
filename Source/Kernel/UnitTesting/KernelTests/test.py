"""
test.py

Run a set of Ikaros unit tests in the same directory as this code
test files must start with "test" and end with ".ikg"

"""

import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

bold = "\033[1m"
red = "\033[31m"
reset = "\033[0m"


if __name__ == "__main__":

    def get_description(file_path):
        tree = ET.parse(file_path)
        root = tree.getroot()
        value = root.get("description")
        return '' if value is None else value+" – "

    print(f"\n{bold}Running Ikaros Unit Tests{reset}\n")
    errors = 0
    file_count = 0
    current_directory = Path(__file__).resolve().parent
    ikaros_binary = current_directory / "../../../../Bin/ikaros_d"
    for item in current_directory.iterdir():
        if item.name.startswith("test") and item.suffix.lower() == '.ikg':
            file_count+=1
            result = subprocess.run( f"{ikaros_binary} -s0 -b {item}", shell=True, text=True, capture_output=True)
            if result.returncode == 0:
                print(f"[  OK  ]  {get_description(item)}{item.name}{reset}")
            else:
                print(f"{red}{bold}[ FAIL ]  {get_description(item)}{item.name}{reset}")
                errors += 1

    if errors > 0:
        print(f"\n{red}{bold}***** Ikaros failed {errors} tests *****{reset}\n")
    else:
        print(f"\nIkaros passed all {file_count} tests\n")
