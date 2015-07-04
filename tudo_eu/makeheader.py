#! python3

import os
import sys
import re
from glob import glob

Debug = True

def getBlock(file):
    result = ''
    for line in file:
        if line == '\n':
            result += line
            break
        else:
            result += line
    return result

def getCode(file, is_code):
    result = getCode.last
    while True:
        block = getBlock(file)
        if block == '':
            break
        elif isCode(block) != is_code:
            break
        else:
            result += block
    getCode.last = block
    return result
getCode.last = ''

def isCode(str):
    if str[0] == '#':
        return False
    if str.find('typedef') != -1 or str.find('struct') != -1:
        return False
    return True

def removeCode(str):
    i = 0
    chaves = 0
    result = ''
    while i < len(str):
        if str[i] == '{':
            chaves += 1
        elif str[i] == '}':
            chaves -= 1
            if chaves == 0:
                result = result[0:-1] + ';'
        elif chaves == 0:
            result += str[i]
        i+=1
    return result

def main():
    allFiles = {
        'header': glob('*.h'),
        'source': glob('*.c')
    }
    files = []
    # para todos os arquivos .c que possuem .h
    for source in allFiles['source']:
        basename = source[0:-2]
        if basename+'.h' in allFiles['header'] and basename != 'main' and basename != 'misc':
            files += [basename]

    if not files: # se estiver vazio
        print('Nao arquivo encontrado')
        sys.exit(0)

    for basename in files:
        print('Criando arquivo "'+basename+'.h" de "'+basename+'.c".')
        c_mid = '' # meio do código fonte
        line_block = '' # bloco com linhas para averiguar se faz parte do começo ou do fim do arquivo
        # processando
        h_file = open(basename+'.h', 'r', encoding='utf-8')
        c_file = open(basename+'.c', 'r', encoding='utf-8')

        # arquivo header
        h_beg = getCode(h_file, False) # começo do arquivo header
        h_mid = getCode(h_file, True) # meio do arquivo header
        h_end = getCode(h_file, False) # fim do arquivo header

        # arquivo source
        c_beg = getCode(c_file, False) # começo do arquivo source
        c_mid = getCode(c_file, True) # meio do arquivo source
        c_end = getCode(c_file, False) # fim do arquivo source

        h_file.close()
        c_file.close()

        # salvando no arquivo header
        h_file = open(basename+'.h', 'w', encoding='utf-8')
        h_file.write(h_beg)
        h_file.write(removeCode(c_mid))
        if h_end != '':
            h_file.write('\n\n'+h_end)
        h_file.close()

if __name__ == '__main__':
    main()