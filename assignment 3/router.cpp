#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define ll long long
#define MAXLEN 10000
#define INF LONG_LONG_MAX
#define TIMES 1000
using namespace std;

map < pair <ll, ll>, ll> minc;
map < pair <ll, ll>, ll> maxc;
map < pair <ll, ll>, ll> cost;
mutex costl;
map < ll, ll > lseq;

class neighbour
{
public:
    ll min;
    ll max;
    ll nodeid;
    struct sockaddr_in     servaddr;
    neighbour(ll id, ll mi, ll ma);
};

neighbour::neighbour(ll id, ll mi, ll ma)
{
    nodeid = id;
    min = mi;
    max = ma;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10000+id);
    servaddr.sin_addr.s_addr = INADDR_ANY;
}

map <ll, vector <neighbour> > neighbours;

int sockfd;
struct sockaddr_in servaddr;

void UDPserver(ll id){
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
      
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(10000+id);
      
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void err(){
    cout<<"Invalid input"<<endl;
    cout<<"help:"<<endl;
    cout<<"./ospf ­-i id -­f infile ­-o outfile -­h hi -­a lsai ­-s spfi"<<endl;
}

void input(ll id, string inf){
    ifstream ifile;
    ifile.open(inf);
    string spam;
    getline(ifile, spam); 
    stringstream s(spam);
    ll nnod;s>>nnod;
    ll e;s>>e;
    while(e--){
        ll i;
        ll j;
        ll mi;
        ll ma;
        ifile>>i;
        ifile>>j;
        ifile>>mi;
        ifile>>ma;
        neighbour n1(j, mi, ma);
        neighbour n2(i, mi, ma);
        if(neighbours.count(i)!=0){
            neighbours[i].push_back(n1);
        }
        else{
            vector <neighbour> v1;
            v1.push_back(n1);
            neighbours[i] = v1;
        }
        if(neighbours.count(j)!=0){
            neighbours[j].push_back(n2);
        }
        else{
            vector <neighbour> v2;
            v2.push_back(n2);
            neighbours[j] = v2;
        }
        if(i>j){
            swap(i, j);
        }
        minc[{i, j}] = mi;
        maxc[{i, j}] = ma;
        cost[{i, j}] = ma;
    }
    ifile.close();
}

int hello(double hi, ll id){
    //cout<<"in hello"<<endl;
    ll co = TIMES;
    while(co--){
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds((unsigned int)hi);
        char message[MAXLEN];
        sprintf(message, "HELLO %lld", id);
        for(neighbour n : neighbours[id])
        {
            if(n.nodeid>id){
                struct sockaddr_in     servaddr = n.servaddr;
                sendto(sockfd, (const char *)message, strlen(message),
                MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                    sizeof(servaddr));
            }
        }
        std::this_thread::sleep_until(x);
    }
    //cout<<"out hello"<<endl;
    return 0;
}

int receiv(ll nodeid){
    //cout<<"in receive"<<endl;
    while(true){
        socklen_t len;
        int n;

        char buffer[MAXLEN];
        char reply[MAXLEN];
        char copybuff[MAXLEN];
        struct sockaddr_in cliaddr;
    
        len = sizeof(cliaddr);
        //cout<<"belted"<<endl;
        n = recvfrom(sockfd, (char *)buffer, MAXLEN, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);
        buffer[n] = '\0';
        strcpy(copybuff, buffer);
        //cout<<"not belted"<<endl;
        stringstream s(buffer);
        string message;
        s>>message;
        unique_lock <mutex> lock(costl);
        if(message == "HELLO"){
            ll id;
            s>>id;
            ll mi;
            ll ma;
            mi = minc[{id, nodeid}];
            ma = maxc[{id, nodeid}];
            sprintf(reply, "HELLOREPLY %lld %lld %lld", nodeid, id, mi + rand()%(ma-mi+1));
            sendto(sockfd, (const char *)reply, strlen(reply), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);
        }
        else if(message == "HELLOREPLY"){
            ll i;
            ll j;
            ll co;
            s>>i>>j>>co;
            cost[{j, i}] = co;
        }
        else if(message == "LSA"){
            ll srcid;
            ll seqno;
            ll n;
            s>>srcid>>seqno>>n;
            if(srcid==nodeid){
                continue;
            }
            if(lseq.count(srcid)==0){
                lseq[srcid] = seqno;
                while(n--){
                    ll neigh;
                    ll cos;
                    s>>neigh>>cos;
                    cost[{min(neigh, srcid), max(srcid, neigh)}] = cos;
                }
                for(neighbour n : neighbours[nodeid])
                {
                    struct sockaddr_in     servaddr = n.servaddr;
                    if(servaddr.sin_port != cliaddr.sin_port){
                        sendto(sockfd, (const char *)copybuff, strlen(copybuff),
                        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                            sizeof(servaddr));
                    }
                }
            }
            else if(lseq[srcid]<seqno){
                while(n--){
                    ll neigh;
                    ll cos;
                    s>>neigh>>cos;
                    cost[{min(neigh, srcid), max(srcid, neigh)}] = cos;
                }
                for(neighbour n : neighbours[nodeid])
                {
                    struct sockaddr_in     servaddr = n.servaddr;
                    if(servaddr.sin_port != cliaddr.sin_port){
                        sendto(sockfd, (const char *)copybuff, strlen(copybuff),
                        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                            sizeof(servaddr));
                    }
                }
            }
        }
        lock.unlock();
    }
    //cout<<"out receive"<<endl;
}

ll seqNo = 0;

void lsa(ll id, double lsai){
    //cout<<"in lsa"<<endl;
    ll co = TIMES;
    while(co--){
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds((unsigned int)lsai);
        char message[MAXLEN];
        string mes = "LSA ";
        mes = mes + to_string(id) + " ";
        mes += to_string(seqNo) + " ";
        seqNo++;
        mes += to_string(neighbours[id].size()) + " ";
        unique_lock <mutex> lock(costl);
        for(neighbour n : neighbours[id]){
            ll j = n.nodeid;
            mes += to_string(j) + " ";
            if(id < j){
                mes += to_string(cost[{id, j}]) + " ";
            }
            else{
                mes += to_string(cost[{j, id}]) + " ";
            }
        }
        lock.unlock();
        strcpy(message, mes.c_str());
        for(neighbour n : neighbours[id])
        {
            struct sockaddr_in     servaddr = n.servaddr;
            sendto(sockfd, (const char *)message, strlen(message),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));
        }
        std::this_thread::sleep_until(x);
    }
    //cout<<"out lsa"<<endl;
}

ll dijcount = 1;

void dijkstra(ll src, string outf, double spfi){
    //cout<<"in dijkstra"<<endl;
    //cout<<src<<endl;
    ll co = TIMES;
    ofstream ofile;
    ofile.open(outf);
    while(co--){
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds((unsigned int)spfi);
        std::this_thread::sleep_until(x);
        multimap <ll, ll> dis;
        unordered_set <ll> todo;
        unordered_map <ll, ll> dist;
        map <ll, ll> prev;
        for(auto p : neighbours){
            if(p.first != src){
                todo.insert(p.first);
                dist[p.first] = INF;
            }
            else{
                dis.insert({0, p.first});
                todo.insert(p.first);
                dist[p.first] = 0;
            }
        }
        while(!todo.empty()){
            ll w;
            bool flag = 0;
            for(multimap <ll, ll>::iterator it = dis.begin();it!=dis.end();){
                if(todo.count(it->second)!=0){
                    w = it->second;
                    //ofile<<w<<" ";
                    flag = 1;
                    break;
                }
                else{
                    dis.erase(it++);
                }
            }
            if(flag){
                todo.erase(w);
                unique_lock <mutex> lock(costl);
                for(auto neighbour : neighbours[w]){
                    ll v = neighbour.nodeid;
                    //ofile<<v<<" ";
                    if(dist[v] > dist[w] + cost[{min(v, w), max(v, w)}]){
                        dist[v] = dist[w] + cost[{min(v, w), max(v, w)}];
                        dis.insert({dist[v], v});
                        prev[v] = w;
                    }
                }
                //ofile<<endl;
                lock.unlock();
            }
        }
        ofile<<"Routing Table for Node No. "+to_string(src)+" at Time "+to_string((ll)(dijcount*spfi/1000))<<endl;
        dijcount++;
        ofile<<"Destination"<<setw(15)<<"Path"<<setw(15)<<"Cost"<<endl;
        for(auto ele : prev){
            ofile<<ele.first;
            string path = "";
            ll nod = ele.first;
            path += to_string(nod) + "-";
            while(prev[nod]!=src){
                path += to_string(prev[nod]) + "-";
                nod = prev[nod];
            }
            path += to_string(src);
            reverse(path.begin(), path.end());
            ofile<<setw(25)<<path<<setw(15);
            ofile<<dist[ele.first]<<endl;
        }
    }
    ofile.close();
    //cout<<"out dijkstra"<<endl;
}


int main(int argc, char *argv[]){
    ll nodeid;
    string inf;
    string outf;
    double hi = 1;
    double lsai = 5;
    double spfi = 20;
    bool f1=0;
    bool f2=0;
    bool f3=0;
    if(argc%2==0){
        err();
        return 0;
    }
    for (int i = 1; i < argc; i+=2)
    {
        string inp = argv[i];
        stringstream s(argv[i+1]);
        if(inp=="-i"){
            if(!(s>>nodeid)){
                err();
                return 0;
            }
            f1 = 1;
        }
        else if(inp=="-f"){
            if(!(s>>inf)){
                err();
                return 0;
            }
            f2 = 1;
        }
        else if(inp=="-o"){
            if(!(s>>outf)){
                err();
                return 0;
            }
            f3 = 1;
        }
        else if(inp=="-h"){
            if(!(s>>hi)){
                err();
                return 0;
            }
        }
        else if(inp=="-a"){
            if(!(s>>lsai)){
                err();
                return 0;
            }
        }
        else if(inp=="-s"){
            if(!(s>>spfi)){
                err();
                return 0;
            }
        }
        else{
            err();
            return 0;
        }
    }
    if(f1&&f2&&f3==0){
        err();
        return 0;
    }
    hi *= 1000;
    lsai *= 1000;
    spfi *= 1000;
    outf += "-"+to_string(nodeid)+".txt";
    //remove if not required
    //srand(time(0));
    input(nodeid, inf);
    UDPserver(nodeid);
    thread t1(hello, hi, nodeid);
    t1.detach();
    thread t2(lsa, nodeid, lsai);
    t2.detach();
    thread t3(dijkstra, nodeid, outf, spfi);
    t3.detach();
    thread t4(receiv, nodeid);
    t4.detach();
    while(true){
        ;
    }
    //cout<<nodeid<<endl;
    //receiv(nodeid);
}