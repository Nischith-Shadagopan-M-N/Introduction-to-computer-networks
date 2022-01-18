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

#define ll long long
#define MAXLEN 10000
#define INF LONG_LONG_MAX
#define TIMES 1000
#define PORT 8080
#define MFACTOR 1500
#define THRESH 500
using namespace std;

string receiver;
ll recport;
ll nbits;
ll maxPacketLength;
ll packetGenRate;
ll maxPackets;
ll windowSize;
ll bufferSize;
bool debug = 0;

int sockfd;
struct sockaddr_in servaddr;

void UDPclient(){
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(recport);
    servaddr.sin_addr.s_addr = INADDR_ANY;
}

void err(){
    cout<<"Invalid input"<<endl;
    cout<<"help:"<<endl;
    cout<<"./SenderSR ­-d* -s <string> ­-p <stirng> -­n <integer> -­L <integer> ­-R <integer> -N <integer> -W <integer> -B <integer>"<<endl;
    cout<<"* arguments are optional";
}

int maxdec = 1;
queue <string> buffer;
mutex bufferl;
int npackets = 0;
mutex npacketsl;
set <int> unackpackets;
mutex unackpacketsl;
int seqNo=0;
int nunack = 0;
int nack = 0;
mutex nackl;
double rtt = 0;
mutex rttl;
int mretransmit = 0;
unordered_map <int, string> transmitbuff;
unordered_map <int, ll> startTime;
unordered_map<int, ll> attempts;
mutex startTimel;
ll startt = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();

void timeout(unsigned int ttl, int seqno){
    unsigned int timeo = ttl;
    int temp = 0;
    while(1){
        if(timeo<50){
            timeo = 50;
        }
        auto x = std::chrono::steady_clock::now() + std::chrono::microseconds(timeo);
        std::this_thread::sleep_until(x);
        unique_lock <mutex> lock(unackpacketsl);
        //cout<<"Entered unackpacketsl"<<endl;
        bool flag = unackpackets.count(seqno)>0;
        //cout<<"Exited unackpacketsl"<<endl;
        lock.unlock();
        if(flag){
            char msg[MAXLEN];
            string seq = transmitbuff[seqno];
            strcpy(msg, seq.c_str());
            sendto(sockfd, (const char *)msg, strlen(msg),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));
            unique_lock <mutex> lock(npacketsl);
            //cout<<"Entered npacketsl"<<endl;
            if(npackets<10){
                timeo = 300000;
            }
            else{
                unique_lock <mutex> lock1(rttl);
                //cout<<"Entered rttl"<<endl;
                timeo = MFACTOR * rtt;
                //cout<<"Exited rttl"<<endl;
                lock1.unlock();
            }
            npackets++;
            //cout<<"Exited npacketsl"<<endl;
            lock.unlock();
            temp++;
            unique_lock <mutex> lock1(nackl);
            //cout<<"Entered nackl"<<endl;
            mretransmit = max(mretransmit, temp);
            /*if(mretransmit==10){
                break;
            }*/
            //cout<<"Exited nackl"<<endl;
            lock1.unlock();
            unique_lock <mutex> lock2(startTimel);
            attempts[seqno]++;
            //startTime[seqno] = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
            lock2.unlock();
        }
        else{
            break;
        }
        //cout<<"timeout "<<seqno<<endl;
    }
}

void genPacket(){
    bool flag = 1;
    while(flag){
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds((unsigned int)1000/packetGenRate);
        std::this_thread::sleep_until(x);
        int length = 39 + rand()%(maxPacketLength-39);
        string message = "";
        for (int i = 0; i < length; i++)
        {
            message += 'a';
        }
        unique_lock <mutex> lock(bufferl);
        //cout<<"Entered bufferl1"<<endl;
        if(buffer.size()<bufferSize){
            buffer.push(message);
        }
        //cout<<"Exited bufferl"<<endl;
        lock.unlock();
        unique_lock <mutex> lock1(nackl);
        //cout<<"Entered nackl"<<endl;
        flag = nack<maxPackets && mretransmit<=10;
        //cout<<"Exited nackl"<<endl;
        lock1.unlock();
        //cout<<"packet generated"<<endl;
    }
}

void sendPacket(){
    bool flag = 1;
    ofstream ofile;
    while(flag){
        if(nunack<windowSize){
            string seq = "";
            seq += to_string(seqNo);
            seq += ' ';
            unique_lock <mutex> lock(bufferl);
            //cout<<"Entered bufferl2"<<endl;
            if(buffer.size()!=0){
                seq += buffer.front();
                transmitbuff[seqNo] = seq;
                //cout<<seq<<endl;
                buffer.pop();
            }
            else{
                //cout<<"Exited bufferl"<<endl;
                lock.unlock();
                continue;
            }
            //cout<<"Exited bufferl"<<endl;
            lock.unlock();
            char msg[MAXLEN];
            strcpy(msg, seq.c_str());
            sendto(sockfd, (const char *)msg, strlen(msg),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));
            unique_lock <mutex> lock3(startTimel);
            startTime[seqNo] = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
            attempts[seqNo] = 1;
            lock3.unlock();
            unique_lock <mutex> lock1(npacketsl);
            //cout<<"Entered npacketsl"<<endl;
            if(npackets<10){
                thread t1(timeout, 300000, seqNo);
                t1.detach();
            }
            else{
                unique_lock <mutex> lock1(rttl);
                //cout<<"Entered rttl"<<endl;
                thread t2(timeout, MFACTOR * rtt, seqNo);
                t2.detach();
                //cout<<"Exited rttl"<<endl;
                lock1.unlock();
            }
            unique_lock <mutex> lock2(unackpacketsl);
            //cout<<"Entered unackpacketsl"<<endl;
            unackpackets.insert(seqNo);
            nunack++;
            //cout<<"Exited unackpacketsl"<<endl;
            lock2.unlock();
            npackets++;
            //cout<<"Exited npacketsl"<<endl;
            lock1.unlock();
            seqNo++;
            seqNo %= maxdec;
        }
        unique_lock <mutex> lock1(nackl);
        //cout<<"Entered nackl"<<endl;
        flag = nack<maxPackets && mretransmit<=10;
        //cout<<"Exited nackl"<<endl;
        lock1.unlock();
        //cout<<"packet sent"<<endl;
    }
    ofile.close();
}

void acknowledge(){
    socklen_t len;
    int n;
    char buffer[MAXLEN];
    bool flag = 1;
    while(flag){
        n = recvfrom(sockfd, (char *)buffer, MAXLEN, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
        buffer[n] = '\0';
        stringstream s(buffer);
        int seqno;
        s>>seqno;
        ll tim = chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
        set <int> acked;
        // need to fix this function for cumulative acknowledgement
        unique_lock <mutex> lock(unackpacketsl);
        //cout<<"Entered unackpacketsl"<<endl;
        for(auto x : unackpackets){
            if(x>seqno){
                break;
            }
            acked.insert(x);
        }
        for(auto x : acked){
            unackpackets.erase(x);
            nunack--;
        }
        //cout<<"Exited unackpacketsl"<<endl;
        lock.unlock();
        unique_lock <mutex> lock1(nackl);
        //cout<<"Entered nackl"<<endl;
        unique_lock <mutex> lock2(rttl);
        //cout<<"Entered rttl"<<endl;
        unique_lock <mutex> lock3(startTimel);
        for(auto x: acked){
            double temp = tim - startTime[x];
            if(temp<THRESH || attempts[x]>1){
                rtt = rtt * nack + temp;
                nack++;
                rtt /= nack; 
            }
            else{
                nack++;
            }  
            if(debug){
                cout  <<  "Seq # : "  <<  x  <<  " Time generated : "  <<  startTime[x]  /  1000  -  startt  /  1000  <<  ":"  <<  startTime[x]  -  (startTime[x]  /  1000)  *  1000 <<  " RTT : "  <<  temp  <<  " Number of attempts : "  <<  attempts[x]  <<  endl;
            }
        }
        lock3.unlock();
        //cout<<"Exited rttl"<<endl;
        lock2.unlock();
        flag = nack<maxPackets && mretransmit<=10;
        //cout<<"Exited nackl"<<endl;
        lock1.unlock();
        //cout<<"packet acknowledged"<<endl;
    }
    cout<<"Packet generation rate : "<<packetGenRate<<endl;
    cout<<"Packet length : "<<maxPacketLength<<endl;
    cout<<"Retransmission ratio : "<<(double)npackets/nack<<endl;
    cout<<"Average RTT value : "<<rtt<<endl;
}

int main(int argc, char *argv[]){
    if(argc<9){
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
        else if(inp=="-s"){
            if(!(s>>receiver)){
                err();
                return 0;
            }
        }
        else if(inp=="-p"){
            if(!(s>>recport)){
                err();
                return 0;
            }
        }
        else if(inp=="-n"){
            if(!(s>>nbits)){
                err();
                return 0;
            }
        }
        else if(inp=="-L"){
            if(!(s>>maxPacketLength)){
                err();
                return 0;
            }
        }
        else if(inp=="-R"){
            if(!(s>>packetGenRate)){
                err();
                return 0;
            }
        }
        else if(inp=="-N"){
            if(!(s>>maxPackets)){
                err();
                return 0;
            }
        }
        else if(inp=="-W"){
            if(!(s>>windowSize)){
                err();
                return 0;
            }
        }
        else if(inp=="-B"){
            if(!(s>>bufferSize)){
                err();
                return 0;
            }
        }
        else{
            err();
            return 0;
        }
    }
    maxdec = maxdec<<nbits;
    UDPclient();
    thread t1(genPacket);
    t1.detach();
    //cout<<"hello1"<<endl;
    thread t2(sendPacket);
    t2.detach();
    //cout<<"hello2"<<endl;
    thread t3(acknowledge);
    t3.detach();
    //cout<<"hello3"<<endl;
    while (true)
    {
        ;
    }
}