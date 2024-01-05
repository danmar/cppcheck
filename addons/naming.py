#!/usr/bin/env python3
#
# cppcheck addon for naming conventions
#
# namingng.py was made backward compatible with naming.py; call through.

import sys
import cppcheckdata
import namingng

namingng.main()
sys.exit(cppcheckdata.EXIT_CODE)
