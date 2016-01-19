#!/usr/bin/env python

import os,sys

class ioList(list):

    def __init__(self,var='(0 0 0)'):
        if isinstance(var,list):
            list.__init__(self,var)
        elif isinstance(var,str):
            s = strList.strip('() "')
            try:
                list.__init__(self,map(float, s.split()))
            except TypeError:
                print 'Error in ioList.__init__'
                raise

    def __str__(self):
        return '('+' '.join((str(a) for a in self))+')'

class interactor:

    def __init__(self):
        pass

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

    def iFileSelector(self,prompt='Select from above',default='',path='',suffix='*',folder=False, stripShow=False):
        print
        import glob
        globbed = glob.glob(os.path.join(path,'*'+suffix))

        if globbed and not default:
            default=globbed.pop(0)

        if not  default:
            self.error('No files with suffix %s found'%suffix)

        selectables=[default]
        self.info('Selectable files:')

        for f in globbed:
            if os.path.isfile(f):
                f=f.strip()
                selectables.append(f)

        for i,f in enumerate(selectables):
            s = f
            if stripShow:
                s=os.path.basename(os.path.splitext(f)[0])
            self.info('[%i] - %s' % (i,s))

        isel = self.get(prompt=prompt,default=0,test=int,allowed=range(len(selectables)))

        self.info('Got file: %s' % (selectables[isel]))

        selected = selectables[isel]

        return selected,os.path.basename(os.path.splitext(selected)[0])

    def fileSelector(self,prompt='Select from above',default='',path='',suffix='*',folder=False):
        print
        import glob
        globbed = glob.glob(os.path.join(path,'*'+suffix))
        if globbed and not default:
            default=globbed.pop(0)

        if not  default:
            self.error('No files with suffix %s found'%suffix)

        selectables=[default]
        self.info('Selectable files:')

        for f in globbed:
            if os.path.isfile(f):
                f=f.strip()
                selectables.append(f)
                self.info('|--> '+f)

        # Try to remove default duplicate
        # selectables = list(set(selectables))

        try:
            selected = raw_input('%s [%s]: '% (prompt,selectables[0])).strip()
            if not selected: selected = selectables[0]
            if not os.path.isfile(selected):
                self.warn('Selection is not a file')
                selected = self.fileSelector(prompt,default,path,suffix,folder)

        except (KeyboardInterrupt,SystemExit):
            self.error('Execution aborted by user')
        return selected

    def get(self,prompt='Enter value',default=10,test=int, allowed=[], showAllowed=False):
        print
        if showAllowed:
            for ok in allowed:
                self.info('|--> %s '%ok)
        try:
            selected = raw_input('%s [%s]: '% (prompt,default))
            if not selected:
                selected = default

            try:
                selected = test(selected)
            except:
                self.warn('Typo: %s is not %s' % (str(selected),test.__name__))
                selected = self.get(prompt,default,test,allowed)

            if allowed and not test(selected) in allowed:
                selected = self.get(prompt,default,test,allowed)

        except (KeyboardInterrupt,SystemExit):
            self.error('Execution aborted by user')

        return selected

    def iGet(self,prompt='Enter value',default=0,test=int, allowed=[], showAllowed=False):
        print
        if showAllowed:
            for i,ok in enumerate(allowed):
                self.info('(%3i)%s '%(i,ok))
        try:
            selected = raw_input('%s [%s]: '% (prompt,default))
            if not selected:
                selected = default
            try:
                selected = test(selected)
            except:
                self.warn('Typo: %s is not %s' % (str(selected),test.__name__))
                selected = self.iGet(prompt,default,test,allowed)

            if allowed and not test(selected) in range(len(allowed)):
                selected = self.iGet(prompt,default,test,allowed)

        except (KeyboardInterrupt,SystemExit):
            self.error('Execution aborted by user')

        if test(selected) in range(len(allowed)):
            print 'test', selected
            return allowed[selected]
        else:
            print 'notest',selected
            return selected

    def yesno(self,prompt='Enter value',default='N',test=str,allowed=['y','n','Y','N']):
        print
        try:
            selected = raw_input('%s (y/n) [%s]: '% (prompt,default))
            if not selected: selected = default
            if not selected in allowed:
                selected = self.yesno(prompt,default,test,allowed)
                if selected:
                    selected = 'y'
                else:
                    selected = 'n'
        except (KeyboardInterrupt,SystemExit):
            self.error('Execution aborted by user')

        if selected.lower() == 'y':
            return True
        else:
            return False

if __name__=='__main__':

    i=interactor()

    val=i.get(test=float,default=10)
    sel=i.fileSelector(suffix=".pyc")
    print sel,val

    #f=open('/home/niklasw/OpenFOAM/extend/TurboMachinery/README.svn')
    #print removeLines(f.read(),'=====','Policy')


