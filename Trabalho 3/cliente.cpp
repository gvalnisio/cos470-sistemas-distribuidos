// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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


//Definindo o mutex

int sock;

class def_mutex {
public:

    //Função pra estabelecer conexão com o coordenador através do uso de 
    //sockets, ideia tirada do seguinte endereço:
    //"https://stackoverflow.com/questions/61172683/server-reading-a-command-line-argument-passed-to-the-client"    
    def_mutex(char *ip_coordenador) {

        int val_read;
        struct sockaddr_in server_address;

        //em caso de erro
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cout << "Erro: Nao foi possivel criar o socket." << endl;
            exit(-1);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        //em caso de erro
        if (inet_pton(AF_INET, ip_coordenador, &server_address.sin_addr) <= 0) {
            cout << "Erro: erro no endereço IPv4. " << endl;
            exit(-1);
        }

        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            cout << "Erro: erro com a conexao com o socket do servidor." << endl;
            exit(-1);
        }
    }
    ~def_mutex(){};
};


//Função que envia uma mensagem com separadores (ex:"1|pid|000...") para o
//coordenador e espera pela mensagem de "GRANT" de forma bloqueante.
int acquire_lock() {

    char *buffer = transforma(REQUEST, getpid());
    send(sock, buffer, sizeof(char) * 30, 0);
    int valread = read(sock, buffer, sizeof(char) * 30);
    delete buffer;
    if (valread == 0) {
        return 0;
    }
    return 1;
}
    
//Função que envia uma mensagem com separadores (ex:"3|pid|000...") para o
//coordenador avisando o "RELEASE" do lock.
int release_lock() {

    char *buffer = transforma(RELEASE, getpid());
    send(sock, buffer, sizeof(char) * 30, 0);
    delete buffer;
    return 1;
}

int main(int argc, char *argv[]) {

    int k = atoi(argv[1]);
    int r = atoi(argv[2]);
    def_mutex mutex(argv[3]);
    ofstream myfile;
    
    for (int i = 0; i < r; i++) {

        if (!acquire_lock()) {
            return -1;
        }

        //Escrevendo o horário e o pid do processo/thread no documento de texto 
        //"resultado.txt" e esperando k segundos.
        myfile.open("resultado.txt", ios::app);
        auto current_time = chrono::high_resolution_clock::now();
        myfile << chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch()).count() << "|" << getpid() << "\n";
        myfile.close();
        release_lock();
        sleep(k);
    }
}