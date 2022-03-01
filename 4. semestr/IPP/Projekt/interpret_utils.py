# IPP project, task 2, part 1: interpret.py
# File:   interpret_utils.py
# Author: Ondřej Ondryáš (xondry02@stud.fit.vubr.cz)
# Date:   2021-04-11

import sys


def eprint(*args, **kwargs):
    """Wrapper for print() that outputs to stderr."""
    print(*args, file=sys.stderr, **kwargs)
