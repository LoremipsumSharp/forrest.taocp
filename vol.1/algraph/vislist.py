#!/usr/bin/env python

import sys
import matplotlib.pyplot
import matplotlib.patches
import numpy
import threading

class NodeError(Exception):
    pass

class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(self, vector):
        return Point(self.x + vector.x, self.y + vector.y)

    def __sub__(self, vector):
        return Point(self.x - vector.x, self.y - vector.y)

    def __eq__(self, p):
        return (self.x == p.x and self.y == p.y)

class Vector(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __mul__(self, scale):
        return Vector(self.x * scale.x, self.y * scale.y)

    def __div__(self, scale):
        return Vector(self.x / scale.x, self.y / scale.y)

    def vectorx(self):
        return Vector(self.x, 0)

    def vectory(self):
        return Vector(0, self.y)

    def __eq__(self, vector):
        return (self.x == vector.x and self.y == vector.y)

class MatplotlibWrapper(object):
    fig = None
    ax  = None
    txt = None
    def __init__(self):
        if MatplotlibWrapper.fig == None:
            MatplotlibWrapper.fig = matplotlib.pyplot.figure() # Figure
        if MatplotlibWrapper.ax == None:
            MatplotlibWrapper.ax  = MatplotlibWrapper.fig.add_subplot(111)  # Axes

        MatplotlibWrapper.txt = MatplotlibWrapper.ax.text(
            0, 0,
            '',
            horizontalalignment='left',
            verticalalignment='bottom',
            fontsize=12,
            color='green'
            )

    def get_fig(self):
        return MatplotlibWrapper.fig

    def get_ax(self):
        return MatplotlibWrapper.ax

    def display(self, text):
        MatplotlibWrapper.txt.set_text(text)

    def show(self):
        matplotlib.pyplot.show()

class Node(object):
    def __init__(self, origin, vector, matplotlib_wrapper):
        self.southwest = origin
        self.northwest = origin + vector.vectorx()
        self.northeast = origin + vector
        self.southeast = origin + vector.vectory()

        self.north     = origin + vector / Vector(2,1)
        self.south     = origin + Vector(vector.x / 2, 0)
        self.west      = origin + Vector(0, vector.y / 2)
        self.east      = origin + vector / Vector(1,2)

        self.center    = origin + vector / Vector(2,2)

        self.width     = vector.x
        self.height    = vector.y

        self.annotation  = None
        self.text        = None
        self.texts       = []
        self.arrows      = []

        self.mw = matplotlib_wrapper

        self.__draw()

    def __draw(self):
        self.rect = matplotlib.patches.Rectangle(
            (self.southwest.x, self.southwest.y),
            width=self.width,
            height=self.height,
            facecolor='white'
            )
        self.mw.get_ax().add_patch(self.rect)

    def __arrow(self, f, t):
        return Node.ax.annotate("",
                                xy=(t.x, t.y), xycoords='data',
                                xytext=(f.x, f.y), textcoords='data',
                                arrowprops=dict(arrowstyle="->",
                                                connectionstyle="arc3"),
                                )

    def set_facecolor(self, c):
        self.rect.set_facecolor(c)

    def del_annotation(self):
        if self.annotation != None:
            self.mw.get_ax().texts.remove(self.annotation)
            self.annotation = None

    def set_annotation(self, text, pos, vector=Vector(0,0)):
        if self.annotation == None:
            default_dist = 10
            default_vector = {'north': Vector(0, default_dist),
                              'south': Vector(0,-default_dist),
                              'west' : Vector(-default_dist,0),
                              'east' : Vector( default_dist,0),
                              }
            align = {
                'north': ('center', 'bottom'),
                'south': ('center', 'top'),
                'west' : ('right',  'center'),
                'east' : ('left',   'center'),
                }

            if not default_vector.has_key(pos):
                raise NodeError

            if not align.has_key(pos):
                raise NodeError('invalid pos: '+pos)

            if vector == Vector(0,0):
                vector = default_vector[pos]

            p0 = eval('self.'+pos)
            p1 = p0 + vector

            a = self.mw.get_ax().annotate(text,
                                          xy=(p0.x, p0.y), xycoords='data',
                                          xytext=(p1.x, p1.y), textcoords='data',
                                          arrowprops=dict(arrowstyle="->",
                                                          connectionstyle="arc3"),
                                          horizontalalignment=align[pos][0],
                                          verticalalignment=align[pos][1],
                                          fontsize=8
                                          )
            self.annotation = a
        else:
            t = self.annotation.get_text()
            self.annotation.set_text(t+'\n'+text)

    def add_arrow(self, pos, fromto, vector):
        """
        fromto: 'from' or 'to'

        pos: west, east, north, south

            pos='west':                pos='east':
                          +---+                      +---+
                    <---> |   |                      |   | <--->
                          o---+                      o---+

            pos='north':               pos='south':
                            ^
                            |
                            v                        +---+
                          +---+                      |   |
                          |   |                      o---+
                          o---+                        ^
                                                       |
                                                       v
        """
        if not pos in ('west', 'east', 'north', 'south'):
            raise NodeError('invalid pos: '+pos)

        if not fromto in ('from', 'to'):
            raise NodeError('invalid fromto: '+fromto)
    
        p0 = eval('self.'+pos)
        p1 = p0 + vector

        if fromto == 'to':
            p0, p1 = p1, p0

        a = self.__arrow(p0, p1)
        self.arrows.append(a)

    def set_text(self, text):
        if self.text == None:
            self.text = self.mw.get_ax().text(self.center.x, self.center.y,
                                              text,
                                              horizontalalignment='center',
                                              verticalalignment='center',
                                              fontsize=8)
        else:
            self.text.set_text(text)

    def add_text(self, text, pos, vector):
        """
        pos='west':                pos='east':
                      +---+                      +---+      
                label |   |                      |   | label
                      o---+                      o---+      

        pos='north':               pos='south':
                      label                      +---+
                      +---+                      |   |
                      |   |                      o---+
                      o---+                      label
        """
        align = {'north': ('center', 'top'),
                 'south': ('center', 'bottom'),
                 'west' : ('left',   'center'),
                 'east' : ('right',  'center')
                 }
        if not align.has_key(pos):
            raise NodeError('invalid pos: '+pos)

        point = eval('self.'+pos) + vector
        t = self.mw.get_ax().text(point.x, point.y, text,
                                  horizontalalignment=align[pos][0],
                                  verticalalignment=align[pos][1],
                                  fontsize=8)
        self.texts.append(t)

class VisList(object):
    def __init__(self, matplotlib_wrapper, algorithm_thread):
        self.mw = matplotlib_wrapper
        self.t  = algorithm_thread

        self.nodes = {}         # memory_addr : Node
        self.node_size = Vector(20, 5)

        fig= self.mw.get_fig()
        ax = self.mw.get_ax()

        #              +---+---+- ... -+---+
        #              |   |   |  ...  |   |
        # M_origin --> +---+---+- ... -+---+
        self.M_origin  = Point(self.node_size.x, 80)
        m_range = self.t.ge.m_range
        ax.set_xlim(0,
                    (len(m_range) + 2) * self.node_size.x
                    )
        ax.set_ylim(0,100)

        for k in self.t.ge.m_range:
            node = Node(Point(self.M_origin.x + (k - m_range[0]) * self.node_size.x,
                              self.M_origin.y),
                        self.node_size,
                        self.mw)
            node.add_text(str(k), 'north', Vector(0,5))
            self.nodes[k] = node

        self.update()

        fig.canvas.mpl_connect('button_press_event', self.on_click)
        fig.canvas.mpl_connect('axes_enter_event', self.enter_axes)
        fig.canvas.mpl_connect('axes_leave_event', self.leave_axes)

        #ax.set_axis_off()
        algorithm_thread.set_visualizer(self)
        
    def update(self):
        for n in self.nodes.values():
            n.del_annotation()

        for ptext in self.t.ge.pointers:
            p = eval('self.t.%s' % ptext)
            self.nodes[p].set_annotation(ptext, 'south')

        for p in self.t.ge.m_range:
            content = eval('self.t.%s[%d]' % (self.t.ge.m_name, p))
            self.nodes[p].set_text(str(content))
            if content == 0:
                self.nodes[p].set_facecolor('white')
            else:
                self.nodes[p].set_facecolor('blue')

    def savefig(self, filename):
        matplotlib.pyplot.savefig(filename)

    def show(self):
        self.mw.show()

    def on_click(self, event):
        print 'click' + '-'*80
        self.t.resume()

    def enter_axes(self, event):
        self.mw.get_ax().set_axis_off()
        self.mw.get_fig().canvas.draw()

    def leave_axes(self, event):
        self.mw.get_ax().set_axis_on()
        self.mw.get_fig().canvas.draw()

class GraphElements(object):
    def __init__(self):
        self.m_range   = []
        self.m_name    = ''
        self.pointers  = []
        #self.values    = []

    def __str__(self):
        s = 'the following are visible in the graph:'
        stuff = '-' * ((80 - len(s)) / 2)
        s = stuff + s + stuff
        s += '\nmemory range:\n\t' + str(self.m_range)
        rmin = self.m_range[0]
        rmax = self.m_range[-1]
        s += '\nmemory:\n\t%s[%d~%d]' % (self.m_name, rmin, rmax)
        s += '\npointers:'
        for p in self.pointers:
            s += '\n\t'+str(p)
        s += '\n'+'-'*80+'\n'
        return s

class ListAlgorithm(threading.Thread):
    """
    Usage:

        class AlgorithmBlah(ListAlgorithm):
            def __init__(self, matplotlib_wrapper):
                ListAlgorithm.__init__(self, 'Algorithm Blah', matplotlib_wrapper)

                self.ge = GraphElements()

                # algorithm related below:
                self.FOO = ...
                self.BAR = ...

                # watch the variables
                self.ge.m_range = ...
                self.ge.m_name  = ...
                for k in ...: self.ge.pointers.append(...)
                print self.ge

            def prepare_fig(self):
                self.visualizer.nodes[0].set_facecolor('yellow')

            def the_algorithm(self):
                ...
                self.show_and_suspend()
                ...
                self.show_and_suspend()
                ...

        # Matplotlib Wrapper
        mw = MatplotlibWrapper()

        # The Algorithm Thread
        t  = AlgorithmBlah(mw)
        t.start()

        # Generate the graph
        vl = VisList(mw, t)
        vl.show()

        # this line is needed when close the Matplotlib window:
        t.stop()

    """
    figno = 0
    def __init__(self, tname, matplotlib_wrapper):
        threading.Thread.__init__(self, name=tname)
        self.e = threading.Event()
        self.mw = matplotlib_wrapper
        self.quitting = False
        self.visualizer = None

    def set_visualizer(self, v):
        self.visualizer = v
        self.prepare_fig()

    def prepare_fig(self):
        pass

    def the_algorithm(self):
        """should be overloaded by subclasses"""
        pass

    def run(self):
        self.suspend()
        self.the_algorithm()

        if not self.quitting:
            self.visualizer.update()
            self.mw.display('finished')
            self.mw.get_fig().canvas.draw()
        print self.name, 'finished.'

    def suspend(self):
        if not self.quitting:
            #self.mw.display('waiting...')
            self.mw.get_fig().canvas.draw()
            self.e.wait()
        if not self.quitting:
            #self.mw.display('running...')
            self.e.clear()

    def resume(self):
        self.e.set()

    def show_and_suspend(self):
        if not self.quitting:
            fig_name = '%s (%03d).png' % (self.name, ListAlgorithm.figno)
            self.mw.display(fig_name)
            self.visualizer.update()
            self.suspend()
            self.visualizer.savefig(fig_name)
            print "%s saved" % fig_name
            ListAlgorithm.figno += 1

    def stop(self):
        self.resume()
        self.quitting = True

class AlgorithmBlah(ListAlgorithm):
    def __init__(self, matplotlib_wrapper):
        ListAlgorithm.__init__(self, 'Algorithm Blah', matplotlib_wrapper)

        self.ge = GraphElements()

        # algorithm related below:
        self.n         = 4      # nr. of stacks
        self.L_end     = 20     # memory size
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
        self.visualizer.nodes[0].set_facecolor('yellow')

    def the_algorithm(self):
        self.TOP[1] = 5
        self.show_and_suspend()
        self.CONTENTS[8] = 99
        self.show_and_suspend()
        self.BASE[2] = 10
        #self.show_and_suspend()

if __name__ == '__main__':
    print 'BEGIN'

    # Matplotlib Wrapper
    mw = MatplotlibWrapper()

    # The Algorithm Thread
    t  = AlgorithmBlah(mw)
    t.start()

    # Generate the graph
    vl = VisList(mw, t)
    vl.show()

    # this line is needed when close the Matplotlib window:
    t.stop()

    print 'THE END'

