import sys, json
def main(fname):
    fp = open(fname, 'r')
    contents = fp.readlines()
    fp.close()
    funcs = []
    for i, line in enumerate(contents):
        if line[0] != ' ' and line[0] != '\t' and '(' in line:
            if line.startswith('PHPAPI') or line.startswith('void') or line.startswith('int') or line.startswith('static') or line.startswith('char') or line.startswith('unsigned'):
                if line.rstrip().endswith(','):
                    funcs.append(line.rstrip() + contents[i + 1])
                else:
                    funcs.append(line)
    fp = open(fname.rstrip('.c') + 'funclist.file', 'w+')
    fp.write(parsefunc(funcs))
    fp.close()

types = ['void', 'char*','int', 'char', 'int*', 'pval', 'pval*', 'uint', 'unsigned char','unsigned char*', 'size_t', 'size_t*']

def getparamtype(string):
    string = string.strip() # remove spaces
    tmp = string.split()
    name = tmp[-1]
    thetype = ''
    if len(tmp) >= 2 and tmp[-2] == '*':
        if tmp[0] == 'static' or tmp[0] == 'const':
            thetype = ' '.join(tmp[1: -2])
        else:
            thetype = ' '.join(tmp[0: -2])
        thetype += '*'

    else:
        if tmp[0] == 'static' or tmp[0] == 'const':
            thetype = ' '.join(tmp[1: -1])
        else:
            thetype = ' '.join(tmp[0: -1])
        if name[0] == '*':
            thetype += '*'

    if thetype == '' and string == 'INTERNAL_FUNCTION_PARAMETERS':
        thetype = string
    #print(string, thetype)
    return thetype

def parseparams(string):
    params = []
    for p in string.split(','):
        params.append(getparamtype(p))
    return params

def getfuncname(string):
    tmp = string.strip().split()
    name = tmp[-1]

    thetype = tmp[: -1]
    if thetype[0] == 'PHPAPI'and len(thetype) == 2:
        thetype = thetype[1]
    elif thetype[0] == 'static' and thetype[1] == 'inline':
        thetype = ' '.join(thetype[2: ])
    elif thetype[0] == 'static':
        thetype = ' '.join(thetype[1: ])
    elif len(thetype) == 1:
        thetype = thetype[0]
    else:
        thetype = ' '.join(thetype)

    if name[0] == '*':
        return name[1: ], thetype + '*'
    else:
        return name, thetype

def parsefunc(funcheads):
    functions = {}
    for i, func in enumerate(funcheads):
        lpos = func.find('(')
        rpos = func.find(')')
        if rpos < 0:
            continue
        name, returntype = getfuncname(func[: lpos])
        params = parseparams(func[lpos + 1: rpos])
        value = {"params": params, "return": returntype}
        if name in functions and functions[name] != value:
            print('conflict definition', name)
            print(functions[name], params)
        else:
            functions[name] = value
    string = json.dumps(functions)
    return string

main(sys.argv[1])
