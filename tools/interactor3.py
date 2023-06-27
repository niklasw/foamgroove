#!/usr/bin/env python

import os,sys

class Interactors(dict):
    def __init__(self,description="Some helpful info"):
        dict.__init__(self)
        import argparse
        descString = description
        self.parser = argparse.ArgumentParser(description=descString)

    def add(self,interactor,name,letter):
        interactor.setName(name,letter)
        if type(interactor) == type(BooleanSelector()):
            self.parser.add_argument('-'+letter,'--'+name,
                    dest=name,action='store_true',default=interactor.default,
                    help=interactor.help)
        else:
            self.parser.add_argument('-'+letter,'--'+name,
                    dest=name,default=interactor.default,
                    help=interactor.help)
        self[name] = interactor

    def parseArgs(self):
        self.arguments = self.parser.parse_args()

    def getAll(self):
        self.parseArgs()
        argumentDict = vars(self.arguments)
        for name,interactor in self.iteritems():
            if interactor.default == argumentDict[name]:
                interactor.get()
            else:
                if type(interactor) == type(BooleanSelector()):
                    interactor.selected = Bool(argumentDict[name])
                else:
                    interactor.selected = argumentDict[name]

    def __str__(self):
        s = '\n'.join([str(i) for i in self.values() ])
        return s


class Interactor:

    def __init__(self, prompt='Input',default=None):
        self.__dict__.update(locals())
        del self.__dict__['self']
        self.selected = None
        self.help = 'No help'
        self.name = 'No name'
        self.letter = '-'

    def setName(self,name,letter):
        self.name = name
        self.letter = letter

    def error(self,s,sig=1):
        print '\nError %s!\n' % s
        sys.exit(sig)

    def warn(self,s):
        output = 'Warning %s!' % s
        print '\n'+'='*len(output)
        print output
        print '='*len(output)

    def info(self,s):
        print '\t%s' % s

    def __str__(self):
        return str(self.name)+' = '+str(self.selected)

class ValueSelector(Interactor):
    def __init__(self,prompt='Enter value',default=10,test=int,allowed=[],showAllowed=False):
        Interactor.__init__(self, prompt=prompt, default=default)
        self.__dict__.update(locals())
        del self.__dict__['self']

        self.add()

    def add(self):
        pass

    def get(self):
        selected = self.default
        try:
            selected = raw_input('%s [%s]: '% (self.prompt,self.default))
            if not selected:
                selected = self.default

            try:
                selected = self.test(selected)
            except:
                self.warn('Typo: %s is not %s' % (str(selected),self.test.__name__))
                selected = self.get()

            if self.allowed and not self.test(selected) in self.allowed:
                self.warn('Not allowed %s'%(selected,))
                selected = self.get()

        except (KeyboardInterrupt,SystemExit):
            self.error('Execution aborted by user')

        self.selected = selected
        return self.selected

class Bool:
    def __init__(self, val):
        falses = ['n','N','no','0']
        accepted = falses+['y','Y','1','yes',True,False]
        if not str(val) in accepted:
            raise Exception('Cant init Bool from '+str(val))
        self.value=True
        if val in falses:
            self.value=False

    def __str__(self):
        return ('Y' if self.value else 'N')


class BooleanSelector(object,ValueSelector):
    def __init__(self,prompt='Yes or no?',default='N'):
        ValueSelector.__init__(self,prompt=prompt,default=default,test=str,allowed=['y','n','Y','N'])
        self.__dict__.update(locals())
        del self.__dict__['self']

    def get(self):
        self.selected = super(self.__class__,self).get()
        if self.selected.lower() in ['y','yes']:
            self.selected = 'y'
        else:
            self.selected = 'n'

        return self.selected

    def __str__(self):
        return str(self.name)+' = '+str(self.selected)

class FileSelector(Interactor):
    def __init__(self,default='',prompt='Select from above', path='',suffix='*',folder=False):
        Interactor.__init__(self, prompt=prompt, default=default)
        self.__dict__.update(locals())
        del self.__dict__['self']

        self.errors = []
        self.selectables = []

        self.add()

    def add(self):
        import glob
        globbed = glob.glob(os.path.join(self.path,'*'+self.suffix))

        if globbed and not self.default:
            self.default=globbed.pop(0)

        if not  self.default:
            self.error('No files with suffix %s found'%self.suffix)

        self.selectables=[self.default]

        for f in globbed:
            if os.path.isfile(f):
                f=f.strip()
                self.selectables.append(f)


    def get(self):
        self.info('Selectable files:')
        for i,f in enumerate(self.selectables):
            self.info('[%i] - %s' % (i,f))

        v = ValueSelector(prompt=self.prompt,default=0,test=int,allowed=range(len(self.selectables)))
        isel = v.get()

        self.selected = self.selectables[isel]
        self.info('Got file: %s' % (self.selectables[isel]))
        return self.selected

if __name__=='__main__':

    iv    = ValueSelector(prompt='Enter a float', test=float, default=1.0)
    iFile = FileSelector(suffix='.pyc',prompt='Select pyc file')
    b     = BooleanSelector(prompt='Is it yes or no?')
    j     = ValueSelector(prompt='Enter an int', test=int, default=0)

    I = Interactors('Test function for interactor3')
    I.add(iv,'floatvalue','f')
    I.add(iFile,'filevalue','i')
    I.add(b,'boolvalue','b')
    I.add(j,'intvalue','j')

    I.getAll()

    print iFile.selected
    print b.selected

    print ''
    print  I

