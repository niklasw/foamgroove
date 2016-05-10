#!/usr/bin/python

from string import Template
import os

class htmlTemplate:
    def __init__(self, templateFile):
        self.OK = False
        self.content = ''
        try:
            with open(templateFile,'r') as fp:
                self.template = Template(fp.read())
                self.OK = True
        except:
            self.content = 'Template not found {0}'.format(templateFile)

    def addContent(self, curpage='index', **kwargs):
        if self.OK:
            self.content = self.template.safe_substitute(kwargs,  currentpage=curpage)
            self.template = Template(self.content)

    def __str__(self):
        return self.content

class htmlTable:

    def __init__(self, rowList):
        self.rows = rowList
        self.rowCounter = 0
        self.content = ''

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

    def new(self, cls='', head=[]):
        start = '<table class={0}>\n'.format(cls)
        end   = '\n</table>'
        header = self.newRow(head,clst=('head','head')) if head else ''
        rowList=[header]

        for row in self.rows:
            rowList.append(self.newRow(row))
        rows = '\n'.join(rowList)
        self.content = start+rows+end

class listLink:
    def __init__(self,name='**',href='#',cls=''):
        self.dictionary= {'name':name, 'href':href, 'class':cls}
        self.content = self.generate()

    def generate(self):
        line = '<li><a class="$class" href="$href">$name</a></li>'
        return Template(line).safe_substitute(self.dictionary)

    def __str__(self):
        return str(self.content)

class htmlList:
    def __init__(self,itemList=[], name='List'):
        self.items = itemList
        self.content=self.generate(head=name)

    def generate(self, head='List'):
        content = '<ul>\n'
        content+= '\t<li style="font-weight:bold;">{0}</li>\n'.format(head)
        content+= '\t<li>-----------</li>\n'
        for item in self.items:
            content+= '\t<li>{0}</li>\n'.format(item)
        content += '</ul>\n'
        return content

    def update(self, itemList, name='List'):
        self.items=itemList
        self.content = self.generate(head=name)

class htmlImage:
    def __init__(self,name,src,cls='',description=''):
        self.name = name
        self.src = src
        self.cls = cls
        self.description = description
        self.content = self.generate()

    def generate(self):
        s = '<div class="{0}">'.format(self.cls)
        s+= '<img src="{0}" alt="{1}" />'.format(self.src,self.name)
        s+= '<p>{0}</p>'.format(self.description)
        s+= '</div>'
        return s



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

    table = htmlTable(A)
    table.new(cls='mytable', head=['A','B'])

    doc.addContent(content1=table.content)
    doc.addContent(content2=table.content)

    L = ['niklas','provar','en','lista']
    alink = listLink(name='A link',href='http://www.hetsa.nu',cls='')
    L.append(alink)
    alist=htmlList(L,'List name')
    doc.addContent(content3=alist.content)


    print doc
