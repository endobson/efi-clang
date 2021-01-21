#include "efi.h"

void test_str(char* v) {}

char c1[256];

char* const c2 = c1;
char* const c3 = c1;

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st)
{
    test_str("ca");
    test_str("bb");
    test_str("ba");
    test_str("da");
    test_str("db");
    test_str("cb");
    test_str("ab");
    test_str("aa");
    
    return 0;
}

