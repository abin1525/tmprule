int main(){
    int ix;
    float fx = 1.85;
    float fy = -1.85;
    ix = fx; // Wrong
    ix = (int)fy; // OK
    return 0;
}
