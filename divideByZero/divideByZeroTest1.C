int main(){
    int key = 1010;
    key = key / 20; // Rigth
    int ki = key/0; // Wrong
    int keyi = key%0; // Wrong
    long kl = key/0l; // Wrong
    long long kll = key/0ll; // Wrong
    return 0;
}
