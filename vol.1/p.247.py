#!/usr/bin/env python
"""author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
   see TAOCP::p.247
"""

import matplotlib.pyplot
import matplotlib.patches
import numpy
import threading
from algraph.vislist import *

class AlgorithmP247(ListAlgorithm):
    def __init__(self, matplotlib_wrapper):
        ListAlgorithm.__init__(self, 'TAOCP p247', matplotlib_wrapper)

        self.ge = GraphElements()

        # algorithm related below:
        self.n         = 4      # nr. of stacks
        self.L_end     = 10     # memory size
        self.CONTENTS  = [0] * (self.L_end + 1) # memory: CONTENTS[1] ~ CONTENTS[L_end]
        self.TOP       = [0] * (self.n + 1)     # TOPs  of stacks: TOP[1] ~ TOP[n]
        self.BASE      = [0] * (self.n + 2)     # BASEs of stacks: BASE[1] ~ BASE[n+1]

        self.BASE[self.n+1] = self.L_end

        # watch the variables
        self.ge.m_name = 'CONTENTS'
        self.ge.m_range = range(0, len(self.CONTENTS))
        for k in range(1, len(self.TOP)):
            self.ge.pointers.append('TOP[%d]' % k)
        for k in range(1, len(self.BASE)):
            self.ge.pointers.append('BASE[%d]' % k)
        print self.ge

    def prepare_fig(self):
        self.visualizer.nodes[self.TOP[0]].set_facecolor('yellow')

    def the_algorithm(self):
        self.show_and_suspend()
        self.push(1, 11)
        self.show_and_suspend()
        self.push(1, 12)
        self.show_and_suspend()
        self.push(4, 41)
        self.show_and_suspend()
        self.push(2, 21)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(3, 31)
        self.show_and_suspend()
        self.push(1, 13)
        self.show_and_suspend()
        self.push(1, 14)
        self.show_and_suspend()
        self.push(2, 22)
        self.show_and_suspend()
        self.push(4, 42)
        self.show_and_suspend()
        self.pop(2)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(4, 43)
        self.show_and_suspend()
        self.push(4, 44)
        self.show_and_suspend()
        self.push(4, 45)
        self.show_and_suspend()
        self.push(4, 46)
        self.show_and_suspend()
        # self.push(3, 93)
        # self.show_and_suspend()
        # self.push(3, 93)
        # self.show_and_suspend()
        # self.push(3, 93)
        # self.show_and_suspend()
        # self.push(4, 93)
        # self.show_and_suspend()
        # self.push(4, 93)
        # self.show_and_suspend()
        # self.pop(4)
        # self.show_and_suspend()
        # self.pop(4)
        # self.show_and_suspend()
        # self.pop(4)
        # self.show_and_suspend()
        # self.pop(4)
        # self.show_and_suspend()
        # self.pop(4)
        # self.show_and_suspend()
        # self.pop(1)
        # self.show_and_suspend()
        # self.pop(1)
        # self.show_and_suspend()
        # self.pop(1)
        # self.show_and_suspend()

    def overflow(self, i):
        print "OVERFLOW"

        for k in range(i+1, self.n+1): # i < k <= n
            if self.TOP[k] < self.BASE[k+1]:
                # print '\t%d < %d <= %d' % (i,k,self.n)
                # print '\tTOP[%d] < BASE[%d+1]' % (k,k)
                # print '\tCONTENTS(L+1) <- CONTENTS(L), for TOP[%d] >= L > BASE[%d+1]' % (k,i)
                # # TOP[k] >= L > BASE[i+1]
                for L in range(self.TOP[k], self.BASE[i+1], -1):
                    self.CONTENTS[L+1] = self.CONTENTS[L]
                for j in range(i+1,k+1):
                    self.BASE[j] += 1
                    self.TOP[j] += 1
                return 'solved'

        for k in range(i-1,0,-1):   # i > k >= 1
            if self.TOP[k] < self.BASE[k+1]:
                # print '\t%d > %d >= 1' % (i,k)
                # print '\tTOP[%d] < BASE[%d+1]' % (k,k)
                # print '\tCONTENTS(L-1) <- CONTENTS(L), for BASE[%d+1] < L <= TOP[%d]' % (k,i)
                # # BASE[k+1] < L <= TOP[i]
                # print 'BASE[%d+1] =' % k, self.BASE[k+1]
                # print 'TOP[%d]' % i, self.TOP[i]
                # print range(self.BASE[k+1]+1, self.TOP[i]+1)
                for L in range(self.BASE[k+1]+1, self.TOP[i]):
                    self.CONTENTS[L-1] = self.CONTENTS[L]
                    self.CONTENTS[L] = 0
                for j in range(k+1,i+1):
                    self.BASE[j] -= 1
                    self.TOP[j] -= 1
                return 'solved'

        print "no room left"
        return 'unsolved'

    def underflow(self, i):
        print "UNDERFLOW"

    def push(self, i, Y):
        print 'push(%d, %d)' % (i, Y)
        self.TOP[i] = self.TOP[i] + 1
        if self.TOP[i] > self.BASE[i+1]:
            if self.overflow(i) == 'unsolved':
                return

        self.CONTENTS[self.TOP[i]] = Y

    def pop(self, i):
        print 'pop(%d)' % i
        if self.TOP[i] == self.BASE[i]:
            self.underflow(i)
        else:
            Y = self.CONTENTS[self.TOP[i]]
            self.CONTENTS[self.TOP[i]] = 0
            self.TOP[i] = self.TOP[i] - 1
            return Y

if __name__ == '__main__':
    print 'BEGIN'

    # Matplotlib Wrapper
    mw = MatplotlibWrapper()

    # The Algorithm Thread
    t  = AlgorithmP247(mw)
    t.start()

    # Generate the graph
    vl = VisList(mw, t)
    vl.show()

    # this line is needed when close the Matplotlib window:
    t.stop()

    print 'THE END'
