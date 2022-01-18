#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#define PORT 8080 

#define MAX_COMMAND 100
#define MAX_USERS 200
#define MAX_STRING_SIZE 400
#define MAX_MAIL_SIZE 1000
#define MAX_USER_ID 100

char buff[1000]; 
char** users;
int numUsers = 0;

int main(int argc, char *argv[]){
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    int port = atoi(argv[2]);
    serv_addr.sin_port = htons(port); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 


    char input[MAX_COMMAND];
    int flag = 1;
    while (flag)
    {
        printf("%s", ">");
        scanf("%s", input);
        bzero(buff, sizeof(buff)); 
        strcpy(buff, input);
        send(sock, buff, sizeof(buff), 0);
        if(strcmp(input, "Listusers")==0){
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff)); 
            printf("%s", buff);
        }
        else if(strcmp(input, "Adduser")==0){
            char userid[MAX_USER_ID];
            scanf("%s", userid);
            bzero(buff, sizeof(buff)); 
            strcpy(buff, userid);
            send(sock, buff, sizeof(buff), 0);
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff)); 
            printf("Set password (no spaces allowed) :\n");
            char pw[MAX_USER_ID];
            scanf("%s", pw);
            bzero(buff, sizeof(buff)); 
            strcpy(buff, pw);
            send(sock, buff, sizeof(buff), 0);
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff));
            printf("%s", buff);
        }
        else if(strcmp(input, "SetUser")==0){
            char userid[MAX_USER_ID];
            char pw[MAX_USER_ID];
            scanf("%s", userid);
            bzero(buff, sizeof(buff)); 
            strcpy(buff, userid);
            send(sock, buff, sizeof(buff), 0);
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff)); 
            if(strcmp(buff, "User ID doesnt exist\n")==0){
                printf("%s", buff);
                continue;
            }
            printf("Enter password:\n");
            scanf("%s", pw);
            bzero(buff, sizeof(buff)); 
            strcpy(buff, pw);
            send(sock, buff, sizeof(buff), 0);
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff)); 
            printf("%s", buff);
            if(strcmp(buff, "Incorrect password\n")==0){
                continue;
            }
            int done = 1;
            char inp[MAX_COMMAND];
            while(done){
                printf("%s", userid);
                printf("%s", ">");
                fflush(stdin); 
                scanf("%s", inp);
                bzero(buff, sizeof(buff)); 
                strcpy(buff, inp);
                send(sock, buff, sizeof(buff), 0);
                if(strcmp(inp, "Read")==0){
                    bzero(buff, sizeof(buff)); 
                    read(sock, buff, sizeof(buff)); 
                    printf("%s", buff);
                }
                else if(strcmp(inp, "Delete")==0){
                    bzero(buff, sizeof(buff)); 
                    read(sock, buff, sizeof(buff)); 
                    printf("%s", buff);
                }
                else if(strcmp(inp, "Send")==0){
                    bzero(buff, sizeof(buff)); 
                    char recinp[MAX_USER_ID];
                    char c;
                    scanf("%c", &c);
                    scanf("%[^\n]", recinp);
                    scanf("%c", &c);
                    sprintf(buff+strlen(buff), "%s", recinp);
                    send(sock, buff, sizeof(buff), 0);
                    char *recid = strtok(recinp, " "); 
                    printf("%s", "Type Message:\n");
                    bzero(buff, sizeof(buff));
                    char message[MAX_MAIL_SIZE];
                    while(1){
                        message[0] = '\0';
                        scanf("%[^#]", message);
                        sprintf(buff+strlen(buff), "%s", message);
                        message[0] = '\0';
                        scanf("%[#]", message);
                        if(strcmp("###", message)==0){
                            sprintf(buff+strlen(buff), "\n");
                            break;
                        }
                        sprintf(buff+strlen(buff), "%s", message);
                    } 
                    strcpy(message, buff);
                    while(recid!=NULL){
                        bzero(buff, sizeof(buff)); 
                        read(sock, buff, sizeof(buff)); 
                        if(strcmp(buff, "User ID doesnt exist\n")==0){
                            printf("User ID : %s doesnt exist\n", recid);
                            recid = strtok(NULL, " ");
                            continue;
                        }
                        send(sock, message, sizeof(message), 0);
                        recid = strtok(NULL, " ");
                    }
                }
                else if(strcmp(inp, "Forward")==0){
                    bzero(buff, sizeof(buff)); 
                    char recinp[MAX_USER_ID];
                    char c;
                    scanf("%c", &c);
                    scanf("%[^\n]", recinp);
                    scanf("%c", &c);
                    sprintf(buff+strlen(buff), "%s", recinp);
                    send(sock, buff, sizeof(buff), 0);
                    char *recid = strtok(recinp, " "); 
                    while(recid!=NULL){
                        bzero(buff, sizeof(buff)); 
                        read(sock, buff, sizeof(buff)); 
                        if(strcmp(buff, "User ID doesnt exist\n")==0){
                            printf("User ID : %s doesnt exist\n", recid);
                            recid = strtok(NULL, " ");
                            continue;
                        }
                        bzero(buff, sizeof(buff)); 
                        read(sock, buff, sizeof(buff)); 
                        printf("%s", buff);
                        recid = strtok(NULL, " ");
                    }
                }
                else if(strcmp(inp, "Done")==0){
                    done = 0;
                    bzero(buff, sizeof(buff)); 
                    read(sock, buff, sizeof(buff)); 
                    printf("%s", buff);
                }
                else if(strcmp(inp, "help")==0){
                    printf("Read : Read current message\n");
                    printf("Delete : Delete current message\n");
                    printf("Send <receiverid1> <receiverid2> ... <receiveridn> : Send message to specified users\n");
                    printf("Forward <receiverid1> <receiverid2> ... <receiveridn> : Forward message to specified users\n");
                    printf("Done : Done with current user\n");
                }
                else{
                    printf("unrecognized command \'%s\'\n", inp);
                    fflush(stdin); 
                    continue;
                }
            }
        }
        else if(strcmp(input, "Quit")==0){
            flag = 0;
            bzero(buff, sizeof(buff)); 
            read(sock, buff, sizeof(buff)); 
            printf("%s", buff);
            close(sock);
        }
        else if(strcmp(input, "help")==0){
            printf("Listusers : List of users separated by spaces\n");
            printf("Adduser : Add user to server\n");
            printf("SetUser : Select user\n");
            printf("Quit : Close connection with server\n");
        }
        else if(strcmp(input, "Read")==0){
            printf("User has not been set\n");
        }
        else if(strcmp(input, "Delete")==0){
            printf("User has not been set\n");
        }
        else if(strcmp(input, "Send")==0){
            printf("User has not been set\n");
        }
        else if(strcmp(input, "Forward")==0){
            printf("User has not been set\n");
        }
        else if(strcmp(input, "Done")==0){
            printf("User has not been set\n");
        }
        else{
            printf("unrecognized command \'%s\'\n", input);
            fflush(stdin); 
            continue;
        }
    }
    return 0;
}