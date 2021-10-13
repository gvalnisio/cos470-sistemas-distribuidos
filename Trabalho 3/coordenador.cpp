#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <string>
#include <queue>
#include <tuple>
#include <mutex>
#include <thread>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

using namespace std;

#define PORT 8080

//Definindo os valores das mensagens de "REQUEST", "GRANT" e "RELEASE".
#define REQUEST 1
#define GRANT 2
#define RELEASE 3


//Função que transforma a mensagem recebida no formato com separadores
//junto do id do processo/thread.
char *transforma(int mensagem, int id) {

    char *mensagem_separadores = new char[30]{0};
    string msg = to_string(mensagem) + '|' + to_string(id) + '|';
    msg.copy(mensagem_separadores, msg.length(), 0);
    return mensagem_separadores;
}

//Função que transforma de volta a mensagem no formato com separadores ao
//seu formato original, separando a mensagem e o id do processo/thread.
void destransforma(char *buffer, int *mensagem, int *id) {

    string separador = "|";
    string s(buffer);
    size_t x = s.find(separador);
    *mensagem = stoi(s.substr(0, x));
    s.erase(0, x + 1);
    size_t y = s.find(separador);
    *id = stoi(s.substr(0, y));
}



//Criando variavéis globais de acesso ao lock
bool em_uso = false;
mutex lock_mutex;
map<int, int> mapa_processo;
queue<tuple<int, int>> fila_mutex;
ofstream myfile;
auto current_time = chrono::high_resolution_clock::now();
    
//Função que garante o acesso ao próximo da fila
void proximo_fila() {
    
    em_uso = true;
    tuple<int, int> proximo = fila_mutex.front();
    fila_mutex.pop();
    
    char *mensagem = transforma(GRANT, get<0>(proximo));
    send(get<1>(proximo), mensagem, sizeof(char) * 30, 0);
    
    cout << "Processo " << get<0>(proximo) << " pede acesso (REQUEST)." << endl;
    //escrevendo o REQUEST no log, assim como o horario do pedido e numero do processo/thread. 
    myfile.open("log_resultado.txt", ios::app);
    myfile << "REQUEST" << "|" << chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count() << "|" << get<0>(proximo) << "\n";
    myfile.close();

    cout << "Processo " << get<0>(proximo) << " recebeu acesso (GRANT)." << endl;
    //escrevendo o GRANT no log, assim como o horario do pedido e numero do processo/thread. 
    myfile.open("log_resultado.txt", ios::app);
    myfile << "GRANT" << "|" << chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count() << "|" << get<0>(proximo) <<"\n";
    myfile.close();
    
    if(mapa_processo.find(get<0>(proximo)) == mapa_processo.end()) {
        mapa_processo[get<0>(proximo)] = 1;
    } else {
        mapa_processo[get<0>(proximo)] += 1;
    }
}
    
//Função para quando um dos processos/threads pede o lock
void acquire_lock(int p_id, int p_fd) {
    
    lock_mutex.lock();
    fila_mutex.push(make_tuple(p_id, p_fd));
    
    if(mapa_processo.find(p_id) == mapa_processo.end()) {
        mapa_processo[p_id] = 0;
    }
    if(!em_uso) {
        proximo_fila();
    }
    lock_mutex.unlock();
}
    
//Função para quando o lock é liberado
void release_lock() {
    
    tuple<int, int> proximo = fila_mutex.front();
    lock_mutex.lock();

    cout << "Lock liberado, regiao critica livre (RELEASE)." << endl;
    //escrevendo o RELEASE no log, assim como o horario do pedido e numero do processo/thread. 
    myfile.open("log_resultado.txt", ios::app);
    myfile << "RELEASE" << "|" << chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count() << "|" << get<0>(proximo) <<"\n";
    myfile.close();

    if(fila_mutex.empty()) {
        em_uso = false;
    } else {
        proximo_fila();
    }
    lock_mutex.unlock();
}
    
//Função que imprime a fila das threads que querem acesso à 
//região crítica.
void imprime_fila() {
    
    lock_mutex.lock();
    queue<tuple<int, int>> copia = fila_mutex;
    
    while (!copia.empty()) {
        cout << get<0>(copia.front()) << " ";
        copia.pop();
    }


    std::cout << std::endl;
    lock_mutex.unlock();
}
    
//Função que imprime a contagem de "GRANTS" que já foram dados
//pelo coordenador.
void imprime_log() {
    
    lock_mutex.lock();
    
    int i = 0;
    for(auto a = mapa_processo.cbegin(); a != mapa_processo.cend(); ++a) {
        std::cout << a->first << " | " << a->second << "\n";
        i++;
    }
    
    std::cout << i << " processos." << std::endl;
    lock_mutex.unlock();
}



//Ideia do código retirada dos endereços:
//https://blog.pantuza.com/artigos/programando-um-protocolo-utilizando-sockets
//https://www.vivaolinux.com.br/topico/C-C++/Socket-Server-C
int server() {

	int master_socket;
	struct sockaddr_in address;
    int addresslen = sizeof(address);
	
	//Criando o "master socket"

    //Em caso de erro:
	if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
		cout << "Erro: socket nao pode ser criado." << endl;
		return -1;
	}
	
    //Tipo de socket criado:
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
		
	//Ligando o socket ao port definido
	if(bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
		cout << "Erro: socket nao pode ser ligado ao port." << endl;
		return -1;
	}		
	if(listen(master_socket, 1) < 0) {
		cout << "Erro ao definir as conexoes." << endl;
		return -1;
	}
    cout << "Conectado ao port " << PORT  << "." << endl;
		
	while(true) {

        int socket_novo = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addresslen);
        if(socket_novo < 0) {
            cout << "Nao foi possivel aceitar a nova conexao." << endl;
            return -1;
        }

        //Função executada para tratar cada socket novo
        auto fconnection = [&, socket_novo]() {
            
            char buffer[30];
            while (true) {
                
                int valread = read(socket_novo, buffer, sizeof(char) * 30);
                if(valread <= 0) {
                    break;
                }

                int mensagem, processo;
                destransforma(buffer, &mensagem, &processo);

                if(mensagem == REQUEST) {
                    acquire_lock(processo, socket_novo);
                } else if(mensagem == RELEASE) {
                    cout << processo << " ";
                    release_lock();
                } else {
                    cout << "Mensagem invalida.\n";
                }
            }
        };
        thread(fconnection).detach();
    }
    return 0;
}


//Função Principal que deve ser multi-threaded
int main() {

    //criando uma thread para a função server
    thread(server).detach(); 

    //criando uma lista de comandos para a função principal
    int resposta;
    cout << "O que voce deseja fazer?" << endl;
    cout << "1 - Imprime a fila de pedidos atual" << endl;
    cout << "2 - Imprime quantas vezes o processo foi atendido pelo coordenador" << endl;
    cout << "3 - Finaliza o programa" << endl;
    
    
    while (true) {
        
        cin >> resposta;
        if(resposta == 1) {
            imprime_fila();
        } else if(resposta == 2) {
            imprime_log();
        } else if(resposta == 3) {
            cout << "Finalizando o programa..." << endl;
            return 0;
        } else {
            cout << "Por favor insira um numero valido." << endl;
        }
    }

    return 1;
}