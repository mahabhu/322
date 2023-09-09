#include<bits/stdc++.h>
// #include <unistd.h>
using namespace std;
typedef long long int ll;



string padding(string datastring, ll m){
    ll n = datastring.size();
    if(n%m==0) return datastring;
    for(ll i=n%m; i<m; i++){
        datastring+="~";
    }
    return datastring;
}

bool ispower(ll n){
    ll h=0;
    while(n){
        h++;
        n = (n-1)&n;
    }
    return (h==1);
}

void printstr(vector<bool> v, bool hams = false){
    if(hams){
        for(ll i=0; i<v.size(); i++){
            if(ispower(i+1)){
                cout << "\033[31m" << v[i] << "\033[0m";
            }
            else{
                cout<< v[i];
            }
        }
        return;
    }

    for(ll i=0; i<v.size(); i++){
        cout<< v[i];
    }
    return;
}

vector<bool> ascii(char c){
    vector<bool> s(8);
    for(ll i=0; i<8; i++) s[7-i] = ((c>>i)&1);
    return s;
}

vector<vector<bool> > blocks(string datastring, ll m){
    vector<vector<bool> > v;
    for(ll i=0; i<datastring.size(); i+=m){
        vector<bool> app(m*8);
        for(ll j=i; j<i+m; j++){
            vector<bool> lol = ascii(datastring[j]);
            for(ll k=0; k<8; k++){
                app[(j-i)*8+k] = lol[k];
            }
        }
        v.push_back(app);
    }
    return v;
}



vector<bool> hamming(vector<bool> datastring){
    vector<bool> p(20);
    for(ll i=0; i<datastring.size(); i++){
        ll k = i+1;
        for(ll j=0; j<20; j++){
            if(((k>>j)&1)){
                p[j] = p[j]^datastring[i];
            }
        }
    }
    vector<bool> v;
    ll r = 1, pc = 0, dc = 0;
    while(dc<datastring.size()){
        if(ispower(r)){
            v.push_back(p[pc]); pc++;
        }
        else{
            v.push_back(datastring[dc]); dc++;
        }
        r++;
    }
    return v;
}


void solve(){

    std::cout << "\033[34m" << "blue" << "\033[0m" << std::endl;

    // Print "red" in red
    std::cout << "\033[31m" << "red" << "\033[0m" << std::endl;

    // Print "yellow" in yellow
    std::cout << "\033[33m" << "yellow" << "\033[0m" << std::endl;

    char str[100000];
    cin.getline(str,100000);
    string datastring=str;
    ll m; cin>> m;
    datastring = padding(datastring,m);
    cout<< datastring<< endl;
    // double p; cin>> p;
    // string polynom; cin>> polynom;
    vector<vector<bool> > blc = blocks(datastring,m);
    for(ll i=0; i<blc.size(); i++){
        printstr(blc[i]);
        cout<< endl;
    }
    cout<< endl;
    for(ll i=0; i<blc.size(); i++){
        printstr(hamming(blc[i]), true);
        cout<< endl;
    }
    

}

int main(){
    solve();
}