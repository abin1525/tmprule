class Foo{


};

typedef Foo Foo2;
int main(){
 

 const Foo2* x = new Foo();

 Foo* y;
 y = const_cast<Foo*>(x);

};
