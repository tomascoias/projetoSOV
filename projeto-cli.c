// projeto-cli2.0.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#define exit_on_error(s,m) if ( s < 0 ) { perror(m); exit(1); }
// Estrutura de Mensagem
typedef struct {
char nome[50];
char mensagem[500];
} ChatMsg;
main() {
// Socket Client
int s = socket ( PF_INET, SOCK_STREAM, 0 );
exit_on_error ( s, "socket");
// Endereco do servido
struct sockaddr_in s_addr;
s_addr.sin_family = AF_INET;
s_addr.sin_addr.s_addr = inet_addr ( "127.0.0.1" );
s_addr.sin_port = htons(5678);
// Connect
int status;
status=connect( s, (struct sockaddr*)&s_addr, sizeof(s_addr) );
exit_on_error ( status, "connect");
printf("Ligado ao servidor!\n");
// Criar a estrutura
while(1){
ChatMsg m;
printf("Nome:");
fgets(m.nome, 50, stdin);
m.nome[strcspn(m.nome, "\n")] = 0; //Remover \n
printf("Mensagem:");
fgets(m.mensagem, 500, stdin);
// Enviar a struct
int n = send(s, &m, sizeof(m), 0);
exit_on_error(n, "send");
// Receber a resposta
char resposta[100];
n = recv(s, resposta, sizeof(resposta), 0);
exit_on_error(n, "recv");
printf("Servidor: %s\n", resposta);
}
close(s);
}