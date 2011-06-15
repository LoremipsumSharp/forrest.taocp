#!/usr/bin/env python
"""Algorithm J @ TAOCP::p.177
   author: Forrest Y. Yu <forrest.yu@gmail.com>, http://forrestyu.net/
"""

import sys
import re
import StringIO
import pygraphviz as pgv
from algraph.algraphviz import Algraphviz

class AlgorithmJ(Algraphviz):
    """Algorithm J @ TAOCP::p.177
       see Algraphviz.__doc__
    """

    def __init__(self, X):
        Algraphviz.__init__(self, "TAOCP - Algorithm J", ['m', 'i', 'j'])

        self.X       = [0] + X
        self.X0      = self.X[:]
        self.n       = len(X)
        self.m       = 0
        self.i       = 0
        self.j       = 0

        self.indices = range(1, self.n+1)
        self.done    = False

    def more_edge_attr(self, k):
        if self.X[k] < 0:
            if abs(self.X0[k]) == abs(self.X[k]):
                return {}
            else:
                return {'style' : 'dashed'}
        else:
            return {'color' : "red"}

    def __J1(self):
        for k in self.indices:
            self.X[k] = -self.X[k]
        self.m = self.n
        self.i = self.j = 0

    def __J2(self):
        self.j = self.m

    def __J3(self):
        while True:
            self.i = self.X[self.j]
            if self.i > 0:
                self.j = self.i
            else:
                break

    def __J4(self):
        self.X[self.j] = self.X[-self.i]
        self.X[-self.i] = self.m

    def __J5(self):
        self.m -= 1
    
    def get_result(self):
        if not self.done:
            print "no result yet. (please run() first)"
            return None
        else:
            return self.X[1:]

    def run(self, Fig=False):
        self.__J1()
        while self.m > 0:
            self.prt("J1/J5 - J2")
            self.__J2()
            self.prt("J2 - J3")
            self.__J3()
            self.prt("J3 - J4")
            self.__J4()
            self.prt("J4 - J5")
            self.__J5()
        self.prt("done")
        self.done = True


if __name__ == '__main__':
    #J = AlgorithmJ([6, 2, 1, 5, 4, 3])
    J = AlgorithmJ([3, 4, 5, 6, 2, 1])
    #J = AlgorithmJ([2, 3, 4, 5, 6, 7, 1])
    #J = AlgorithmJ([1, 3, 4, 6, 5, 2, 7])
    #J = AlgorithmJ([3, 7, 4, 6, 2, 5, 1])
    #J = AlgorithmJ([7, 3, 1, 5, 6, 2, 4])
    #J = AlgorithmJ([4, 6, 1, 5, 9, 8, 3, 7, 2])
    #J = AlgorithmJ([8, 7, 6, 5, 9, 2, 1, 4, 3])
    #J = AlgorithmJ([8, 7, 5, 6, 9, 2, 1, 3, 4])

    J.run(True)
    print "result:"
    print J.get_result()

