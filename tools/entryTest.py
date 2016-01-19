#!/usr/bin/env python

import sys

def error(s,sig=1):
    print '\nError %s!\n' % s
    sys.exit(sig)

def warn(s):
    output = 'Warning %s!' % s
    print '\n'+'='*len(output)
    print output
    print '='*len(output)

def info(s):
    print '\t%s' % s


class List(list):
    def __init__(self, s='(0 0 0)', typ=float):
        self.string = s
        self.type = typ
        l = [self.type(a) for a in self.string.strip('() ').split()]
        list.__init__(self,l)

    def __str__(self):
        return '(%s)' % (' '.join(map(str,self)))

def strToClass(astring):
    classDict = {'float':float,
                 'str':str,
                 'List':List,
                 'int':int}
    return classDict[astring]

class Configuration:

    def __init__(self, confFile, incFile):
        from ConfigParser import ConfigParser
        self.confFile = confFile
        self.incFile = incFile
        self.parser = ConfigParser()
        self.parser.read(self.confFile)
        print self.incFile

    def getEntryNames(self):
        return self.parser.sections()

    def getEntry(self,name):
        p = self.parser
        if p.has_section(name):
            return dict(p.items(name))
        else:
            return False

    def generateIncludeFile(self,fileName=''):
        fh = ( open(fileName,'w') if fileName else sys.stdout )
        for item in self.getEntryNames():
            e = entry(item)
            e.fromDict(self.getEntry(item))
            fh.write(e.__str__())

class IncludeFile:
    def __init__(self,configuration):
        self.conf = configuration
        self.name = configuration.incFile

    def readlines(self):
        with open(self.name,'r') as fh:
            for line in fh:
                yield line

    def getEntryNames(self):
        import string
        for line in self.readlines():
            name, data = string.split(line,maxsplit=1)
            if not name in self.conf.getEntryNames():
                print 'Entry name %s is not defined by configuration' % name
            yield name

    def lookup(self, entryName):
        e = entry(entryName)
        defaultEntry = self.conf.getEntry(entryName)
        if defaultEntry:
            for line in self.readlines():
                e.fromDict(defaultEntry)
                if e.name == line.split()[0]:
                    e.fromString(line)
                else:
                    continue
                if not e:
                    print 'Could not lookup entry %s' % entryName
                    return False
                break
        else:
            print 'Could not get defaultEntry for %s' % (entryName,)
            return False
        return e

    def write(self,entries):
        with open(self.name,'w') as fh:
            ( fh.write(e) for e in entries )


class entry:
    strClassMap = {'float':float,
                   'str':str,
                   'List':List,
                   'int':int}

    def __init__(self,name,valueType='', valueClass=int):
        self.name = name
        self.valueType = valueType
        self.valueClass = valueClass
        self.value=self.valueClass()

    def fromDict(self,dictionary):
        self.valueType = '' if dictionary['type'] == 'default' else dictionary['type']
        self.valueClass = self.strClassMap[dictionary['class']]
        self.setValue(dictionary['defaultvalue'])

    def fromString(self, line):
        line=line.split(';')[0]
        columns = line.split()
        if len(columns) > 1:
            name=columns.pop(0)
            self.name = name
            if self.valueType:
                columns.pop(0)
            self.setValue( ' '.join(columns) )
        else:
            print 'Could not create entry from string:\n>> %s' % line
            return False

    def setValue(self, value):
        try:
            self.value = self.valueClass(value)
        except:
            info('Tried to set entry value to type %s' % self.valueClass)
            error('Could not set value for "%s" to "%s". Type error.' % (self.name,value))

    def __str__(self):
        return '%30s%20s%20s;\n' % (self.name, str(self.valueType), self.value.__str__())

class entryDict(dict):
    def __init__(self, d={}):
        dict.__init__(self,d)

    def __str__(self):
        return ''.join([ e.__str__() for e in self.sortedList() ])

    def assertKey(self,name):
        if not self.has_key(name):
            error('Entry %s not registred' % name)
        else:
            pass

    def set(self,name,value):
        self.assertKey(name)
        self[name].setValue(value)

    def get(self,name):
        self.assertKey(name)
        return self[name]

    def lookup(self,inc,names):
        for name in names:
            ent = inc.lookup(name)
            if ent:
                self[name] = ent

    def sortedList(self,reverse=False):
        from operator import attrgetter
        return sorted(self.values(),key = attrgetter('name'))

if __name__=='__main__':
    includeFileName = 'simpleSetup.test'
    configurationFileName = 'foamGroove.conf'

    configuration = Configuration(configurationFileName, includeFileName)

    entryNames = configuration.getEntryNames()

    configuration.generateIncludeFile()

    inc = IncludeFile(configuration)
    print '--------------------------------------------------'

    entries = entryDict()

    names=inc.getEntryNames()
    entries.lookup(inc,names)
    entries.set('initialVelocity','(1 0 1.2)')
    entries.set('flowrate',2e-3)
    print entries

