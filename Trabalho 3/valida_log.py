import pandas as pd


log = pd.read_table('log_resultado.txt', sep = '|', header = None, names = ['mensagem', 'tempo', 'processo'])

r = 1
i = 0
while i < (len(log.mensagem) - 2):
    a = log.iloc[i].mensagem
    b = log.iloc[i + 1].mensagem
    
    if a == 'REQUEST':
        if b == 'GRANT':
            i += 1
        else:
            print("Erro")
            r = 0
            break
    elif a == 'GRANT':
        if b == 'RELEASE':
            i += 1
        else:
            print("Erro")
            r = 0
            break
    elif a == 'RELEASE':
        if b == 'REQUEST':
            i += 1
        else:
            print("Erro")
            r = 0
            break

if r == 1:
    print("Processos/threads acessaram o arquivo 'log_resultado.txt' na ordem correta.")
else:
    print("Processos/threads não respeitaram a ordem de acesso.")