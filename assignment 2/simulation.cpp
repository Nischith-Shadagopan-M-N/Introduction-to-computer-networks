#include <bits/stdc++.h>
#define ll long long
#define CONGEST_THRESHOLD_INIT 2147483647
#define K_I_INIT 1
#define K_M_INIT 1
#define K_N_INIT 1
#define K_F_INIT 0.5
#define P_TIMEOUT 0.01
#define MSS 1
#define REC_WINDOW_INIT 1000
#define SEED 1616751377

using namespace std;

void err(){
    cout<<"Invalid input"<<endl;
    cout<<"help:"<<endl;
    cout<<"./cw ­i <double> ­m <double> ­n <double> ­f <double> ­s <double> ­T <int> ­o outfile"<<endl;
}

int main(int argc, char *argv[]){
    double Ki = K_I_INIT;
    double Km = K_M_INIT;
    double Kn = K_N_INIT;
    double Kf = K_F_INIT;
    double congestionWindow = Ki*MSS;
    ll recWindow = REC_WINDOW_INIT;
    double congestionThreshold = CONGEST_THRESHOLD_INIT;
    ll windowSize;
    ll count;
    double probTimeOut = P_TIMEOUT;
    string outf;
    bool chf=0;
    if(argc%2==0){
        err();
        return 0;
    }
    for (int i = 1; i < argc; i+=2)
    {
        string inp = argv[i];
        stringstream s(argv[i+1]);
        if(inp=="-i"){
            if(!(s>>Ki)){
                err();
                return 0;
            }
        }
        else if(inp=="-m"){
            if(!(s>>Km)){
                err();
                return 0;
            }
        }
        else if(inp=="-n"){
            if(!(s>>Kn)){
                err();
                return 0;
            }
        }
        else if(inp=="-f"){
            if(!(s>>Kf)){
                err();
                return 0;
            }
        }
        else if(inp=="-s"){
            if(!(s>>probTimeOut)){
                err();
                return 0;
            }
        }
        else if(inp=="-T"){
            if(!(s>>count)){
                err();
                return 0;
            }
        }
        else if(inp=="-o"){
            if(!(s>>outf)){
                err();
                return 0;
            }
            chf = 1;
        }
        else{
            err();
            return 0;
        }
    }
    if(chf==0){
        cout<<"output file not provided"<<endl;
        return 0;
    }
    default_random_engine generator(SEED);
    bernoulli_distribution distribution(1-probTimeOut);
    ofstream fout;
    fout.open(outf);
    //cout<<count<<endl;
    while(count>0){
        windowSize = ceil(congestionWindow);
        for (int i = 0; i < windowSize; i++)
        {
            count--;
            if(distribution(generator)){
                if(congestionWindow < congestionThreshold){
                    congestionWindow = min(congestionWindow + Km*MSS, (double)recWindow);
                }
                else{
                    congestionWindow = min(congestionWindow + Kn* MSS * MSS / congestionWindow, (double)recWindow);
                }
            }
            else{
                congestionThreshold = congestionWindow / 2;
                congestionWindow = max((double)1, Kf * congestionWindow);
                fout<<congestionWindow<<endl;
                break;
            }
            fout<<congestionWindow<<endl;
        }
        
    }
    fout.close();
}