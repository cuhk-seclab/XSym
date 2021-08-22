import os
import re

support_type=["char", "int", "void", "char*", "int*", "long", "long*", "short", "short*""unsigned char", "unsigned int","unsigned char*", "unsigned int*", "bool", "bool*", "pval", "pval*", "size_t", "size_t*", "uint", "uint*"]
dir_path="./functions"
files=os.listdir(dir_path)

total=0
solved=0

def write_code(input_type, function_name, output_type):
    fi = open("./auto/"+function_name+'.c', 'w+')
    fi.write("#include \"stdio.h\""+'\n')
    fi.write("#include \"phli.h\""+'\n')
    if input_type[0]=="INTERNAL_FUNCTION_PARAMETERS":
        fi.write("void "+function_name+"(")
    else:
        fi.write(output_type+" "+function_name+'(')
    s=",".join(input_type)
    fi.write(s+");"+'\n')

    fi.write("int main(int argc, char* argv[]) {"+'\n')
    fi.write("char *result_dir=\"/data/phli/results/"+function_name+"/\";"+'\n')

    i = -1 
    para_name=[]
    call=function_name+'('

    if input_type[0]=="INTERNAL_FUNCTION_PARAMETERS":
        fi.write("HashTable *ht;\nHashTable *list;\nHashTable *plist;\npval *return_value=phli_construct_pval_string(\"arg_1\");\n")
        fi.write(function_name+'(ht, return_value, list, plist);\n')
        fi.write('}')

    else:
        s=""
        for type in input_type:
            i=i+1
            if type=="char":
                fi.write("char* arg_"+str(i)+"=phli_create_char(\"arg_"+str(i)+"\");"+'\n')
                para_name.append("*arg_"+str(i))
            elif type=="char*":
                fi.write("char* arg_"+str(i)+"=phli_create_char_array(\"arg_"+str(i)+"\");"+'\n')
                para_name.append("arg_"+str(i))
            elif type=="int" or type=="long":
                fi.write("long* arg_"+str(i)+"=phli_create_long(\"arg_"+str(i)+"\");"+'\n')
                para_name.append("*arg_"+str(i))
            elif type=="int*" or type=="long*":
                fi.write("int* arg_"+str(i)+"=phli_create_int_array(\"arg_"+str(i)+"\");"+'\n')
                para_name.append("arg_"+str(i))
            elif type=="bool":
                fi.write("bool* arg_"+str(i)+"=phli_create_bool(\"arg_"+str(i)+"\");\n")
                para_name.append("*arg_"+str(i))
            elif type=="uint" or type=="unsigned int" or type=="size_t":
                fi.write("unsigned int* arg_"+str(i)+"=phli_create_unsignedint(\"arg_"+str(i)+"\");\n")
                para_name.append("*arg_"+str(i))
            elif type=="unsigned char*":
                fi.write("unsigned char* arg_"+str(i)+"=phli_create_unsigned_char_array(\"arg_"+str(i)+"\");\n")
                para_name.append("arg_"+str(i))
            elif type=="unsigned int*" or type=="uint*" or type=="size_t*":
                fi.write("unsigned int* arg_"+str(i)+"=phli_create_unsigned_int_array(\"arg_"+str(i)+"\");\n")
                para_name.append("arg_"+str(i))
            elif type=="unsigned char":
                fi.write("unsigned char* arg_"+str(i)+"=phli_create_unsigned_char(\"arg_"+str(i)+"\");\n")
                para_name.append("*arg_"+str(i))
            elif type=="pval":
                fi.write("pval* arg_"+str(i)+"=phli_construct_pval_string(\"arg_"+str(i)+"\");\n")
                para_name.append("*arg_"+str(i))
            elif type=="pval*":
                fi.write("pval* arg_"+str(i)+"=phli_construct_pval_string(\"arg_"+str(i)+"\");\n")
                para_name.append("arg_"+str(i))
            else:
                print type 
        if output_type!="void":
            print(function_name, output_type)
            fi.write(output_type+" result;\n")
            s+="result="
            
        s+=function_name+'('+','.join(para_name)+');\n}'
        fi.write(s)
    fi.close()




for file in files:
    if not os.path.isdir(file):
        path=dir_path+'/'+file
        #iter_f=iter(f)
        '''
        iterate each line:
        1. Parse function prototype format: Return value type + function name + (  + input arguments + )
        2. Replace ARG_COUNT(ht) with 1
        '''
        for line in open(path, 'r'):
            #print line

            if(re.match(r"\s*(unsigned|signed)?\s*(void|int|char|short|long|float|double|pval)(\*)?\s+(\*)?(\w+)\s*\([^)]*\)\s*", line)):#it's a function'
                #print 1
                total=total+1

                s=line[: line.find('(')].strip()
                s1=s.split()
                function_name=s1[-1].strip()
                input_type=[]
                output_type=""

                #print function_name
                input_args=line[line.find('(')+1: line.find(')')]

                if input_args=="INTERNAL_FUNCTION_PARAMETERS":
                    input_type.append(input_args)
                    if(function_name[0]=='*'):
                        function_name=function_name[1:]
                    output_type="char*" #TODO
                    write_code(input_type, function_name, output_type)
                    solved=solved+1

                else:
                    arg_list=input_args.split(",")
                    if arg_list[0]=="":
                        continue
                    can=1
                    for arg in arg_list:
                        arg=arg.strip()
                        arg=arg.split()
                        if len(arg)==1 and arg[0]!="void":
                            print (function_name, arg)
                            can=0
                            break
                        if arg[0]=="void":
                            arg_type="void"
                        else:
                            arg_type=arg[-2].strip()
                        if(arg[0].strip()=="unsigned"):
                            arg_type="unsigned "+arg_type
                        if(arg[-1][0]=='*'):
                            arg_type=arg_type+'*'
                        if arg_type not in support_type:
                            print (function_name, arg_type)
                            can=0
                            break
                        input_type.append(arg_type)
                    if can==1:
                        output_type+=s1[-2].strip()
                        if(len(s1)>=3 and s1[-3].strip()=="unsigned"):
                            output_type="unsigned "+output_type
                        if(function_name[0]=='*'):
                            function_name=function_name[1:]
                            output_type=output_type+'*'
                        if(output_type not in support_type):
                            print (function_name, output_type)
                            can=0
                    if can==1: # we can model this function
                        #print function_name, input_type
                        #print function_name
                        write_code(input_type, function_name, output_type)
                        solved=solved+1

print (total, solved)

                

