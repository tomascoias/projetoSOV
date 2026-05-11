// sockstr-cli1.0.c
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
int id;
char nome[100];
int idade;
} MsgCliente;
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
MsgCliente m;
printf("ID: ");
scanf("%d", &m.id);
getchar();
printf("Nome:");
fgets(m.nome, 100, stdin);
m.nome[strcspn(m.nome, "\n")] = 0; //Remover \n
printf("Idade; ");
scanf("%d", &m.idade);
// Enviar a struct
int n = send(s, &m, sizeof(m), 0);
exit_on_error(n, "send");
// Receber a resposta
char msg2[100];
n = recv(s, msg2, sizeof(msg2), 0);
exit_on_error(n, "recv");
// Mostrar 
printf("\nResposta servidor:\n");
printf("%s\n", msg2);
close(s);
}
