#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define SA struct sockaddr
#define LISTENQ 128
#define MAXLINE 1024
#define SERV_PORT 8080

/* 
 * Returns the 'num' number string in a ';' separated string line
 * Helper function for readFile()
 */
char* getfield(char* line, int num)
{
    char* tok;
    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

/* 
 * Reads a ';' separated text file and stores the 
 * usernames and corresponding passwords in the arrays 'user' and 'pass'.
 * It runs just after the server starts running.
 */
int readFile(char user[5][200], char pass[5][200])
{
    FILE* stream = fopen("org.txt", "r");

    char line[1024];
    int i = 0;

    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        char* tmp1 = strdup(line);
        char *u = getfield(tmp, 1);
        strcpy(user[i], u);
        char *p = getfield(tmp1, 2);
        strcpy(pass[i], p);

        i++;
        
        // NOTE strtok clobbers tmp
        free(tmp);
        free(tmp1);
    }
    //Returns no. of registered users.
    return i;
}

/* 
 * Writes the arrays 'user' and 'pass' in a ';' separated text file.
 * It runs just before the termination of the server.
 */
void writeFile(char user[5][200], char pass[5][200], int numOfUser)
{
    FILE* stream = fopen("org.txt", "w");
    int i = 0;
    while(i < numOfUser)
    {
        char line[1024];

        strcpy(line, user[i]);
        strcat(line, ";");
        strcat(line, pass[i]);
        strcat(line, "\n");
        
        fputs(line, stream);

        i++;
    }

}

/*
 * Finds the given username and password (stored in buf) in the arrays 'user' and 'pass'.
 * 'numOfUser' is the total number of registered users till now.
 * Returns :
 *    -1  if error in userId or password.
 *    -2  if this is a new user. 
 *    >=0 indicating the index at which the username and password are found.
 */
int findUser(char user[5][200], char pass[5][200], char buf[], int numOfUser)
{
    int result;
    int i = 0;
    char *token = strtok(buf, "\n");

    for (i = 0; i < numOfUser; i++)
    {
        result = strcmp(user[i], token);

        if (result == 0)
        {
            token = strtok(NULL, "\n");

            if (strcmp(pass[i], token) == 0)
            {
                //Return the index of the userId.
                return i;
            }
            //
            return -1;
        }
    }
    if(numOfUser < 5)
    {
        strcpy(user[numOfUser], token);
        
        token = strtok(NULL, "\n");
        if(token == NULL)
            //-1 indicates some error in userId or password.
            return -1;
        strcpy(pass[numOfUser], token);
        
        //Indicates new user registration.
        return -2;
    }
    return -1;
}


/*
 * Finds whether the given animal name (in 'buf') is already present(in 'animals') or not.
 * 'count' is the number of animals stored till now.
 * Returns: 
 *      i  : the index at which the animal name is found.
 *      OR
 *      -1 : indicating that the animal name is not found.
 */
int findAnimal(char animals[5][200], char buf[], int count)
{
    int i, result;
    for (i = 0; i < count; i++)
    {
        result = strcasecmp(animals[i], buf);
        if (result == 0)
        //Returns index if animal is found.
            return i;
    }
    //Returns -1, if animal not found
    return -1;
}



/*
 * Writes all of the stored animal names in 'animals' to the client socket.
 * Helper function for the 'QUERY' command.
 * 'count' is the number of animals stored till now.
 */
void printAnimal(int sockfd, char animals[15][200], int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        char *animal = animals[i];
        strcat(animal, "\n");

        write(sockfd, animal, strlen(animal));

        //To remove \n from the animal's name.
        char *token = strtok(animals[i], "\n");
        strcpy(token, animals[i]);
    }
}

int main()
{

    char user[5][200];
    char pass[5][200];
    int numOfUser = readFile(user, pass);

    int loginStatus[FD_SETSIZE] = {0};

    char animals[15][200] = {"DOG", "CAT", "SNAKE", "PIG", "HORSE"};
    char sounds[15][200] = {"WOOF", "MEOW", "HISS", "GRUNT", "NEIGH"};
    int count = 5;

    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    ssize_t n;
    fd_set rset, allset;
    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    printf("Socket successfully bind..\n");

    listen(listenfd, LISTENQ);
    printf("Server listening..\n");

    maxfd = listenfd;
    maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for (;;)
    {
        bzero(buf, sizeof(buf));
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset))
        {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
            printf("Connected to client..\n");

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE)
            {
                printf("Too many clients");
                return 0;
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd)
            {
                maxfd = connfd;
            }
            if (i > maxi)
            {
                maxi = i;
            }
            if (--nready <= 0)
                continue;
        }
        for (i = 0; i <= maxi; i++)
        {
            bzero(buf, sizeof(buf));
            if ((sockfd = client[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset))
            {
                if ((n = read(sockfd, buf, MAXLINE)) == 0)
                {
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    loginStatus[i] = 0;
                    char *msg = "Connection closed with the client..\n";
                    write(1, msg, strlen(msg));
                }
                else
                {
                    if(strcasecmp(buf, "\n") == 0)
                    {
                        continue;
                    }
                    if (loginStatus[i] == 0)
                    {
                        int index;
                        if(strcasecmp(buf, "\n\n") == 0)
                        {
                            index = -1;
                        }
                        else
                            index = findUser(user, pass, buf, numOfUser);
                        if (index == -1)
                        {
                            char *temp = "Sorry your id and password is not correct\n";
                            write(sockfd, temp, strlen(temp));
                            loginStatus[i] = 0;
                        }
                        else if(index == -2)
                        {
                            numOfUser++;
                            char *msg = "Registered successfully..\n";
                            write(sockfd, msg, strlen(msg));
                            loginStatus[i] = 1;
                            char num[5];
                            write(1, msg, strlen(msg));
                        }
                        else
                        {
                            char *msg = "User logged in..\n";
                            write(1, msg, strlen(msg));
                            loginStatus[i] = 1;
                            write(sockfd, msg, strlen(msg));
                        }
                    }
                    else
                    {
                        char *token = strtok(buf, "\n");
                        if (strcasecmp(buf, "SOUND") == 0)
                        {
                            char msg[] = "SOUND: OK\n";
                            write(sockfd, msg, strlen(msg));
                        }
                        else if (strcasecmp(token, "QUERY") == 0)
                        {
                            printAnimal(sockfd, animals, count);

                            char *msg = "QUERY: OK\n";
                            write(sockfd, msg, strlen(msg));
                        }
                        else if (strcasecmp(token, "STORE") == 0)
                        {
                            char *first = strtok(NULL, "\n");
                            char *second = strtok(NULL, "\n");
                            char *msg = "STORE: OK\n";

                            int storeIdx = findAnimal(animals, first, count);

                            // store new animal 
                            if (storeIdx == -1)
                            {
                                // but check if storage is not full
                                if (count < 15)
                                {
                                    strcpy(sounds[count], second);

                                    strcpy(animals[count], first);

                                    count++;
                                }
                            }
                            else // update animal's sound
                            {
                                strcpy(sounds[storeIdx], second);
                            }

                            write(sockfd, msg, strlen(msg));
                        }
                        else if (strcasecmp(buf, "BYE") == 0)
                        {
                            char *msg = "BYE: OK\n";
                            write(sockfd, msg, strlen(msg));
                            printf("Closing client %d\n", sockfd);
                            client[i] = -1;
                            loginStatus[i] = 0;
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                        }
                        else if (strcasecmp(buf, "END") == 0)
                        {
                            char *msg1 = "END: OK\n";
                            
                            for (int i = 0; i <= maxi; i++)
                            {
                                sockfd = client[i];
                                if (sockfd > -1)
                                {
                                    printf("Closing client %d\n", sockfd);
                                    write(sockfd, msg1, strlen(msg1));
                                    client[i] = -1;
                                    loginStatus[i] = 0;
                                    close(sockfd);
                                    FD_CLR(sockfd, &allset);
                                    
                                }
                            }
                            writeFile(user, pass, numOfUser);
                            printf("Closing server..\n");
                            close(listenfd);
                            FD_CLR(listenfd, &allset);
                            exit(1);
                        }
                        else
                        {
                            int index = findAnimal(animals, token, count);

                            if (index == -1)
                            {
                                char temp[] = "I DON\'T KNOW ";
                                strcat(temp, buf);
                                strcat(temp, "\n");
                                write(sockfd, temp, strlen(temp));
                            }
                            else
                            {
                                char msg[] = "A ";
                                char *token = strtok(buf, "\n");
                                strcat(msg, token);
                                strcat(msg, " SAYS ");
                                strcat(msg, sounds[index]);
                                strcat(msg, "\n");

                                write(sockfd, msg, strlen(msg));
                            }
                        }
                    }
                }
                if (--nready <= 0)
                    break;
            }
        }
    }
}
