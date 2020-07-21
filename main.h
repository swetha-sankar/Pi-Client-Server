/*Swetha Sankar and Lauren Baron
 *Final Project: Header File 
 *Includes function declarations and imports for main
 *May 2020
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sense/sense.h>
#include <linux/input.h>
#include <signal.h>
void error(char *msg);
void handler(int sig);
int createServer(int myPortno, int *sockfd, int *newsockfd);
void callbackFn(unsigned int code);
void drawScreen(char* color);
int recvServer(int mynewsockfd, char* colorBuffer);
void sendServer(int mynewsockfd, char* myOutput);
void sendClient(int sockfd, char* colorBuffer);
int createClient(int myPortno, int *sockfd, struct hostent* server);
