#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Algorithm 2.2.2G @ TAOCP::p.248
   author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
"""

import matplotlib.pyplot
import matplotlib.patches
import numpy
import threading
import algraph.tcolor
from algraph.vislist import *

DBG_PRINT=True

class AlgorithmG(ListAlgorithm):
    def __init__(self, matplotlib_wrapper):
        ListAlgorithm.__init__(self, 'TAOCP p248', matplotlib_wrapper)

        self.ge = GraphElements()

        # algorithm related below:
        self.L_0       = 0
        self.n         = 4      # nr. of stacks
        self.L_end     = 10     # memory size
        self.CONTENTS  = [0]        * (self.L_end + 1) # memory: CONTENTS[1] ~ CONTENTS[L_end]
        self.TOP       = [self.L_0] * (self.n + 1)     # TOPs  of stacks: TOP[1] ~ TOP[n]
        self.BASE      = [self.L_0] * (self.n + 2)     # BASEs of stacks: BASE[1] ~ BASE[n+1]

        self.BASE[self.n+1] = self.L_end

        # Algorithm G related
        self.SUM       = self.L_end - self.L_0
        self.INC       = 0
        self.NEWBASE   = [self.L_0] * (self.n + 1)
        self.OLDTOP    = [self.L_0] * (self.n + 1)     # OLDTOPs  of stacks: OLDTOP[1] ~ OLDTOP[n]
        self.D         = [0]        * (self.n + 1)
        self.alpha     = self.beta = 0.0

        # watch the variables
        self.ge.m_name = 'CONTENTS'
        self.ge.m_range = range(0, len(self.CONTENTS))
        for k in range(1, len(self.TOP)):
            self.ge.pointers.append('TOP[%d]' % k)
        for k in range(1, len(self.OLDTOP)):
            self.ge.pointers.append('OLDTOP[%d]' % k)
        for k in range(1, len(self.BASE)):
            self.ge.pointers.append('BASE[%d]' % k)
        for k in range(1, len(self.NEWBASE)):
            self.ge.pointers.append('NEWBASE[%d]' % k)
        print self.ge

        # for printing
        self.tc = algraph.tcolor.TColor()

    def prepare_fig(self):
        self.visualizer.nodes[self.TOP[0]].set_facecolor('yellow')

    def the_algorithm(self):
        self.show_and_suspend()
        self.push(1, 11)
        self.push(1, 12)
        self.push(4, 41)
        self.push(2, 21)
        self.pop(1)
        self.push(3, 31)
        self.push(1, 13)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(3, 32)
        self.show_and_suspend()
        self.pop(3)
        self.show_and_suspend()
        self.push(1, 14)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(3, 33)
        self.show_and_suspend()
        self.pop(3)
        self.show_and_suspend()
        self.push(1, 15)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(3, 34)
        self.show_and_suspend()
        self.pop(3)
        self.show_and_suspend()
        self.push(1, 16)
        self.show_and_suspend()
        self.pop(1)
        self.show_and_suspend()
        self.push(3, 35)
        self.show_and_suspend()
        self.pop(3)
        self.show_and_suspend()
        self.push(2, 22)
        self.show_and_suspend()
        self.pop(2)
        self.show_and_suspend()


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
        def G():
            def G1():
                """initialization"""
                self.SUM = self.L_end - self.L_0
                self.INC = 0
            def G2(j):
                """do three things:
                    - decrease  SUM  (=free memory)
                    - calculate D[j] (=ΔTOP[j])
                    - increase  INC  (=ΣD[]=ΣΔTOP[])
                """
                self.SUM -= self.TOP[j] - self.BASE[j]
                if self.TOP[j] > self.OLDTOP[j]:
                    self.D[j] = self.TOP[j] - self.OLDTOP[j] # D[]: ΔTOP[]
                    self.INC += self.D[j]
                else:
                    self.D[j] = 0
            def G3():
                """do nothing"""
                assert self.SUM >= 0
            def G4():
                """calculate α and β, so that
                      D[j-1] * β
                    = D[j-1] * 0.9 * (SUM / INC)
                    = D[j-1] * 0.9 * (SUM / ΣD[k])
                
                        D[j-1]
                    = ---------- * 0.9 * SUM
                        ΣD[k]
                """
                self.alpha = 0.1 * (self.SUM / self.n)
                self.beta  = 0.9 * (self.SUM / self.INC)
                if DBG_PRINT:
                    self.tc.color_print(('red',), 'α:', self.alpha)
                    self.tc.color_print(('red',), 'β:', self.beta)
            def G5():
                """calculate NEWBASE[]"""
                self.NEWBASE[1] = self.BASE[1]
                sigma = 0
                for j in range(2, self.n + 1):
                    if DBG_PRINT:
                        self.tc.color_print(('green',), '\n', 'j:', j, '-'*8)

                    # τ = σ + α + D[j-1] * β
                    tau = sigma + self.alpha + self.D[j-1] * self.beta
                    if DBG_PRINT:
                        self.tc.color_print(('green',), 'τ = σ + α + D[j-1] * β:', self.tc.gen_start_str(('Green',)),
                                            tau, '=', sigma, '+', self.alpha + self.D[j-1] * self.beta)

                    # NEWBASE[j] = NEWBASE[j-1] + (TOP[j-1] - BASE[j-1]) + ⌊τ⌋ - ⌊σ⌋
                    self.NEWBASE[j] = self.NEWBASE[j-1] + (self.TOP[j-1] - self.BASE[j-1]) + int(tau) - int(sigma)
                    if DBG_PRINT:
                        self.tc.color_print(('green',),
                                            'NEWBASE[%d] = NEWBASE[%d] + (TOP[%d] - BASE[%d]) + ⌊τ⌋ - ⌊σ⌋:' % (j, j-1, j-1, j-1),
                                            self.tc.gen_start_str(('Green',)), self.NEWBASE[j], '=',
                                            self.NEWBASE[j-1], '+ (', self.TOP[j-1], '-', self.BASE[j-1], ') +', int(tau), '-', int(sigma))
                    # σ = τ
                    sigma = tau
                if DBG_PRINT:
                    self.tc.color_print(('cyan',), '   BASE[]:', self.BASE)
                    self.tc.color_print(('cyan',), 'NEWBASE[]:', self.NEWBASE)
            def G6():
                """apply NEWBASE[]"""
                self.TOP[i] -= 1
                R()
                self.TOP[i] += 1
                for j in range(1, self.n + 1):
                    self.OLDTOP[j] = self.TOP[j]
                print 'NEWBASE[] applied'
                #self.show_and_suspend()
            ##############
            # Here begins:
            G1()
            for j in range(1, self.n + 1):
                G2(j)
            # at this time:
            #     SUM:  free memory
            #     D[k]: ΔTOP[k]
            #     INC:  ΣD[k]
            self.tc.color_print(('blue',), 'SUM:', self.SUM)
            self.tc.color_print(('blue',), 'D:',   self.D)
            self.tc.color_print(('blue',), 'INC:', self.INC)
            G3()
            G4()
            G5()
            G6()
            return True
        # END of G()

        def R():
            def R1():
                j = 1
            def R2():
                for j in range(1, self.n + 1):
                    if self.NEWBASE[j] < self.BASE[j]:
                        R3(j)
            def R3(j):
                self.delta = self.BASE[j] - self.NEWBASE[j]
                for L in range(self.BASE[j] + 1, self.TOP[j] + 1):
                    self.CONTENTS[L-self.delta] = self.CONTENTS[L]
                    self.CONTENTS[L] = 0
                self.BASE[j] = self.NEWBASE[j]
                self.TOP[j] -= self.delta
            def R4():
                for j in range(self.n, 0, -1):
                    if self.NEWBASE[j] > self.BASE[j]:
                        R5(j)
            def R5(j):
                self.delta = self.NEWBASE[j] - self.BASE[j]
                for L in range(self.TOP[j], self.BASE[j], -1):
                    self.CONTENTS[L+self.delta] = self.CONTENTS[L]
                    self.CONTENTS[L] = 0
                self.BASE[j] = self.NEWBASE[j]
                self.TOP[j] += self.delta
            ##############
            # Here begins:
            R1()
            R2()
            R4()
        # END of R()

        print "OVERFLOW"

        j = 0
        if G():
            return 'solved'
        else:
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
    t  = AlgorithmG(mw)
    t.start()

    # Generate the graph
    vl = VisList(mw, t)
    vl.show()

    # this line is needed when close the Matplotlib window:
    t.stop()

    print 'THE END'
