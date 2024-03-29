#include <stdio.h>

void foo(__attribute__((unused)) int a, [[maybe_unused]] int b) { printf("Hello World!"); }

int main(void) {
  foo(1, 2);
  return 0;
}
