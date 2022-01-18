#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_USERS 200
#define MAX_COMMAND 100
#define MAX_STRING_SIZE 400
#define MAX_MAIL_SIZE 1000
#define MAX_USER_ID 100

FILE * dat;
char buff[1000]; 
char** users;
char** passwords;
int* numberOfMails;
int currIndex = -1;
FILE *currMailptr = NULL;
int flag = 0;
int numUsers = 0;
int server_fd, new_socket, valread; 

void lstu(){
    bzero(buff, sizeof(buff));
    if(numUsers==0){
        sprintf(buff+strlen(buff), "No users created");
    }
    for(int i=0; i<numUsers; i++){
        sprintf(buff+strlen(buff), "%s ", users[i]);
    }
    sprintf(buff+strlen(buff), "\n");
    send(new_socket, buff, sizeof(buff), 0);
}

int userExists(char userId[]){
    for(int i=0; i<numUsers; i++){
        if(strcmp(userId, users[i])==0)
            return i;
    }
    return -1;
}

void addu(char userId[], char pw[]){
    bzero(buff, sizeof(buff));
    if(userExists(userId) != -1){
        sprintf(buff+strlen(buff), "Userid already present\n");
    }
    else{
        numUsers += 1;
        users = (char**)realloc(users, numUsers*sizeof(char *));
        users[numUsers-1] = (char*)malloc(strlen(userId)*sizeof(char));
        passwords = (char**)realloc(passwords, numUsers*sizeof(char *));
        passwords[numUsers-1] = (char*)malloc(strlen(pw)*sizeof(char));
        strcpy(users[numUsers-1], userId);
        strcpy(passwords[numUsers-1], pw);
        FILE *fp;
        char pat[500] = "mailserver/";
        strcat(pat, userId);
        fp = fopen(pat, "w");
        fclose(fp);
        numberOfMails = (int*)realloc(numberOfMails, (numUsers)*sizeof(int));
        numberOfMails[numUsers-1] = 0;
        sprintf(buff+strlen(buff), "User : %s created\n", userId);
    }
    send(new_socket, buff, sizeof(buff), 0);
}

int user(char userId[]){
    bzero(buff, sizeof(buff));
    currIndex = userExists(userId);
    if(currIndex == -1){
        sprintf(buff+strlen(buff), "User ID doesnt exist\n");
        send(new_socket, buff, sizeof(buff), 0);
        return 0;
    }
    else{
        flag = 1;
        char pw[MAX_USER_ID];
        sprintf(buff+strlen(buff), "User ID exists\n");
        send(new_socket, buff, sizeof(buff), 0);
        bzero(buff, sizeof(buff)); 
        read(new_socket, buff, sizeof(buff));
        strcpy(pw, buff);
        if(strcmp(pw, passwords[currIndex])==0){
            bzero(buff, sizeof(buff));
            sprintf(buff+strlen(buff), "User ID exists and has %d messages\n", numberOfMails[currIndex]);
            char pat[500] = "mailserver/";
            strcat(pat, userId);
            currMailptr = fopen(pat, "r");
            if(currMailptr != NULL)
                rewind(currMailptr);
        }
        else{
            bzero(buff, sizeof(buff));
            sprintf(buff+strlen(buff), "Incorrect password\n");
            send(new_socket, buff, sizeof(buff), 0);
            return 0;
        }
    }
    send(new_socket, buff, sizeof(buff), 0);
    return 1;
}

void readm(){
    bzero(buff, sizeof(buff));
    if(flag==0){
        sprintf(buff+strlen(buff), "User has not been set\n");
        send(new_socket, buff, sizeof(buff), 0);
        return;
    }
    if(currMailptr == NULL || getc(currMailptr) == EOF){
        sprintf(buff+strlen(buff), "No More Mail\n");
    }
    else{
        fseek(currMailptr, -1L, SEEK_CUR);
        char mail[MAX_MAIL_SIZE];
        while(1){
            mail[0] = '\0';
            fscanf(currMailptr, "%[^#]", mail);
            sprintf(buff+strlen(buff), "%s", mail);
            mail[0] = '\0';
            fscanf(currMailptr, "%[#]", mail);
            sprintf(buff+strlen(buff), "%s", mail);
            if(strcmp("###", mail)==0){
                fscanf(currMailptr, "\n");
                sprintf(buff+strlen(buff), "\n");
                break;
            }
        }
        char check = getc(currMailptr);
        if(check == EOF){
            rewind(currMailptr);
        }
        else{
            fseek(currMailptr, -1L, SEEK_CUR);
        }
    }
    send(new_socket, buff, sizeof(buff), 0);
}

void delm(){
    bzero(buff, sizeof(buff));
    if(flag==0){
        sprintf(buff+strlen(buff), "User has not been set\n");
        send(new_socket, buff, sizeof(buff), 0);
        return;
    }
    if(currMailptr == NULL || getc(currMailptr) == EOF){
        sprintf(buff+strlen(buff), "No More Mail\n");
    }
    else{
        fseek(currMailptr, -1L, SEEK_CUR);
        char userId[MAX_STRING_SIZE];
        strcpy(userId, users[currIndex]);
        if(numberOfMails[currIndex]==1){
            char mail[MAX_MAIL_SIZE];
            while(1){
                mail[0] = '\0';
                fscanf(currMailptr, "%[^#]", mail);
                mail[0] = '\0';
                fscanf(currMailptr, "%[#]", mail);
                if(strcmp("###", mail)==0){
                    fscanf(currMailptr, "\n");
                    break;
                }
            }
            fclose(currMailptr);
            char pat[500] = "mailserver/";
            strcat(pat, userId);
            currMailptr = fopen(pat, "w");
            fclose(currMailptr);
            currMailptr = fopen(pat, "r");
        }
        else{
            long int temp = ftell(currMailptr);
            rewind(currMailptr);
            FILE* tempfile = fopen("mailserver/temp.tmp", "w");
            char c;
            for(int i=0;i<temp;i++){
                c = getc(currMailptr);
                putc(c, tempfile);
            }
            char mail[MAX_MAIL_SIZE];
            while(1){
                mail[0] = '\0';
                fscanf(currMailptr, "%[^#]", mail);
                mail[0] = '\0';
                fscanf(currMailptr, "%[#]", mail);
                if(strcmp("###", mail)==0){
                    fscanf(currMailptr, "\n");
                    break;
                }
            }
            while(c = getc(currMailptr) != EOF){
                putc(c, tempfile);
            }
            fclose(currMailptr);
            fclose(tempfile);
            remove(userId);
            rename("mailserver/temp.tmp", userId);
            char pat[500] = "mailserver/";
            strcat(pat, userId);
            currMailptr = fopen(pat, "r");
            fseek(currMailptr, temp, SEEK_SET);
            if(getc(currMailptr) == EOF){
                rewind(currMailptr);
            }
            else{
                fseek(currMailptr, -1L, SEEK_CUR);
            }
        }
        sprintf(buff+strlen(buff), "Message Deleted\n");
        numberOfMails[currIndex] -= 1;
    }
    send(new_socket, buff, sizeof(buff), 0);
}

void sendm(char userId[], char content[]){
    bzero(buff, sizeof(buff));
    if(flag==0){
        sprintf(buff+strlen(buff), "User has not been set\n");
        send(new_socket, buff, sizeof(buff), 0);
        return;
    }
    char fuser[MAX_STRING_SIZE];
    strcpy(fuser, users[currIndex]);
    int index = userExists(userId);
    char pat[500] = "mailserver/";
    strcat(pat, userId);
    FILE *rfp = fopen(pat, "a");
    time_t t;
    time(&t);
    char subject[MAX_MAIL_SIZE];
    sscanf(content, "%s", subject);
    fprintf(rfp, "From: %s\nTo: %s\nDate: %sSubject: %s\n%s###\n", fuser, userId, ctime(&t), subject, content);
    //sprintf(buff+strlen(buff), "From: %s\nTo: %s\n. . .\n%s\n. . .\n###\n", fuser, userId, content);
    fclose(rfp);
    numberOfMails[index] += 1;
}

void forwardm(char userId[]){
    bzero(buff, sizeof(buff));
    if(currMailptr == NULL || getc(currMailptr) == EOF){
        sprintf(buff+strlen(buff), "No Mail to forward\n");
    }
    else{
        fseek(currMailptr, -1L, SEEK_CUR);
        long int temp = ftell(currMailptr);
        char mail[MAX_MAIL_SIZE];
        char message[MAX_MAIL_SIZE];
        bzero(message, sizeof(message));
        for (int i = 0; i < 4; i++)
        {
            fscanf(currMailptr, "%[^\n]", mail);
            getc(currMailptr);
        }
        while(1){
            mail[0] = '\0';
            fscanf(currMailptr, "%[^#]", mail);
            sprintf(message+strlen(message), "%s", mail);
            mail[0] = '\0';
            fscanf(currMailptr, "%[#]", mail);
            if(strcmp("###", mail)==0){
                //sprintf(message+strlen(message), "\n");
                fscanf(currMailptr, "\n");
                break;
            }
            sprintf(message+strlen(message), "%s", mail);
        }
        fseek(currMailptr, temp, SEEK_SET);
        sendm(userId, message);
        sprintf(buff+strlen(buff), "Mail forwarded\n");
    }
    send(new_socket, buff, sizeof(buff), 0);
}

void doneu(){
    bzero(buff, sizeof(buff));
    if(flag==0){
        sprintf(buff+strlen(buff), "User has not been set\n");
        send(new_socket, buff, sizeof(buff), 0);
        return;
    }
    else{
        sprintf(buff+strlen(buff), "%s's session terminated\n", users[currIndex]);
    }
    flag = 0;
    send(new_socket, buff, sizeof(buff), 0);
}

int main(int argc, char *argv[]){
    mkdir("mailserver", 0700);
    if( access( "data", F_OK ) == 0 ) {
        dat = fopen("data", "r");
        int nu;
        fscanf(dat, "%d\n", &nu);
        char temp[1000];
        char tpw[1000];
        int temp2;
        for(int i=0;i<nu;i++){    
            fscanf(dat, "%s %d %s\n", temp, &temp2, tpw);
            numUsers += 1;
            users = (char**)realloc(users, numUsers*sizeof(char *));
            users[numUsers-1] = (char*)malloc(strlen(temp)*sizeof(char));
            strcpy(users[numUsers-1], temp);
            passwords = (char**)realloc(passwords, numUsers*sizeof(char *));
            passwords[numUsers-1] = (char*)malloc(strlen(tpw)*sizeof(char));
            strcpy(passwords[numUsers-1], tpw);
            numberOfMails = (int*)realloc(numberOfMails, (numUsers)*sizeof(int));
            numberOfMails[numUsers-1] = temp2;
        }
        fclose(dat);
    }
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
    
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, 
                                                &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    int port = atoi(argv[1]);
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( port ); 
    
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    while (1){
        dat = fopen("data", "w");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                        (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 

        int quit = 1;
        char input[MAX_COMMAND];
        while(quit){
            bzero(buff, sizeof(buff)); 
            read(new_socket, buff, sizeof(buff));
            strcpy(input, buff);
            if(strcmp(input, "Listusers")==0){
                lstu();
            }
            else if(strcmp(input, "Adduser")==0){
                char userid[MAX_USER_ID];
                char pw[MAX_USER_ID];
                bzero(buff, sizeof(buff)); 
                read(new_socket, buff, sizeof(buff));
                strcpy(userid, buff);
                bzero(buff, sizeof(buff)); 
                sprintf(buff+strlen(buff), "User ID received\n");
                send(new_socket, buff, sizeof(buff), 0);
                bzero(buff, sizeof(buff)); 
                read(new_socket, buff, sizeof(buff));
                strcpy(pw, buff);
                addu(userid, pw);
            }
            else if(strcmp(input, "SetUser")==0){
                char userid[MAX_USER_ID];
                bzero(buff, sizeof(buff)); 
                read(new_socket, buff, sizeof(buff));
                strcpy(userid, buff);
                int done = user(userid); 
                char inp[MAX_COMMAND];
                while(done){
                    bzero(buff, sizeof(buff)); 
                    read(new_socket, buff, sizeof(buff));
                    strcpy(inp, buff);
                    if(strcmp(inp, "Read")==0){
                        readm();
                    }
                    else if(strcmp(inp, "Delete")==0){
                        delm();
                    }
                    else if(strcmp(inp, "Send")==0){
                        char recinp[MAX_USER_ID];
                        bzero(buff, sizeof(buff)); 
                        read(new_socket, buff, sizeof(buff));
                        strcpy(recinp, buff);
                        char *recid = strtok(recinp, " "); 
                        while(recid!=NULL){
                            bzero(buff, sizeof(buff));
                            int tf = userExists(recid);
                            if(tf == -1){
                                sprintf(buff+strlen(buff), "User ID doesnt exist\n");
                                send(new_socket, buff, sizeof(buff), 0);
                                recid = strtok(NULL, " ");
                                continue;
                            }
                            else{
                                sprintf(buff+strlen(buff), "User ID exists\n");
                            }
                            send(new_socket, buff, sizeof(buff), 0);
                            char message[MAX_MAIL_SIZE];
                            bzero(buff, sizeof(buff)); 
                            read(new_socket, buff, sizeof(buff));
                            strcpy(message, buff);
                            sendm(recid, message);
                            recid = strtok(NULL, " ");
                        }
                    }
                    else if(strcmp(inp, "Forward")==0){
                        char recinp[MAX_USER_ID];
                        bzero(buff, sizeof(buff)); 
                        read(new_socket, buff, sizeof(buff));
                        strcpy(recinp, buff);
                        char *recid = strtok(recinp, " "); 
                        while(recid!=NULL){
                            bzero(buff, sizeof(buff));
                            int tf = userExists(recid);
                            if(tf == -1){
                                sprintf(buff+strlen(buff), "User ID doesnt exist\n");
                                send(new_socket, buff, sizeof(buff), 0);
                                recid = strtok(NULL, " ");
                                continue;
                            }
                            else{
                                sprintf(buff+strlen(buff), "User ID exists\n");
                            }
                            send(new_socket, buff, sizeof(buff), 0);
                            forwardm(recid);
                            recid = strtok(NULL, " ");
                        }
                    }
                    else if(strcmp(inp, "Done")==0){
                        done = 0;
                        doneu();
                    }
                }
            }
            else if(strcmp(input, "Quit")==0){
                quit = 0;
                close(new_socket);
            }
        }
        fprintf(dat, "%d\n", numUsers);
        for(int i=0;i<numUsers;i++){    
            fprintf(dat, "%s %d %s\n", users[i], numberOfMails[i], passwords[i]);
        }
        fclose(dat);
    }
    return 0;
}