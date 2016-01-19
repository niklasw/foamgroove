#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import time,re,string,sys,os

class Patterns:
    rexFloat = '([+-]?\d*[.]?\d+(e[-+]\d+)?)'
    rexWord = '(\w+)'

    instantPat = re.compile('^(Time) = *%s' % rexFloat)
    solverPat = re.compile('Solving for %s,.*Initial residual = %s,'% (rexWord, rexFloat))
    continuityPat = re.compile('time step %s errors.*sum local = %s,'% (rexWord, rexFloat))
    crashPat = re.compile('Foam::sigFpe::sigHandler')
    monitorPat = re.compile('Monitor %s .* = %s'% (rexWord, rexFloat))


class Reader:

    def __init__(self, fileName):
        self.__dict__.update(locals())
        del self.__dict__['self']

        self.curPos = 0
        self.instants = []
        self.curInstant = 0
        self.crashDetected = False

        self.residualDict = {}
        self.monitorDict = {}
        try:
            self.fh = open(self.fileName,'r')
        except:
            print 'Could open log file'

    def parse(self):
        try:
            self.fh.seek(self.curPos)
            for line in self.fh:
                parseMsg = self.parseString(line)
                if parseMsg == 1:
                    self.curPos = self.fh.tell()-len(line)
                elif parseMsg == 666:
                    self.crashDetected = True
                    break
        except:
            print 'Could not read log file'

    def parseString(self, line):
        t = self.searchLine(line,Patterns.instantPat)
        a = self.searchLine(line,Patterns.solverPat)
        c = self.searchLine(line,Patterns.continuityPat)
        m = self.searchLine(line,Patterns.monitorPat)
        if t:
            name, self.curInstant = t
            self.residualDict[self.curInstant] = {}
            return 1
        if a or c:
            key, value = a if a else c
            if not self.residualDict[self.curInstant].has_key(key):
                self.residualDict[self.curInstant][key] = value
            return 2
        if m:
            return 0
            key, value = m
            if not self.monitorDict[self.curInstant].has_key(key):
                self.monitorDict[self.curInstant][key] = value
            return 10

        if self.crashLine(line):
            return 666
        return 0

    def searchLine(self,astring,pattern, groups = (1,2)):
        found = pattern.search(astring)
        if found:
            name,value = (found.group(i) for i in groups)
            return name,float(value)

    def crashLine(self,astring):
        found = Patterns.crashPat.search(astring)
        if found:
            return True
        else:
            return False

class Plotter:
    def __init__(self):
        self.instants = np.array([])
        self.dataArrays = {}

        self.first = True
        self.plotLines = list()

        plt.ion()
        self.figure = plt.figure()
        self.ax = self.figure.add_subplot(111)

    def legend(self):
        return tuple(sorted(self.dataArrays.keys()))

    def updateData(self,residualDict):
        instants = np.array(sorted(residualDict.keys()))[len(self.instants):]
        # Drop last instant, since this will be read again
        # in order to keep instants and data aligned...
        instants = instants[0:-1]

        for instant in instants:
            valueDict = residualDict[instant]

            for key,value in valueDict.iteritems():
                if not self.dataArrays.has_key(key):
                    self.dataArrays[key] = np.array([])

                data = self.dataArrays[key]
                self.dataArrays[key] = np.append(data,np.array([value]))

        self.instants = np.append(self.instants, instants)

    def redraw(self):
        self.figure.canvas.draw()

    def updatePlot(self,static=False):
        if self.first:
            self.first = False

            for item in self.legend():
                #line, = self.ax.semilogy(self.instants, self.dataArrays[item])
                line, = plt.semilogy(self.instants, self.dataArrays[item])
                self.plotLines.append(line)

            plt.legend(self.legend(),loc=3)
            plt.grid('on')
            if static:
                plt.ioff()
                plt.show()
                return
            else:
                self.figure.canvas.draw()
        else:
            for i,item in enumerate(self.legend()):
                plt.legend(self.legend(),loc=3)

                self.plotLines[i].set_xdata(self.instants)
                self.plotLines[i].set_ydata(self.dataArrays[item])

            self.ax.relim()
            self.ax.autoscale_view(True,True,True)

            self.figure.canvas.draw()

    def warning(self, astring = 'Warning'):
        txt = self.figure.suptitle(astring, fontsize=18)

def run(filen, static=False):
    print 'Abort with Ctrl-c'
    reader = Reader(filen)
    plot = Plotter()

    dataUpdateInterval = 5 #s

    try:
        while True:
            curFilePos = reader.curPos
            reader.parse()

            if reader.residualDict and reader.curPos != curFilePos:
                plot.updateData(reader.residualDict)
                plot.updatePlot(static)
                if static: sys.exit(0)

            if reader.crashDetected:
                plot.warning('Alas! solver has crashed')

            for i in range(10):
                plot.redraw()
                time.sleep(dataUpdateInterval/10.0)

    except (KeyboardInterrupt,SystemExit):
        imageName = os.path.splitext(filen)[0]
        print '\nExit. Saving current state to %s.png' % imageName
        plt.savefig(imageName+'.png',format='png')
        plt.close('all')
        sys.exit(0)

if __name__ == "__main__":
    static = (len(sys.argv) > 2)
    print static
    run(sys.argv[1],static)

