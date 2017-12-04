typedef int*** PPINT; // Violation
void foo1() {// Violation
        long** c;
        int*** a;
        PPINT b;
}
int*** foo2(); // Violation
void foo3(const char*** p1, // Violation
                const char** p2[],
                const char* p3[2][2]
                );

int main() {
    char* p; // OK
}
