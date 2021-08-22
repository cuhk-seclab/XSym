import json, os
make_symbolic = 'klee_make_symbolic(%s, %d, \"%s\");\n'
assume = 'klee_assume(%s);'
tab = '\t'
head = '#include <stdio.h>\n#include "phli.h"\nint main(int argc, char *argv[]) { \n\tchar *result_dir = "/data/phli/results/%s/";\n%s\n}'

default_size = {'void': 4, 'char*': 8, 
        'int': 4, 'char': 1, 'int*': 8,
        'pval': 24, 'pval*': 8, 'uint': 4,
        'unsigned char': 1,'unsigned char*': 8, 
        'size_t': 8, 'size_t*': 8}

def getsign(argtype, w, wo):
    if argtype.endswith('*'):
        return w
    else:
        return wo

def getCreateFunction(argtype):
    return 'phli_create_' + argtype.strip('*').replace(' ', '_') + getsign(argtype, '_array', '')

def declare(argtype, argname):
    s = tab + argtype + getsign(argtype, '', '*') + ' ' + argname + ' = ' + getCreateFunction(argtype) + '(\"' + argname + '\");\n'
    #s += tab + make_symbolic % (argname, default_size[argtype], argname)
    return  s

def insertcode(fname, args, returntype = 'void'):
    code = ''
    funcdeclare = '%s %s(%s);\n' % (returntype, fname, ', '.join(args))  
    argstring = ''
    for i, argtype in enumerate(args):
        code += declare(argtype, 'arg_%d' % i)
        argstring += '%sarg_%d, ' % (getsign(argtype, '', '*'), i)

    if returntype != 'void':
        code += tab + returntype  + ' phli_result;\n' 
        code += tab + 'phli_result = ' + fname + '(%s);\n' % argstring.rstrip(', ')
    else:
        code += tab + fname + '(%s);\n' % argstring.rstrip(', ')
        
    return funcdeclare + head % (fname, code)

fp = open('functions/stringfunclist.file')
contents = fp.read()
functions = json.loads(contents)
parent_dir = '/data/phli/results/'
for fname in functions:
    #fname = 'php_bin2hex'
    code = insertcode(fname, functions[fname]['params'], functions[fname]['return'])
    fp = open('mymains/%s.c' % fname, 'w+')
    funcdir = os.path.join(parent_dir, fname)
    if not os.path.exists(funcdir): 
        os.mkdir(funcdir, 0755)
    fp.write(code)
    fp.close()
    #print(code)
    # better prepare a prototype and append these code below
