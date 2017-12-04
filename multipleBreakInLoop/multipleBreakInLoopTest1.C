int main(){
    int a = 2;
    for(;;){ // Wrong
        if(a==1)
            break;
        else
            break;
    }

    while(1){ // Wrong
        break;
        break;
    }
    while(1); // OK
}
