// projeto-cli3.0.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#define exit_on_error(s,m) if ( s < 0 ) { perror(m); exit(1); }
// Estrutura de Mensagem
typedef struct {
char nome[50];
char mensagem[500];
} ChatMsg;

ChatMsg m;

// Thread enviar mensagens
void *enviar_mensagens(void *arg){
  int socket_client = *(int *)arg;
  while(1){
    printf("Mensagem:");
    fgets(m.mensagem, 500, stdin);
    if(strncmp(m.mensagem, "/sair", 5) == 0){
        printf("A sair do chat...\n");
        close(socket_client);
        exit(0);
    }
    int n = send(socket_client, &m, sizeof(m), 0);
    if (n < 0){
      printf("Erro ao enviar mensagem.\n");
      break;
    }
  }
  return NULL;
}

//Thread receber mensagens
void *receber_mensagens(void *arg){
  int socket_client = *(int *)arg;
  ChatMsg rm;
  while(1){
    int n = recv(socket_client, &rm, sizeof(rm), 0);
    if(n <= 0){
      printf("Servidor desconectado. \n");
      break;
    }
    printf("\n[%s]: %s", rm.nome, rm.mensagem);
    printf("Mensagem: ");
    fflush(stdout);
  }
  return NULL;
}
int main() {
// Socket Client
int s = socket ( PF_INET, SOCK_STREAM, 0 );
exit_on_error ( s, "socket");
// Endereco do servidor
struct sockaddr_in s_addr;
s_addr.sin_family = AF_INET;
s_addr.sin_addr.s_addr = inet_addr ( "127.0.0.1" );
s_addr.sin_port = htons(5678);
// Connect
int status;
status=connect( s, (struct sockaddr*)&s_addr, sizeof(s_addr) );
exit_on_error ( status, "connect");
printf("Ligado ao servidor!\n");

while(strlen(m.nome) <= 0){
    printf("Nome: ");
    fgets(m.nome, 50, stdin);
    m.nome[strcspn(m.nome, "\n")] = 0; //Remover \n
  }
//Enviar o nome para o server para comprar se caso for repetido e avisar que esta "Pronto" (ja inseriu o nome)
send(s, &m, sizeof(m), 0);

//Esperar pelo o servidor dar resposta
recv(s, &m, sizeof(m), 0); 

//Elimina o texto que o cliente escreve enquanto esta a espera
tcflush(STDIN_FILENO, TCIFLUSH);

// Threads
pthread_t thread_enviar;
pthread_t thread_receber;
// Thread enviar
pthread_create(&thread_enviar, NULL, enviar_mensagens, (void *)&s);

// Thread receber
pthread_create(&thread_receber, NULL, receber_mensagens, (void *)&s);

// Esperar threads
pthread_join(thread_enviar, NULL);
pthread_join(thread_receber, NULL);

close(s);
return 0;
}