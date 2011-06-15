#!/usr/bin/env python
"""Algorithm 1.1E @ TAOCP::p.2
   author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
"""

import sys

def GCD(m, n):
    r = m % n
    # print '{0:5d} {1:5d} {2:5d}'.format(m, n, r)
    print repr(m).rjust(5) + repr(n).rjust(5) + repr(r).rjust(5)
    if r == 0:
        return n
    else:
        return GCD(n, r)

if __name__ == '__main__':
    m = int(raw_input('input the 1st integer: '))
    n = int(raw_input('input the 2nd integer: '))
    print 'm'.rjust(5) + 'n'.rjust(5) + 'r'.rjust(5)
    GCD(m, n)
    print '         |'
    print '         `- GCD'

