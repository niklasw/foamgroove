#!/usr/bin/python

from string import Template
import os,re
from testSuiteUtils import Info,Error,Debug,Warning,findDirs

class htmlTemplate:
    def __init__(self, templateFile, root='',relRoot=''):
        self.OK = False
        # Set full path to templateFile
        self.templatesRoot = os.path.dirname(__file__)
        templateFile = os.path.join(self.templatesRoot,templateFile)
        self.root = root
        self.relativeRoot = relRoot

        self.content = ''
        try:
            with open(templateFile,'r') as fp:
                self.template = Template(fp.read())
                self.OK = True
        except:
            self.content = 'Template not found {0}'.format(templateFile)
        self.addCrumbs()

    def addCrumbs(self):
        def crumbs(root):
            while len(root) > len(self.relativeRoot):
                yield root
                root = os.path.split(root)[0]

        htList = htmlList([],name='')
        root = self.root
        for path  in reversed(list(crumbs(root))):
            name = os.path.basename(path)
            htList.items.append(htmlLink(name,os.path.join(path,'index.html')))
        htList.update()
        self.addContent(menu=htList.content)

    def addContent(self, curpage='index', **kwargs):
        if self.OK:
            self.content = self.template.safe_substitute(kwargs,  currentpage=curpage)
            self.template = Template(self.content)

    def __str__(self):
        return self.content

class htmlDiv:
    def __init__(self,cls='myclass'):
        self.cls = cls
        self.contentList = []
        self.content = ''

    def generate(self):
        s = '<div class="{0}">\n'.format(self.cls)
        s+= '\n'.join(self.contentList)
        s+= '\n</div><!-- {0} -->'.format(self.cls)
        return s

    def append(self,content):
        self.contentList.append(content)

    def update(self):
        self.content = self.generate()

    def __str__(self):
        return self.content

class htmlPre:
    def __init__(self,header,rawText,level=1):
        self.header = header
        self.level = level
        self.par = rawText
        self.content = self.generate()

    def generate(self):
        s = '\n<h{0}>{1}</h{0}>\n'.format(self.level,self.header)
        s+= '\n<pre>{0}</pre>\n'.format(self.par)
        return s

    def __str__(self):
        return self.content

class htmlSection:
    def __init__(self,header,paragraph, level=1):
        self.header = header
        self.level = level
        self.par = paragraph
        self.content = self.generate()

    def generate(self):
        s = '\n<h{0}>{1}</h{0}>\n'.format(self.level,self.header)
        s+= '\n<p>{0}</p>\n'.format(self.par)
        return s

    def __str__(self):
        return self.content

class htmlTable:

    def __init__(self, rowList, head=[], cls='',description=''):
        self.rows = rowList
        self.head = head
        self.cls = cls
        self.description = description
        self.rowCounter = 0
        self.content = self.generate()

    def newRow(self,cols, clst=('even','odd')):
        # Horisontally striping possilbe through CSS classes
        # even or odd
        cls = clst[self.rowCounter % len(clst)]
        r0 = '<tr class="{0}">'.format(cls)
        r1 = '</tr>'
        c = ' '.join(['<td>{0}</td>'.format(a) for a in cols])
        r = '{0} {1} {2}\n'.format(r0,c,r1)
        self.rowCounter += 1
        return r

    def generate(self):
        div = htmlDiv(self.cls)

        start = '<table>\n'.format(self.cls)
        end   = '\n</table>'
        header = self.newRow(self.head,clst=('head','head')) if self.head else ''
        rowList=[header]
        for row in self.rows:
            rowList.append(self.newRow(row))
        rows = '\n'.join(rowList)
        annotation = '<p>{0}</p>\n'.format(self.description)
        div.append(annotation)
        div.append(start+rows+end)
        div.update()
        return div.content

    def __str__(self):
        return str(self.content)

class htmlLink:
    def __init__(self,name='Press here',href='#',cls=''):
        self.dictionary= {'name':name, 'href':href, 'class':cls}
        self.content = self.generate()

    def generate(self):
        line = '<a class="$class" href="$href">$name</a>'
        return Template(line).safe_substitute(self.dictionary)

    def __str__(self):
        return str(self.content)


class htmlList:
    def __init__(self,itemList=[], name=''):
        self.items = itemList
        self.head = name
        self.content=self.generate()

    def generate(self):
        content = '<ul>\n'
        if self.head:
            content+= '\t<li style="font-weight:bold;">{0}</li>\n'.format(self.head)
            content+= '\t<li>-----------</li>\n'
        for item in self.items:
            content+= '\t<li>{0}</li>\n'.format(item)
        content += '</ul>\n'
        return content

    def update(self):
        self.content = self.generate()

class htmlImage:
    def __init__(self,name,src,cls='',description=''):
        self.name = name
        self.src = src
        self.cls = cls
        self.description = description
        self.content = self.generate()

    def generate(self):
        div = htmlDiv(self.cls)
        div.append('<img src="{0}" alt="{1}" />'.format(self.src,self.name))
        div.append('<p>{0}</p>'.format(self.description))
        div.update()
        return div.content


class htmlTree:
    '''Write index.html files for navigation from presentationRoot
    down to folder matching leafNamePattern. Starts by finding
    leafNamePattern paths and backs up to presentationRoot.

    FIXME: Can be done much neater.'''

    def __init__(self,presentationRoot,leafNamePattern, template):
        self.presentationRoot = presentationRoot
        self.leafNamePattern = leafNamePattern
        self.template = template

    def folderContains(self,root,pattern):
        pat = re.compile(pattern)
        return len([c for c in os.listdir(root) if pat.match(c)])

    def targetPaths(self,root):
        dirs = [ os.path.join(root,d) \
                for d in os.listdir(root) \
                if os.path.isdir(os.path.join(root,d)) ]
        for d in dirs:
            candidate = os.path.join(d,'index.html')
            if os.path.isfile(candidate):
                yield candidate

    def printHtml(self,root):
        div = htmlDiv(cls='menu')
        htList = htmlList([],name='')
        for path in self.targetPaths(root):
            linkName = os.path.basename(os.path.dirname(path))
            htList.items.append(htmlLink(linkName,path))
        htList.update()
        div.append(htList.content)
        div.update()

        doc = htmlTemplate(self.template,root,self.presentationRoot)
        doc.addContent(content1=div.content)
        return doc.content

    def writeHtml(self,root):
        if self.folderContains(root,self.leafNamePattern):
            return False
        content = self.printHtml(root)
        with open(os.path.join(root,'index.html'),'w') as fp:
            fp.write(content)
        return True

    def makeIndexTree(self):
        Info('Generating index.html tree')
        leaves = findDirs(self.presentationRoot,self.leafNamePattern,dirname=True)
        for leaf in leaves:
            currentRoot = leaf
            while len(currentRoot) > len(self.presentationRoot) :
                currentRoot = os.path.split(currentRoot)[0]
                Debug('makeIndexTree: '+currentRoot)
                self.writeHtml(currentRoot)



if __name__=='__main__':
    import sys,os
    from os.path import join as pjoin

    A = []
    for i in range(5):
        r = []
        for j in range(4):
            r.append(j)
        A.append(r)

    doc = htmlTemplate('htmlTemplates/testCase.html')

    paragraph = htmlSection('First  htmlDiv class=book','It goes like this.\n'*4,level=1)
    table = htmlTable(A)

    div1 = htmlDiv(cls='book')
    div1.append(paragraph.content)
    div1.append(table.content)

    L = ['niklas','provar','en','lista']
    alink = htmlLink(name='A link',href='http://www.hetsa.nu',cls='')
    L.append(alink)
    alist=htmlList(L,'List name')

    div1.append(alist.content)
    div1.update()

    div2 = htmlDiv(cls='book')
    paragraph = htmlSection('Second htmlDiv class=book','It goes like this.\n'*8,level=2)
    div2.append(paragraph.content)
    div2.append(table.content)
    div2.update()

    doc.addContent(content3=div1.content)
    doc.addContent(content4=div2.content)


    print doc
