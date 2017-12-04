int main(){
    int i;
    for(;;); // Error
    for(i=0;;++i); // Error
    for(i=0;i<5;i++);	// OK
}
