#!/usr/bin/python
# Program for generating code from QMF schema.xml
#

import sys
import os

def usage():
    print "Usage: %s template schema [output]" % sys.argv[0]

if len(sys.argv) < 3:
    usage()
    sys.exit(1)

template = sys.argv[1]
schema = sys.argv[2]
if len(sys.argv) < 4:
    output = None
else:
    output = sys.argv[3]

from Cheetah.Template import Template
import lxml.etree

schema = lxml.etree.parse(schema)
types = lxml.etree.parse(os.path.join(os.path.dirname(__file__), "types.xml"))
def getType(schema):
    return types.find("type[@schema='%s']" % schema)

nameSpace = {'schema': schema, 'getType': getType}

t = Template(file=template, searchList=[nameSpace])
#f = open(template.replace(".tmpl", ".py"), 'w')
#f.write(t.generatedModuleCode())
#f.close()
if output is not None:
    f = open(output, 'w')
else:
    f = sys.stdout
f.write(str(t))
f.close()
