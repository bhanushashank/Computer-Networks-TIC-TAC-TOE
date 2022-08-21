#include <stdio.h>
#include<bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include<poll.h>
#define MAX 80
#define PORT 8080
#define TIMEOUT 15
#define SA struct sockaddr

using namespace std;

//Identity codes for identification 
char *identity[] = {"TRN","INV","CNT","UPD","WAT","DRW","LSE","WIN","QIT","AGI","YES","SRT","HLD"};
int gameId = 0;

//Error handler
void error(const char *message)
{
    perror(message);
    cout<<"Either the server is down or opponent player is disconnected.\n";
    exit(0);
}

//Receive The message from server
void receiveTheMessage(int sockfd, char * message)
{
    memset(message, 0, 4);
    int ans = read(sockfd, message, 3);
}


//Printing the Grid
void PrintTheGrid(char grid[][3])
{
    printf(" %c | %c | %c \n", grid[0][0], grid[0][1], grid[0][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[1][0], grid[1][1], grid[1][2]);
    printf("-----------\n");
    printf(" %c | %c | %c \n", grid[2][0], grid[2][1], grid[2][2]);
}

//Chossing the position in the grid
void ChooseThePosition(int sockfd)
{
    char buff[10];
    
    for(;;) { 
        cout<<"Enter (ROW, COL) for placing your mark: ";
        fflush(stdout);

        struct pollfd pfd[1];
        pfd[0].fd = 0;
        pfd[0].events = POLLIN;   
        pfd[0].revents = 0;
        int status = poll((struct pollfd*)&pfd , 1, TIMEOUT*1000);
        if(status == 0) {
            int message = 25;
            write(sockfd, &message, sizeof(int));
            break;
        }

        fgets(buff, 10, stdin);
        int row = buff[0] - '0';
        int column = buff[2] - '0';
        int move = 3*(row-1);
        move += (column-1);
        if (move >= 0 && move <= 8){
            write(sockfd, &move, sizeof(int));   
            break;
        } 
        else{
            cout<<"Invalid (ROW,COL) number. Try again.\n";
        }
    }
}

//Updating the grid with respective symbol
void updateTheGrid(int sockfd, char grid[][3])
{
    int id,move;
    read(sockfd, &id, sizeof(int));
    read(sockfd, &move, sizeof(int));

    if(id==1){
        grid[move/3][move%3] = 'X';
    }
    else{
        grid[move/3][move%3] = 'O';
    }
    PrintTheGrid(grid);   
}


int main(int argc, char *argv[])
{
    struct sockaddr_in servaddr;

    // socket create and verification
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }

    int id,gameId;

    //Reading the gameID AND Id from server
    read(sockfd, &id, sizeof(int));
    read(sockfd, &gameId, sizeof(int));

    cout<< "Connected to the game server. Your player ID is "<< (2*gameId+id+1) << "\n";
    gameId++;

    //Arrays for grid as well as for identification codes
    char pincode[4];
    char grid[3][3] = { {' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '} };


    //Waiting till another player joins
    do {
        receiveTheMessage(sockfd, pincode);
        if (!strcmp(pincode, identity[12])){
            cout<<"Waiting for a partner to join . . .\n";
        }
    } while ( strcmp(pincode, identity[11]));

    //Printing the info of the players joined
    if(id==0){
        cout<<"Your partner’s ID is " << 2*gameId <<". Your symbol is ‘O’ \n";
    }
    else{
        cout<<"Your partner’s ID is "<<2*gameId-1<<". Your symbol is ‘X’ \n";
    }

    cout<<"Starting the game …\n";
    cout<<"Your game ID is : " << gameId << "\n";
    PrintTheGrid(grid);
   
    //Here we continously run the loop and read the message send by the server.
    for(;;) {
        receiveTheMessage(sockfd, pincode);

        //Execution in case of won
        if (!strcmp(pincode, identity[7])) { 
            cout<< "You won the game!\n";
            cout<<"Do you want to play again ? \n";
            cout<<"Enter (Y/N) :- ";
            char buff[10];
            fgets(buff,10,stdin);
            //Checking if they want to play again
            if(buff[0]=='Y'){
                int temp = 20;  //Here 20 is used for identification of "YES" message to server
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
                if(!strcmp(pincode,identity[10])){
                    //Clearing the grid
                    for(int x=0;x<3;x++){
                            for(int y=0;y<3;y++){
                                grid[x][y] = ' ';
                            }
                    }
                    //Starting new game
                    cout<<"Starting the game …\n";
                    read(sockfd, &gameId, sizeof(int));
                    cout<<"Your game ID is : " << gameId << "\n";
                    PrintTheGrid(grid);
                    continue;
                }
            }
            else{
                int temp = 19;  //19 is used for Indication of "No" message to server
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
            }
            break;
        } 
        //Execution in case of Draw
        if (!strcmp(pincode, identity[5])) { 
            cout<<"Game is Draw.\n";
            cout<<"Do you want to play again ? \n";
            cout<<"Enter (Y/N) :- ";
            char buff[10];
            fgets(buff,10,stdin);
            //Checking if they want to play again
            if(buff[0]=='Y'){
                int temp = 20;  //Here 20 is used for identification of "YES" message to server
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
                if(!strcmp(pincode,identity[10])){
                    //Start the new game again
                    cout<<"Starting the game …\n";
                    read(sockfd, &gameId, sizeof(int));
                    cout<<"Your game ID is : " << gameId << "\n";
                    //Clearing the grid
                    for(int x=0;x<3;x++){
                            for(int y=0;y<3;y++){
                                grid[x][y] = ' ';
                            }
                    }
                    PrintTheGrid(grid);
                    continue;
                }
            }
            else{
                int temp = 19;  //19 is used for Indication of "No" message to server
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
            }
            break;
        } 
        //Exection in case of Lost
        if (!strcmp(pincode, identity[6])) { 
            cout<<"You lost the game.\n";
            cout<<"Do you want to play again ? \n";
            cout<<"Enter (Y/N) :- ";
            char buff[10];
            fgets(buff,10,stdin);
            //Checking if they want to play again
            if(buff[0]=='Y'){
                int temp = 20;
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
                if(!strcmp(pincode,identity[10])){
                    for(int x=0;x<3;x++){
                            for(int y=0;y<3;y++){
                                grid[x][y] = ' ';
                            }
                    }
                    //Start the new game again
                    cout<<"Starting the game …\n";
                    read(sockfd, &gameId, sizeof(int));
                    cout<<"Your game ID is : " << gameId << "\n";
                    PrintTheGrid(grid);
                    continue;
                }
            }
            else{
                int temp = 19;
                write(sockfd, &temp, sizeof(int));
                receiveTheMessage(sockfd,pincode);
            }
            break;
        }
        //Indication that it's your turn
        if (!strcmp(pincode, identity[0])) { 
            cout<<"It's your turn...\n";
            ChooseThePosition(sockfd);
        }
        //Message placed identity
        if (!strcmp(pincode, identity[1])) { 
            cout<<"This symbol already kept in this position . Try again with another position.\n";
        }
        //updating the grid
        if (!strcmp(pincode, identity[3])) { 
            updateTheGrid(sockfd, grid);
        }
        //Wait till the opponent plays.
        if (!strcmp(pincode, identity[4])) { 
            cout<<"Waiting for opponent's move...\n";
        }
        if(!strcmp(pincode,identity[8])){
            cout<<"Sorry your partner disconnected..\n";
            break;
        }
        //Indication for timeout 
        if(!strcmp(pincode,identity[9])){
                cout<< "\nTIMEOUT\n";
                //ASking do you want to play again
                cout<<"Game is terminated automatically\n";
                cout<<"Do you want to play again ? \n";
                cout<<"Enter (Y/N) :- ";
                char buff[10];
                fgets(buff,10,stdin);
                //Checking if they want to play again
                if(buff[0]=='Y'){
                    int temp = 20;
                    write(sockfd, &temp, sizeof(int));
                    receiveTheMessage(sockfd,pincode);
                    if(!strcmp(pincode,identity[10])){
                        for(int x=0;x<3;x++){
                                for(int y=0;y<3;y++){
                                    grid[x][y] = ' ';
                                }
                        }
                        cout<<"Starting the game …\n";
                        read(sockfd, &gameId, sizeof(int));
                        cout<<"Your game ID is : " << gameId << "\n";
                        PrintTheGrid(grid);
                        continue;
                    }
                }
                else{
                    int temp = 19;  //19 is used for Indication of "No" message to server
                    write(sockfd, &temp, sizeof(int));
                    receiveTheMessage(sockfd,pincode);
                }
                break;
        }
    }
    close(sockfd);
    return 0;
}
