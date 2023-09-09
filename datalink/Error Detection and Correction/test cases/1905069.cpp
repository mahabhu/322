#include<bits/stdc++.h>
#include <unistd.h>
using namespace std;
typedef long long int ll;
#define vbool vector<bool>
#define vll vector<ll>

bool ispower(ll n){ while(n%2==0) n/=2; return (n==1); }

string inputline(){
    char str[1000];
    cin.getline(str,1000);
    string s = str;
    return s;
}

string padding(string s, ll m){
    ll n =  s.size();
    if(n%m==0) return s;
    for(ll i=n%m; i<m; i++) s = s + "~";
    return s;
}

string ascii(char c){
    string s = "";
    for(ll i=0; i<8; i++){
        char f = '0' + (c%2);
        c/=2;
        s = f + s;
    }
    return s;
}

string encode(string s){
    string q = "";
    for(ll i=0; i<s.size(); i++) q = q + ascii(s[i]);
    return q;
}

vbool strtobool(string s){
    ll n = s.size();
    vbool a(n);
    for(ll i=0; i<n; i++) (s[i]=='0')? a[i] = false : a[i] = true;
    return a;
}

bool isin(vll err, ll n){
    ll m = err.size();
    for(ll i=0; i<m; i++){
        if(err[i]==n) return true;
    }
    return false;
}

void printbool(vbool a, bool hamm = false, vll err = vll(0), ll offset=0){
    for(ll i=0; i<a.size(); i++){
        if(isin(err,i+offset)) cout << "\033[31m" << a[i] << "\033[0m";
        else if(hamm && ispower(i+1)) cout << "\033[32m" << a[i] << "\033[0m";
        else cout<< a[i]; 
    }
    cout<< endl; 
}

void printboolcrc(vbool a, ll n){
    for(ll i=0; i<a.size(); i++){
        if(i>=a.size()-n) cout << "\033[32m" << a[i] << "\033[0m";
        else cout<< a[i]; 
    }
    cout<< endl;
}

vbool hamming(vbool a){
    ll r=0;
    while((1<<r)<=a.size()+1){
        ll m = (1<<r)-1;
        a.insert(a.begin()+m,0);
        r++;
    }
    for(ll k=0; k<r; k++){
        bool p = 0;
        for(ll i=0; i<a.size(); i++) if(((i+1)&(1<<k))) p = p^a[i];
        a[(1<<k)-1] = p;
    }
    return a;
}

vbool serialize(vector<vbool> data){
    ll n = data[0].size(), m = data.size();
    vbool a(n*m);
    for(ll i=0; i<n; i++) for(ll j=0; j<m; j++) a[i*m+j]=data[j][i];
    return a;
}

vbool checksummer(vbool data, vbool poly){
    ll n = data.size(), m = poly.size();
    vbool crc(n+m-1), zero(m,0);
    vbool nomi[2]={zero,poly};
    for(ll i=0; i<n; i++) crc[i]=data[i];
    for(ll i=0; i<n; i++){
        bool g = crc[i];
        for(ll j=i; j<i+m; j++) crc[j]=crc[j]^nomi[g][j-i];
        crc[i] = data[i];
    }
    return crc;
}

pair<vbool,vll> sendreceive(vbool a, double p){
    p*=10000;
    ll nu = p, de = 1000000;
    srand(nu);
    vll err;
    for(ll i=0; i<a.size(); i++){
        ll h = rand()%de;
        if(h<nu){
            a[i] = a[i]^true;
            err.push_back(i);
        }
    }
    return make_pair(a,err);
}

bool checkagainst(vbool data, vbool poly){
    ll n = data.size(), m = poly.size();
    n-=m-1;
    vbool zero(m,0);
    vbool nomi[2]={zero,poly};
    for(ll i=0; i<n; i++){
        bool g = data[i];
        for(ll j=i; j<i+m; j++) data[j]=data[j]^nomi[g][j-i];
    }
    for(ll i=n; i<n+m-1; i++) if(data[i]) return false;
    return true;
}

vector<vbool> reblockwithcheck(vbool a, ll m, ll p){
    vector<vbool> data;
    ll n = (a.size()-p+1)/m;
    for(ll i=0; i<n; i++){
        vbool block(m);
        data.push_back(block);
    }
    for(ll i=0; i<m; i++){
        for(ll j=0; j<n; j++){
            data[j][i]=a[i*n+j];
        }
    }
    return data;
}

vbool prunecheckbits(vbool a){
    vbool b;
    for(ll i=0; i<a.size(); i++) if(!ispower(i+1)) b.push_back(a[i]);
    return b;
}

char booltochar(vbool a){
    char c=0;
    for(ll i=0; i<8; i++) c+=(a[i]<<(7-i));
    return c;
}

string booltostr(vbool a){
    string s="";
    for(ll i=0; i<a.size(); i+=8){
        vbool b(8);
        for(ll j=i; j<i+8; j++) b[j-i]=a[j];
        s = s + booltochar(b);
    }
    return s;
}

vbool rectify(vbool a){
    ll idx = 0;
    for(ll k=0; (1<<k)<=a.size(); k++){
        bool p = 0;
        for(ll i=0; i<a.size(); i++){
            if((i+1)&(1<<k)){
                p = p^a[i];
            }
        }
        if(p) idx+=(1<<k);
    }
    if(idx!=0){
        a[idx-1] = a[idx-1]^true;
    }
    return a;
}

string reconstruct(vector<vbool> datablock){
    string s = "";
    for(ll i=0; i<datablock.size(); i++) s = s + booltostr(datablock[i]);
    return s;
}

vll shuffle(vll err, ll n, ll m){
    for(ll i=0; i<err.size(); i++){
        ll g = err[i]/m, h = err[i]%m;
        err[i] = n*h+g;
    }
    return err;
}

int main(){
    cout<< "enter data string: ";
    string s = inputline();

    cout<< "enter number of data bytes in a row (m): ";
    ll m; cin>>m;

    cout<< "enter probability (p): ";
    double p; cin>> p;

    cout<< "enter generator polynomial: ";
    string poly; cin>> poly;

    cout<< endl<< endl;
    cout<< "data string after padding: ";
    s = padding(s,m);
    cout<< s<< endl;
    cout<< endl;

    cout<< "data block (ascii code of m characters per row): \n";
    vector<vbool> datablock(s.size()/m);
    for(ll i=0; i<s.size(); i+=m){
        datablock[i/m] = strtobool(encode(s.substr(i,m)));
        printbool(datablock[i/m]);
    }

    cout<< endl;
    cout<< "data block after adding check bits: \n";
    for(ll i=0; i<s.size(); i+=m){
        datablock[i/m] = hamming(datablock[i/m]);
        printbool(datablock[i/m],true);
    }

    m = datablock[0].size();

    cout<< endl;
    cout<< "data bits after column-wise serialization: \n";
    vbool serialized = serialize(datablock);
    printbool(serialized);
    
    cout<< endl;
    cout<< "data bits after adding CRC checksum (sent frame): \n";
    serialized = checksummer(serialized,strtobool(poly));
    printboolcrc(serialized,poly.size()-1);

    pair<vbool,vll> receive = sendreceive(serialized,p);
    vll flips = receive.second;
    serialized = receive.first;


    cout<< endl;
    cout<< "received frame:\n"; printbool(serialized,false,flips);
    cout<< endl;
    cout<< "result of CRC checksum matching: "; (checkagainst(serialized,strtobool(poly)))? cout<< "no error detected\n" : cout<< "error detected\n";
    cout<< endl;

    flips = shuffle(flips,datablock[0].size(),datablock.size());

    datablock = reblockwithcheck(serialized,m,poly.size());
    cout<< "data block after removing CRC checksum bits:\n";
    for(ll i=0; i<datablock.size(); i++){
        printbool(datablock[i],true,flips,i*datablock[i].size());
        datablock[i] = rectify(datablock[i]);
    }

    cout<< endl;
    cout<< "data block after removing checkbits:\n";
    for(ll i=0; i<datablock.size(); i++){
        datablock[i] = prunecheckbits(datablock[i]);
        printbool(datablock[i]);
    }
    cout<< endl;
    cout<< "output frame: "<< reconstruct(datablock)<< endl;
    cout<< endl;
    cout<< endl;
}
