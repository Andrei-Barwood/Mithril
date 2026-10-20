#include <flint/flint.h>
#include <flint/fmpz.h>
#include <stdio.h>

int fmpz_add_modern(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    fmpz_add(result, a, b);
    return 0;
}

int main() {
    fmpz_t a, b, sum;
    
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(sum);
    
    fmpz_set_str(a, "123456789012345678901234567890", 10);
    fmpz_set_str(b, "987654321098765432109876543210", 10);
    
    int result = fmpz_add_modern(sum, a, b);
    
    printf("Addition test result: ");
    fmpz_print(sum);
    printf("\nStatus: %d\n", result);
    printf("Test completed successfully!\n");
    
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(sum);
    
    return 0;
}
