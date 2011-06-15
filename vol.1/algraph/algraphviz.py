#!/usr/bin/env python

import sys
import re
import decimal
import pygraphviz as pgv

class Algraphviz(object):
    """
    The name ``Algraphviz'' comes from ``ALGorithm'' and ``GRAPH''.

    Usage:
        class AlgorithmFoo(Algraphviz):
            def __init__(self, X):
                Algraphviz.__init__(self, "AlgorithmFoo", ['m', 'i', 'j'])

                self.X              = [0] + X
                self.n              = len(X)
                self.m              = 0
                self.i              = 0
                self.j              = 0
                self.indices        = range(1, self.n+1)

            def more_edge_attr(self, k):
                if self.X[k] < 0:
                    return "black"
                else:
                    return "red"
            
            def run(self, Fig=False):
                ...
                self.prt("running")
                ...
                self.prt("done")

    Notes:
        * `self.X' is a list, like this:
                `self.X' = [0, 3, 7, 4, 6, 2, 5, 1]        ,
          in which 0 is just a placeholder
        * real data in `self.X' is `self.X[1:]' (0 excluded)
        * `self.X[1:]' is a permutation of `range(1,len(self.X)-1)'
                e.g. [3, 7, 4, 6, 2, 5, 1] is a permutation of [1, 2, 3, 4, 5, 6, 7]
        * every `k' is a node
        * every `k -> X[k]' is an edge
        * more attr of edge `k -> X[k]' is determined by `self.more_edge_attr(k)'
        * `self.extra' is a list which contains vars appearing in the graph, like this:
                self.extra = ['m', 'i', 'j']
        * every element in `self.extra' should be a class attribute of AlgorithmFoo
                for example:
                    self.extra = ['m', 'i', 'j']
                means there are `self.m', `self.i', `self.j' in class AlgorithmFoo
        * there's one extra node in the graph, which contains element values of `self.extra'
        * a node has two parts, like this:
                  .------------------------ left part
                  |       .---------------- right part
                  V       |
                +-----+   V
                |     |------+
                |     | m=3  |
                |  3  |      |
                |     | j=-3 |
                |     |------+
                +-----+
        * an extra node looks like:
                +-----+
                | m=1 |
                +-----+
                | i=2 |
                +-----+
                | j=3 |
                +-----+

    Refs:
        http://networkx.lanl.gov/pygraphviz/contents.html
        http://www.graphviz.org/doc/info/attrs.html
        http://www.graphviz.org/doc/info/shapes.html
        http://www.graphviz.org/Gallery.php
    """

    def __init__(self, name, extra):
        # subclass will override self.X and self.indices:
        self.X               = []
        self.indices         = []

        self.FigNo           = 0
        self.node_pos        = {} # str : str
        self.ratio           = decimal.Decimal("0.0139")
        self.extra           = extra
        self.extra_node_name = 'vars'

        self.algorithm_name  = name

    def __extra_text(self):
        """return a list like this:
                ['m=2', 'i=-7', 'j=3']
        """
        var_l = []
        for x in self.extra:
            var_l.append('%s=%d' % (x, eval('self.'+x)))
        return var_l

    def __add_nodes(self, G):
        # nodes:
        for k in self.indices:
            G.add_node(k)

            # the right part
            var_stuff = ''
            for x in self.__extra_text():
                if re.sub(r'^.*=-*', '', x) == str(k):
                    s = x
                else:
                    s = ' '
                var_stuff += '<FONT FACE="monospace" POINT-SIZE="8">'+s+'</FONT>'
                var_stuff += '<BR ALIGN="LEFT"/>'

            # the left part (with a reserved slot for the right part)
            node_label = re.sub(r'\n *', '',
                                """<
                          <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0">
                              <TR>
                                  <TD ROWSPAN="3" BGCOLOR="yellow" WIDTH="20">
                                      <FONT FACE="monospace" POINT-SIZE="22">%(label)s</FONT>
                                  </TD>
                              </TR>
                              <TR>
                                  <TD PORT="here" BGCOLOR="lightblue" FIXEDSIZE="TRUE" WIDTH="30" HEIGHT="35">
                                      %(varstuff)s
                                  </TD>
                              </TR> 
                          </TABLE>
                          >
                          """)
            # the whole node
            node_label = node_label % {'label' : str(k), 'varstuff' : var_stuff}

            # update the node attr
            node_k = G.get_node(k)
            node_k.attr['label'] = node_label

        # extra-node:
        G.add_node(self.extra_node_name)
        # update the extra-node attr
        node_var = G.get_node(self.extra_node_name)
        node_var.attr['shape'] = 'record'
        node_var.attr['color'] = 'blue'
        node_var.attr['fontsize'] = 8
        node_var.attr['label'] = ' | '.join(self.__extra_text())

    def __add_edges(self, G):
        # the main graph
        for k in self.indices:
            s = str(k)
            t = str(abs(self.X[k]))
            G.add_edge(s, t)
            e = G.get_edge(s, t)
            for (k,v) in self.more_edge_attr(k).items():
                if e.attr[k] != None and e.attr[k] != '': # `if k in e.attr' doesn't work for unknown reason
                    print >> sys.stderr, 'Warning: ', k, '=', e.attr[k], ' is overwritten by ', k, '=', v, '.'
                e.attr[k] = v

        # the extra-node
        G.add_edge(self.extra_node_name, self.indices[0], style="invis")

    def __save_nodes_pos(self, G):
        for n in G.nodes():
            self.node_pos[str(n)] = ','.join(str(decimal.Decimal(x)*self.ratio) for x in n.attr['pos'].split(','))+'!'

    def __load_nodes_pos(self, G):
        for (n,p) in self.node_pos.items():
            node = G.get_node(n)
            node.attr['pos'] = p

    def more_edge_attr(self, k):
        """this function should be public because subclasses will override it
        """
        print >> sys.stderr, 'subclass of Algraphviz should override more_edge_attr()'
        assert False

    def prt(self, title):
        A=pgv.AGraph(directed=True)

        A.graph_attr.update(fontname="DejaVu Sans Mono",
                            fontsize=12,
                            rankdir="LR",
                            label=r'\n'+self.algorithm_name+r'\n'+title)
        A.node_attr.update(fontsize=10, shape="box")

        self.__add_nodes(A)

        if self.FigNo == 0:
            graphviz_prog = 'circo'
        else:
            self.__load_nodes_pos(A)
            graphviz_prog = 'neato'

        self.__add_edges(A)

        filename = self.algorithm_name+' ('+str(self.FigNo).rjust(2,'0')+')'
        A.write(filename+'.dot')

        A.layout(prog=graphviz_prog)
        A.draw(filename+'.png')
        print filename+'.png'

        if self.FigNo == 0:
            self.__save_nodes_pos(A)

        self.FigNo += 1

###
### Uncomment class AlgorithmJ to debug class Algraphviz
###
# class AlgorithmJ(Algraphviz):
#     """Algorithm J @ TAOCP::p.177
#        see Algraphviz.__doc__
#     """

#     def __init__(self, X):
#         Algraphviz.__init__(self, "AlgorithmJ", ['m', 'i', 'j'])

#         self.X  = [0] + X
#         self.X0 = self.X[:]
#         self.n  = len(X)
#         self.m  = 0
#         self.i  = 0
#         self.j  = 0

#         self.indices = range(1, self.n+1)
#         self.done    = False

#     def more_edge_attr(self, k):
#         if self.X[k] < 0:
#             if abs(self.X0[k]) == abs(self.X[k]):
#                 return {}
#             else:
#                 return {'style' : 'dashed', 'color' : 'blue'}
#         else:
#             return {'color' : "red"}

#     def __J1(self):
#         for k in self.indices:
#             self.X[k] = -self.X[k]
#         self.m = self.n
#         self.i = self.j = 0

#     def __J2(self):
#         self.j = self.m

#     def __J3(self):
#         while True:
#             self.i = self.X[self.j]
#             if self.i > 0:
#                 self.j = self.i
#             else:
#                 break

#     def __J4(self):
#         self.X[self.j] = self.X[-self.i]
#         self.X[-self.i] = self.m

#     def __J5(self):
#         self.m -= 1
    
#     def get_result(self):
#         if not self.done:
#             print "no result yet. (please run() first)"
#             return None
#         else:
#             return self.X[1:]

#     def run(self, Fig=False):
#         self.__J1()
#         self.prt("between J1 and J2")
#         while self.m > 0:
#             self.__J2()
#             self.__J3()
#             self.prt("between J3 and J4")
#             self.__J4()
#             self.__J5()
#         self.prt("done")
#         self.done = True


if __name__ == '__main__':
    print Algraphviz.__doc__

    ###
    ### Uncomment to debug class Algraphviz
    ###
    # if True:
    #     J = AlgorithmJ([3, 7, 4, 6, 2, 5, 1])
    # else:
    #     J = AlgorithmJ([6, 2, 1, 5, 4, 3])
    #     J = AlgorithmJ([3, 4, 5, 6, 2, 1])
    #     J = AlgorithmJ([2, 3, 4, 5, 6, 7, 1])
    #     J = AlgorithmJ([1, 3, 4, 6, 5, 2, 7])
    # J.run(True)
    # print "result:"
    # print J.get_result()

