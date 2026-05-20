#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
// uniformização do tratamento de erros
#define exit_on_error(s,m) if ( s < 0 ) { perror(m); exit(1); }
typedef struct{
    char nome[50];
    char mensagem[500];
} ChatMsg;
// estrutura para passar as sockets para as threads
struct dados_ligacao {
    int socket_origem;
    int socket_destino;
};

ChatMsg aviso;

int cliente_espera = -1;
ChatMsg dadosEspera;

// função executada pelas threads para receber e enviar mensagens para os clientes
void *chat(void *argumento) {
    //fazer o casting do argumento genérico para a estrutura de dados_ligacao
    struct dados_ligacao *dados = (struct dados_ligacao *) argumento;
    int origem = dados->socket_origem;
    int destino = dados->socket_destino;

    //libertar a memória alocada pela estrutura, porque já não é necessária
    //pois extraimos os valores das sockets para variáveis locais
    free(dados);

    char buffer[sizeof(ChatMsg)];
    int bytes_lidos;


    //ciclo contínuo para ler mensagens do cliente de origem e enviar para o cliente de destino
    while ( (bytes_lidos = recv( origem, buffer, sizeof(buffer), 0)) > 0 ) {
        int bytes_enviados = send( destino, buffer, bytes_lidos, 0);
        //tratamento de erro no envio da mensagem para o cliente de destino
        if (bytes_enviados < 0) {
            perror("Erro ao enviar mensagem!");
            break;
        }
    }
    
    //encerrar corretamente as ligações quando o cliente de origem se desconectar 
    // ou se houver um erro na leitura da mensagem
    printf("Um cliente desconectou-se.\n");
    strcpy(aviso.mensagem, "Outro cliente saiu do chat.\n");    
    send(destino,&aviso, sizeof(aviso), 0);
    close (origem);

    shutdown(destino, SHUT_RDWR);
    close(destino);

    return NULL;
}

int main() {
    //Defenir o nome das mensagens do servidor
    strcpy(aviso.nome, "SERVIDOR");

    //criar o socket principal do servidor
    int sock = socket ( PF_INET, SOCK_STREAM, 0 );
    exit_on_error ( sock, "Erro no socket");

    //configurar o endereço do Servidor
    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    //aceita ligações de qualquer IP
    sock_addr.sin_addr.s_addr = htonl ( INADDR_ANY );
    //Porta de escuta do servidor
    sock_addr.sin_port = htons(5678);

    //associar o socket do servidor ao endereço configurado
    int status = bind (sock, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    exit_on_error (status, "Erro no bind");

    //colocar o servidor à escuta (Decisão de projeto: backlog é 2, pois o projeto exige 2 clientes)
    status = listen (sock, 2);
    exit_on_error (status, "Erro no listen");

    printf("Servidor à escuta na porta 5678. À espera de clientes...\n");
    while(1){
        int novo_cliente = accept(sock, NULL, NULL);
        exit_on_error (novo_cliente, "Erro no accept novo cliente!");
        ChatMsg dadosNovo;
        recv(novo_cliente, &dadosNovo, sizeof(dadosNovo), 0);
        printf("Novo cliente ligado!\n");
        //Nao existe nenhum cliente em espera (Servidor Iniciado pela 1º Vez)
        if(cliente_espera == -1){
            cliente_espera = novo_cliente;
            dadosEspera = dadosNovo;
            printf("Cliente em espera...\n");
        }
        // Já existe alguem à espera
        else{
            //aceitar os dois clientes que se vão ligar ao servidor.
            //o servidor bloqueia nesta fase até que os clientes se liguem e que tenham nomes.
            int cliente_A = cliente_espera;
            ChatMsg dadosClienteA = dadosEspera;
            printf("Cliente A ligado!\n");

            int cliente_B = novo_cliente;
            ChatMsg dadosClienteB = dadosNovo;
            printf("Client B ligado!\n");
            //Ja fazemos o recv no inicio e tratamos dos error ao criar (retirar)

            cliente_espera = -1;
            
            //Verifica se tem os nomes iguais, se tiver adiciona um "id" a frente.
            if(strcmp(dadosClienteA.nome, dadosClienteB.nome) == 0){
                strcat(dadosClienteA.nome, "#1");
                strcat(dadosClienteB.nome, "#2");
            }    

            //Envia aos 2 clientes uma mensagem/aviso com os seus nomes, para se caso haver alguma alteracao
            send(cliente_A,&dadosClienteA,sizeof(dadosClienteA), 0);
            send(cliente_B,&dadosClienteB,sizeof(dadosClienteB), 0);

            printf("O chat vai iniciar! Os clientes podem começar a enviar mensagens...\n");

            //lançar a thread 1 (Cliente A -> Cliente B)
            pthread_t id_threadAB;
            struct dados_ligacao *dados_threadAB = malloc(sizeof(struct dados_ligacao));
            dados_threadAB->socket_origem = cliente_A;
            dados_threadAB->socket_destino = cliente_B;
            pthread_create(&id_threadAB, NULL, chat, (void *) dados_threadAB);

            //lançar a thread 2 (Cliente B -> Cliente A)
            pthread_t id_threadBA;
            struct dados_ligacao *dados_threadBA = malloc(sizeof(struct dados_ligacao));
            dados_threadBA->socket_origem = cliente_B;
            dados_threadBA->socket_destino = cliente_A;
            pthread_create(&id_threadBA, NULL, chat, (void *) dados_threadBA);

            //o processo principal aguarda que as threads terminem)
            pthread_detach(id_threadAB);
            pthread_detach(id_threadBA);
        }
    }
    close(sock);
    return 0;
}