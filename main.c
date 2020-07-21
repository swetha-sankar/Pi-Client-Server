/* Swetha Sankar and Lauren Baron
 * CISC 210-080 
 * Final Project: Main File
 * May 2020
 * Uses number of arguments to determine whether the user is the client or the server. The client uses their
 * joystick to determine what color is displayed on the server's framebuffer. The server sends back what color was 
 * received from the client. The program terminates with CTRL + C. 
 * */
#include "main.h"//header file
#define BUFFER_SIZE 4096 //constant
pi_framebuffer_t* fb; 
char color[BUFFER_SIZE]; //variable to set the color of the framebuffer with the input from the joystick
int run = 1; //used within while loop

/**
 * Function used w/in signal function
 * **/
void handler(int sig){
	run = 0;
}

/**
 * A function to switch the color of the screen and reset the screen when the joystick is moved. Uses 
 * a switch statement for the unsigned int code to alternate the colors between purple, cyan blue, pink, and orange.
 *
 * */
void callbackFn(unsigned int code){
	switch(code){
		//takes the input of the joystick and converts it to a color
		case KEY_UP:
			bzero(color, BUFFER_SIZE);
			strcpy(color, "pink");
			break;
		case KEY_DOWN:
			bzero(color, BUFFER_SIZE);
			strcpy(color, "cyan");
			break;
		case KEY_RIGHT:
			bzero(color, BUFFER_SIZE);
			strcpy(color, "orange");
			break;
		case KEY_LEFT:
			bzero(color, BUFFER_SIZE);
			strcpy(color, "purple");
			break;
		default:
		//does nothing if the user does not move the joystick
			break;
		}
}

/**
 * A function to draw the color on the framebuffer. Takes in the color from the polljoystick function
 * */

void drawScreen(char* color){
	//defines colors
	uint16_t purple = getColor(255, 0, 255);
	uint16_t orange = getColor(255, 128, 0);
	uint16_t pink = getColor(255, 128, 128);
	uint16_t cyan = getColor(0, 255, 255);

	//turns entire screen to the color received from joystick
	if(strncmp(color, "purple", 6) ==0){
		clearBitmap(fb->bitmap, purple);
	}
	else if(strncmp(color, "cyan", 4) == 0){
		clearBitmap(fb->bitmap, cyan);
	}
	else if(strncmp(color, "orange", 6) == 0){
		clearBitmap(fb->bitmap, orange);
	}
	else if(strncmp(color, "pink", 4) == 0){
		clearBitmap(fb->bitmap, pink);
	}

	else{
		clearBitmap(fb->bitmap, 0);
	}
}


/* Based on whether the user is a client or a server, creates socket and implements respective actions. Client sends
 * server color to draw and server draws the color on its framebuffer. Client receives the color received by the 
 * server.*/

int main(int argc, char *argv[]){
	int sockfd;
	int newsockfd;
	int portno;
	char output[BUFFER_SIZE];
	bool isServer;
	struct hostent *server;
	signal(SIGINT, handler);
	pi_joystick_t* joystick = getJoystickDevice();
	fb = getFBDevice();
	pi_i2c_t *device = geti2cDevice();
	clearBitmap(fb->bitmap, 0);
	
	if (argc ==2){
		portno = atoi(argv[1]); //server if 2 arguments
		isServer = true;
	}
	else if(argc == 3){
		portno = atoi(argv[1]); // client if 3 args
		server = gethostbyname(argv[2]);
		isServer = false;
		if(server == NULL){
			fprintf(stderr, "ERROR, no such host \n");
			exit(0);
		}}
	else{
		fprintf(stderr, "Usage: %s <server port> \n", argv[0]);
		exit(0);
	}
	
	if(isServer){
		newsockfd = createServer(portno, &sockfd, &newsockfd);
		while(run){
			int n;
			n = recvServer(newsockfd, color);
			//Depending on what color the server receives, it sends the client back the color that it 
			//receives and draws this on its framebuffer
			if(n>0){
				strcat(output, "Received color: ");
				if(strncmp(color, "purple", 6) == 0){
					strcat(output, "purple");
					drawScreen(color);
					sleep(2); //keeps color on the screen 
					sendServer(newsockfd, output);
				}
				else if(strncmp(color, "cyan", 4)==0){
					strcat(output, "cyan");
					drawScreen(color);
					sleep(2);
					sendServer(newsockfd, output);
				}
				else if(strncmp(color, "orange", 5)==0){
					strcat(output, "orange");
					drawScreen(color);
					sleep(2);
					sendServer(newsockfd, output);
				}
				else if(strncmp(color, "pink", 4) == 0){
					strcat(output, "pink");
					drawScreen(color);
					sleep(2);
					sendServer(newsockfd, output);
				}
				else{
					sendServer(newsockfd, "Unsuccessful");
				}
			}	
		//resets buffers and bitmap
		clearBitmap(fb->bitmap, 0);
		bzero(output, BUFFER_SIZE);
		bzero(color, BUFFER_SIZE);
		}
	}
	else{
		createClient(portno, &sockfd, server);
		while(run){
			sleep(3); //lags joystick input
			pollJoystick(joystick, callbackFn, 1000);
			sendClient(sockfd, color);
			int n;
			n = recv(sockfd, output, BUFFER_SIZE, MSG_DONTWAIT);
			if(n>0){
				fprintf(stdout, "%s\n", output); //prints what the server sent back
				bzero(output, BUFFER_SIZE);
				bzero(color, BUFFER_SIZE);
			}
		}	
	//cleans up joystick, framebuffer, closes sockets
	freeFrameBuffer(fb);
	freeJoystick(joystick);
	close(newsockfd);
	close(sockfd);
	return 0;
	}	
}

/* Prints error message */
void error(char *msg){
	perror(msg);
	exit(0);
}

/* Function to create client given the port number, socket to store, and server- returns the socket */
int createClient(int portno, int *sockfd, struct hostent* server){
	struct sockaddr_in serv_addr;
	*sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(*sockfd<0){
		error("ERROR opening socket");
	}
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);
	if(connect(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
		error("ERROR connecting");
			}
	return *sockfd;
}

/*Function to send request from the client given a socket and the buffer that contains the request for the server */
void sendClient(int sockfd, char* colorBuffer){
	int n;
	n = send(sockfd, colorBuffer, strlen(colorBuffer), 0);
	if(n < 0){
		error("ERROR writing from socket");
	}
}

/*Function to create server given the port number (given the socket so that the main can close the socket when done) 
 * opens, binds, and accepts*/
int createServer(int myPortno, int *sockfd, int *newsockfd){
	int clilen;
	struct sockaddr_in serv_addr, cli_addr;
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(*sockfd < 0){
		error("ERROR opening socket");
	}

	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(myPortno);

	if(bind(*sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0){
		error("ERROR onbinding");
	}

	listen(*sockfd, 5);
	clilen = sizeof(cli_addr);
	*newsockfd = accept(*sockfd, (struct sockaddr*)&cli_addr, &clilen);
	
	if(*newsockfd < 0){
		error("ERROR on accept");
	}

	return *newsockfd;
}

/** Function to receive server. Gets the request on the server (through socket) and puts it into the buffer*/
int recvServer(int mynewsockfd, char* colorBuffer){
	int n;
	n = recv(mynewsockfd, colorBuffer, BUFFER_SIZE-1, MSG_DONTWAIT);	// changed flag to MSG_DONTWAIT
	return n;
}

/** Function to send a message (stored in myOutput back to the client through the socket **/
void sendServer(int mynewsockfd, char* myOutput){
	int n;
	n = send(mynewsockfd, myOutput, strlen(myOutput), 0);
	if(n<0){
		error("ERROR writing to socket");
	}
}

