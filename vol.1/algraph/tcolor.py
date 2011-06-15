#!/usr/bin/env python

import sys

class TColor(object):
    def __init__(self):
        self.CSI = '\033['
        self.d = {
            # Text attributes
            'attr_off'      : '0', # All attributes off
            'bold'          : '1', # Bold on
            'underscore'    : '4', # Underscore (on monochrome display adapter only)
            'blink'         : '5', # Blink on
            'reverse_video' : '7', # Reverse video on
            'conceal'       : '8', # Concealed on

            # Foreground colors
            'black'   : '30',
            'red'     : '31',
            'green'   : '32',
            'yellow'  : '33',
            'blue'    : '34',
            'magenta' : '35',
            'cyan'    : '36',
            'white'   : '37',
            'grey'    : '37',
            # Foreground colors with bold on
            'Black'   : '1;30',
            'Red'     : '1;31',
            'Green'   : '1;32',
            'Yellow'  : '1;33',
            'Blue'    : '1;34',
            'Magenta' : '1;35',
            'Cyan'    : '1;36',
            'White'   : '1;37',
            'Grey'    : '1;37',
                                     
            # Background colors
            'BLACK'   : '40',
            'RED'     : '41',
            'GREEN'   : '42',
            'YELLOW'  : '43',
            'BLUE'    : '44',
            'MAGENTA' : '45',
            'CYAN'    : '46',
            'WHITE'   : '47',
            }

    def gen_start_str(self, color_tuple):
        s = self.CSI
        for x in color_tuple:
            s += self.d[x] + ';'
        return s[:-1] + 'm'

    def gen_end_str(self):
        return self.CSI + self.d['attr_off'] + 'm'

    def color_print(self, *args):
        color_tuple = args[0]
        stuff_tuple = args[1:]

        sys.stdout.write(self.gen_start_str(color_tuple))
        for x in stuff_tuple:
            print x,
        print self.gen_end_str()

if __name__ == '__main__':
    print 'black'

    tc = TColor()

    tc.color_print(('black',), 'black')
    tc.color_print(('red',), 'red')
    tc.color_print(('green',), 'green')
    tc.color_print(('yellow',), 'yellow')
    tc.color_print(('blue',), 'blue')
    tc.color_print(('magenta',), 'magenta')
    tc.color_print(('cyan',), 'cyan')
    tc.color_print(('white',), 'white')

    tc.color_print(('Black',), 'black')
    tc.color_print(('Red',), 'red')
    tc.color_print(('Green',), 'green')
    tc.color_print(('Yellow',), 'yellow')
    tc.color_print(('Blue',), 'blue')
    tc.color_print(('Magenta',), 'magenta')
    tc.color_print(('Cyan',), 'cyan')
    tc.color_print(('White',), 'white')

    tc.color_print(('Green','WHITE',), 'GreenWHITE')

    # print '\033[1;30mGray like Ghost\033[1;m'
    # print '\033[1;31mRed like Radish\033[1;m'
    # print '\033[1;32mGreen like Grass\033[1;m'
    # print '\033[1;33mYellow like Yolk\033[1;m'
    # print '\033[1;34mBlue like Blood\033[1;m'
    # print '\033[1;35mMagenta like Mimosa\033[1;m'
    # print '\033[1;36mCyan like Caribbean\033[1;m'
    # print '\033[1;37mWhite like Whipped Cream\033[1;m'
    # print '\033[1;38mCrimson like Chianti\033[1;m'
    # print '\033[1;41mHighlighted Red like Radish\033[1;m'
    # print '\033[1;42mHighlighted Green like Grass\033[1;m'
    # print '\033[1;43mHighlighted Brown like Bear\033[1;m'
    # print '\033[1;44mHighlighted Blue like Blood\033[1;m'
    # print '\033[1;45mHighlighted Magenta like Mimosa\033[1;m'
    # print '\033[1;46mHighlighted Cyan like Caribbean\033[1;m'
    # print '\033[1;47mHighlighted Gray like Ghost\033[1;m'
    # print '\033[1;48mHighlighted Crimson like Chianti\033[1;m'

    print 'black'

