#include <pthread.h>
#include<bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <poll.h>
#include <chrono> 

#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define TIMEOUT 15

using namespace std;

//Identity codes for identification 
char *identity[] = {"TRN","INV","UPD","WAT","DRW","LSE","WIN","QIT","AGI","YES","HLD","SRT","NO "};

int numberOfPlayers = 0; //number of players at an instant
int gameId = 0;          //Variable for new game
pthread_mutex_t lockVariable;

//Function handler for error
void ErrorMessage(const char *msg)
{
    perror(msg);
    pthread_exit(NULL);
}


//Receive integer from client
int receiveInteger(int sockfd)
{
    int message = 0;
    int n = read(sockfd, &message, sizeof(int));

    if(n <= 0){
        return -1;
    }

    if(n!=sizeof(int)){
        return -1;
    }
    
    return message;
}


//Writing integer to client
void writeIntegerToClient(int sockfd, int message)
{
    int n = write(sockfd, &message, sizeof(int));
    if (n < 0){
        ErrorMessage("writing int to client socket is failed");
    }
}

//writing message to client
void writeMessageToClient(int sockfd, char * message){
    int n = write(sockfd, message, strlen(message));
    if (n < 0){
        ErrorMessage("writing message to client socket is failed");
    }
}

//Connecting clients to server 
void getplayersforGame(int sockfd, int *player1,int *player2)
{
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    int *client_sockfd = (int*)malloc(2*sizeof(int)); 
    client_sockfd[0] = 0;
    client_sockfd[1] = 1;

    //Considering only two players to create a game and assign a thread to them.
    for(int i=0;i<2;i++)
    {
        listen(sockfd, 201 - numberOfPlayers);
        memset(&cli_addr, 0, sizeof(cli_addr));
        clilen = sizeof(cli_addr);

        //Making the connection between players
        client_sockfd[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    
        //If valid is failed then printing error message
        if (client_sockfd[i] < 0){
            ErrorMessage("Accepting connection from a client is failed");
        }
        
        //Assigning id to client
        write(client_sockfd[i], &i, sizeof(int));
        write(client_sockfd[i],&gameId,sizeof(int)); 


        //Number of players at that instant
        numberOfPlayers++;

        //If there is single player,sending message to wait.
        if (i == 0) {
            writeMessageToClient(client_sockfd[0],identity[10]);
            cout<<"Sent odd player to wait until another player join\n";
        }
    }
    //Storing back to integers
    *player1 = client_sockfd[0];
    *player2 = client_sockfd[1];
}


//Checking if they want to play again
bool Replay(int * sockfd){
    int yes1 = receiveInteger(sockfd[0]);
    int yes2 = receiveInteger(sockfd[1]);
    if((yes1 == 20) && (yes2 == 20)){
        return 1;
    }
    return 0;
}

//Check if there is a possibilty of win
int winCheck(char grid[][3],int move){
    int row = move/3;
    int column = move%3;

    if((grid[row][0] == grid[row][1]) && (grid[row][1]==grid[row][2])){
        return 1;
    }

    if((grid[0][column]==grid[1][column])&& (grid[1][column]==grid[2][column])){
        return 1;
    }
    return 0;
}

//Actual code execution(game between two players in thread)
void *ExecuteTheGame(void *data) {

    int *sockfd = (int*)data; 
    //creating grid
    char grid[3][3] = { {' ', ' ', ' '},  {' ', ' ', ' '}, {' ', ' ', ' '} };


    //For calculating time of the game
    auto start = std::chrono::high_resolution_clock::now();


    //Assigning gameId
    gameId += 1;

    //File streaming for writing into logfiles (we are maintaining the file w.r.to each player)
    ofstream fout;

    //Filename
    string ans = "log";
    ans += to_string(gameId);
    ans += ".txt";

    fout.open(ans);

    fout<<"Game ID is : "<<gameId<< "\n";
    //Sending the signal to start the game

    write(sockfd[0], identity[11], strlen(identity[11]));
    write(sockfd[1], identity[11], strlen(identity[11]));
    writeIntegerToClient(sockfd[0], gameId);
    writeIntegerToClient(sockfd[1], gameId);
    int tempId = gameId;
    
    //a[0] -> previous player id  is stored
    //a[1] -> current player id is stored
    //a[2] -> check whether game is over or not
    //a[3] -> Counting number of moves played

    int a[4] = {1,0,0,0};


    //Running the while loop for playing game
    while(!a[2]) {
        
        if (a[0] != a[1]){
            writeMessageToClient(sockfd[(a[1] + 1) % 2], identity[3]);    //Sending oppnent to wait until he plays
        }

        int move = 0;

        for(;;) {         
            //getting move played by the client    
            writeMessageToClient(sockfd[a[1]], identity[0]);
            move = receiveInteger(sockfd[a[1]]);

            if (move == -1) {
                break; 
            }
            if(move == 25){
                break;
            }

            int row = move/3 + 1;
            int column = move%3 + 1;
            fout<<"Player "<< 2*tempId+(a[1]-1) <<" played move ("<<row<<","<<column<<")\n";


            //Validate the move
            if(grid[row-1][column-1]==' '){
                break;
            }  

            //Sending client to try with another input
            fout<<"Move was invalid. Let's try this again...\n";
            writeMessageToClient(sockfd[a[1]], identity[1]);

        }
    
        //If player get disconnected
        if (move == -1) { 
            fout<<"Player "<< 2*tempId+(a[1]-1) <<" is disconnected\n";
            fout<<"Game is completed\n";

            auto end =  std::chrono::high_resolution_clock::now();
            double elapsed_time_ms = std::chrono::duration<double, std::milli>(end-start).count();

            //Store time taken by the game into the file
            fout<<"Time Taken to play the game is: "<<elapsed_time_ms/1000<<" sec\n\n";
            writeMessageToClient(sockfd[(a[1]+1)%2],identity[7]);
            break;
        }
        //Disconnecting the players if time for giving input is more than 15secs
        else if(move == 25){
            fout<<"Players move is TimeOut\n";

            write(sockfd[0], identity[8], strlen(identity[8]));
            write(sockfd[1], identity[8], strlen(identity[8]));

            auto end =  std::chrono::high_resolution_clock::now();
            double elapsed_time_ms = std::chrono::duration<double, std::milli>(end-start).count();

            //Store time taken by the game into the file
            fout<<"Time Taken to play the game is: "<<elapsed_time_ms/1000<<" sec\n\n";

            bool flag = Replay(sockfd);
            if(flag){
                write(sockfd[0], identity[9], strlen(identity[9]));
                write(sockfd[1], identity[9], strlen(identity[9]));
                gameId += 1;
                fout<<"Game ID is : "<<gameId<< "\n";
                writeIntegerToClient(sockfd[0], gameId);
                writeIntegerToClient(sockfd[1], gameId);
                a[0] = 1;
                for(int x=0;x<3;x++){
                    a[x+1] = 0;
                    for(int y=0;y<3;y++){
                        grid[x][y] = ' ';
                    }
                }
                start = std::chrono::high_resolution_clock::now();
                continue;
            } 
            else{
                write(sockfd[0], identity[12], strlen(identity[12]));
                write(sockfd[1], identity[12], strlen(identity[12]));
            }
            break;
        }
        //Continue the game
        else {

            write(sockfd[0], identity[2], strlen(identity[2]));
            write(sockfd[1], identity[2], strlen(identity[2]));

            writeIntegerToClient(sockfd[0], a[1]);
            writeIntegerToClient(sockfd[1], a[1]);

            writeIntegerToClient(sockfd[0], move);
            writeIntegerToClient(sockfd[1], move);

            //update the grid
            if(a[1]){
                grid[move/3][move%3] = 'X';
            }
            else{
                grid[move/3][move%3] = 'O';
            }

            a[2] = winCheck(grid, move);   //Checking if any player won the game


            //If player won the game asking them to play again with sending the meassge that won or lost accordingly.
            if (a[2] == 1) {
                writeMessageToClient(sockfd[a[1]], identity[6]);
                writeMessageToClient(sockfd[(a[1] + 1) % 2], identity[5]);
                fout<<"Player " << 2*tempId+(a[1]-1) << " won the game\n";
                fout<<"Game over\n";

                auto end =  std::chrono::high_resolution_clock::now();
                double elapsed_time_ms = std::chrono::duration<double, std::milli>(end-start).count();

                //Store time taken by the game into the file
                fout<<"Time Taken to play the game is: "<<elapsed_time_ms/1000<<" sec\n\n";

                //Asking for players whether they want to play again
                bool flag = Replay(sockfd);
                if(flag){
                    write(sockfd[0], identity[9], strlen(identity[9]));
                    write(sockfd[1], identity[9], strlen(identity[9]));
                    gameId += 1;
                    fout<<"Game ID is : "<<gameId<< "\n";
                    writeIntegerToClient(sockfd[0], gameId);
                    writeIntegerToClient(sockfd[1], gameId);

                    //Updating the grid accordingly
                    a[0] = 1;
                    for(int x=0;x<3;x++){
                        a[x+1] = 0;
                        for(int y=0;y<3;y++){
                            grid[x][y] = ' ';
                        }
                    }
                    start = std::chrono::high_resolution_clock::now();
                    continue;
                } 
                else{
                    write(sockfd[0], identity[12], strlen(identity[12]));
                    write(sockfd[1], identity[12], strlen(identity[12]));
                }
            }
            //Game is draw in this game.
            else if (a[3] == 8) {                
                fout<<"Game is Draw\n";
                write(sockfd[0], identity[4], strlen(identity[4]));
                write(sockfd[1], identity[4], strlen(identity[4]));
                fout<<"Game over\n";

                //Printing the time taken by the game into file
                auto end =  std::chrono::high_resolution_clock::now();
                double elapsed_time_ms = std::chrono::duration<double, std::milli>(end-start).count();
                fout<<"Time Taken to play the game is: "<<elapsed_time_ms/1000<<" sec\n\n";

                //Asking the players to replay
                bool flag = Replay(sockfd);
                if(flag){
                    write(sockfd[0], identity[9], strlen(identity[9]));
                    write(sockfd[1], identity[9], strlen(identity[9]));
                    gameId += 1;
                    fout<<"Game ID is : "<<gameId<< "\n";
                    writeIntegerToClient(sockfd[0], gameId);
                    writeIntegerToClient(sockfd[1], gameId);

                    //Updating the board
                    a[0] = 1;
                    for(int x=0;x<3;x++){
                        a[x+1] = 0;
                        for(int y=0;y<3;y++){
                            grid[x][y] = ' ';
                        }
                    }
                    start = std::chrono::high_resolution_clock::now();
                    continue;
                } 
                else{
                    write(sockfd[0], identity[12], strlen(identity[12]));
                    write(sockfd[1], identity[12], strlen(identity[12]));
                }
                a[2] = 1;
            }

            a[0] = a[1];
            a[1] += 1;
            a[1] %= 2;
            a[3]++;
        }
    }

    //Closing the file
    fout.close();

    //Closing the sockets for both players
    close(sockfd[0]);
    close(sockfd[1]);

    //Decreenting the number of players by 2 when clients are disconnected
    pthread_mutex_lock(&lockVariable);
    numberOfPlayers -= 2;
    pthread_mutex_unlock(&lockVariable);
    
    free(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{   
    struct sockaddr_in servaddr, cli;
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

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }

    pthread_mutex_init(&lockVariable, NULL);
    cout<<"Game server started. Waiting for players.\n";

    for (;;){
        //Maximun limit of players is 200
        if (200 >= numberOfPlayers) {  

            //Assigning one thread to two players 

            int player1,player2;
            getplayersforGame(sockfd,&player1,&player2);    //Getting there socket descriptors
            int *client_sockfd = (int*)malloc(2*sizeof(int)); 
            memset(client_sockfd, 0, 2*sizeof(int));

            client_sockfd[0] = player1;
            client_sockfd[1] = player2;

            cout<<"Starting new game thread ...\n";

            //Creating the thread for two players
            pthread_t thread;
            int ans = pthread_create(&thread, NULL, ExecuteTheGame, (void *)client_sockfd);
        }
    }

    close(sockfd);
    pthread_mutex_destroy(&lockVariable);
    pthread_exit(NULL);
}
