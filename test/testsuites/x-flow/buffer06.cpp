#include <stdlib.h>

// More complex object

void f() {
    struct Person {
        const char* name;
        int age;
    };
    Person* people = (Person*) malloc(9 * sizeof(Person));
    for (int i = 0; i < 12; i++) {
        people[i].name = "John";
        people[i].age = 23;
    }
}

int main() {
    f();
    return 0;
}
