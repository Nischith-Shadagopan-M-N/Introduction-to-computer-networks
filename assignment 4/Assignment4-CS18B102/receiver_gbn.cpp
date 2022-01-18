#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <ctime>
#include <math.h>
#include <set>
#include <sstream>
#include <map>
#include <queue>
#include <thread>
#include <unordered_map>
#include <mutex>
#include <random>

#define ll long long
#define MAXLEN 10000
#define INF LONG_LONG_MAX
#define TIMES 1000
#define PORT 8080
#define MFACTOR 2
#define SEED 1616751377
#define THRESH 500
using namespace std;

ll recport;
ll nbits=8;
ll maxPackets;
ll windowSize;
ll bufferSize;
double packetErrorRate;
bool debug = 0;
ll startt = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
unordered_map <int, ll> startTime;

void err(){
    cout<<"Invalid input"<<endl;
    cout<<"help:"<<endl;
    cout<<"./SenderSR ­-d* -s <string> ­-p <stirng> -­n <integer> -­L <integer> ­-R <integer> -N <integer> -W <integer> -B <integer>"<<endl;
    cout<<"* arguments are optional";
}

int main(int argc, char *argv[]) {
    if(argc<4){
        err();
        return 0;
    }
    for (int i = 1; i < argc; i+=2)
    {
        string inp = argv[i];
        stringstream s(argv[i+1]);
        if(inp=="-d"){
            debug = 1;
            i--;
        }
        else if(inp=="-p"){
            if(!(s>>recport)){
                err();
                return 0;
            }
        }
        else if(inp=="-e"){
            if(!(s>>packetErrorRate)){
                err();
                return 0;
            }
        }
        else if(inp=="-n"){
            if(!(s>>maxPackets)){
                err();
                return 0;
            }
        }
        else{
            err();
            return 0;
        }
    }
    int sockfd;
    char buffer[MAXLEN];
    struct sockaddr_in servaddr, cliaddr;
      
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
      
    servaddr.sin_family    = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(recport);

    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    socklen_t len;
    int n;
  
    default_random_engine generator(SEED);
    bernoulli_distribution distribution(1-packetErrorRate);

    len = sizeof(cliaddr);  
    ofstream ofile;
    int expectedSeqNo = 0;
    int maxdec = 1<<nbits;
    //ofile.open("output.txt");
    int nack = 0;
    while (1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLEN, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        buffer[n] = '\0';
        //ofile<<strlen(buffer)<<endl;
        if(distribution(generator)){
            stringstream s(buffer);
            int seqNo;
            s>>seqNo;
            if(seqNo == expectedSeqNo){
                startTime[seqNo] = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
                if(debug){
                    cout  <<  "Seq # : "  <<  seqNo  <<  " Time received : "  <<  startTime[seqNo]  /  1000  -  startt  /  1000<<endl;
                }
                nack++;
                string seq = "";
                seq += to_string(expectedSeqNo);
                seq += ' ';
                char msg[MAXLEN];
                strcpy(msg, seq.c_str());
                sendto(sockfd, (const char *)msg, strlen(msg), 
                MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                    len);
                expectedSeqNo++;
                expectedSeqNo %= maxdec;
            }
        }
    }
    ofile.close();
    return 0;
}