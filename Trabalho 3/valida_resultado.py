import pandas as pd


resultado = pd.read_table('resultado.txt', sep = '|', header = None, names = ['tempo', 'processo'])


r = 1
i = 0
while i < (len(resultado.tempo) - 1):
    a = resultado.iloc[i].tempo
    b = resultado.iloc[i + 1].tempo
    
    if a > b:
        print("Erro")
        r = 0
        break
    else:
        i += 1

if r == 1:
    print("Processos/threads acessaram o arquivo 'resultado.txt' na ordem correta.")
else:
    print("Processos/threads não respeitaram a ordem de acesso.")