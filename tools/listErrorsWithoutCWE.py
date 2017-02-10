#!/usr/bin/python

try:
    import argparse
    import xml.etree.ElementTree as ET
    import httplib
    import urllib
    import sys

except ImportError as ImportErrorMsg:
    print("Unable to find needed modules.: {}".format(ImportErrorMsg))
    sys.exit(0)


def main():
    parser = argparse.ArgumentParser(description="List all error without a CWE assigned in CSV format")
    parser.add_argument("-F", metavar="filename", required=True,
                        help="XML file containing output from: ./cppcheck --errorlist --xml-version=2")
    parsed = parser.parse_args()

    try:
        tree = ET.parse(vars(parsed)["F"])
        root = tree.getroot()
    except ET.ParseError as ETerrorMsg:
        print("Unable to parse XML file {}: {}".format(vars(parsed)["F"], ETerrorMsg))
        sys.exit(0)

    try:
        lines = str()
        for child in root.iter("error"):
            if "cwe" not in child.attrib:
                lines += ("{}, {}, {}".format(child.attrib["id"], child.attrib["severity"], child.attrib["verbose"] + "\n"))

        params = urllib.urlencode({"data": lines})
        headers = {"Content-type": "application/x-www-form-urlencoded", "Accept": "text/plain"}
        conn = httplib.HTTPSConnection("usa.boos.core-dumped.info:443")
        conn.request("POST", "/cgi-bin/cppcheck-cwe-mapping-check", params, headers)
        conn.close()

    except Exception:
        print("Unable to parse XML file {}: {}".format(vars(parsed)["F"], error))
        sys.exit(0)


if __name__ == "__main__":
    main()
