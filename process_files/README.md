# Cppcheck-xml-parser

The script named "add_author_information.py" takes a cppcheck report xml file and creates a copy with added information such as author name, author mail and date when an error was created into a new xml file. This additional information can be found at the location of an error. This information is read from git so the source codes must be in git. Type "add_author_information.py --help" to get more information.

To display xml file with additional author data use "create_csv_with_author". This script reads the newly created xml file and creates a new csv file with all information that can be found in xml. To get more information use "create_csv_with_author.py --help".
