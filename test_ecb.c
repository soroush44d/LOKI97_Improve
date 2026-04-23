/*
 * testloki97 - simple program to run a test triple on LOKI97
 *
 * written by Lawrie Brown / May 1998
 */

#include "loki97.h"
int self_test(char* hexkey, char* hexplain);


main(int argc, char* argv[])
{
    int st;

    if (argc < 3) {
        printf("Usage: %s <HexKey> <HexPlaintext>\n", argv[0]);
        return 1;
    }

    char* hexkey = argv[1];
    char* hexplain = argv[2];

    /* Invoke LOKI97 cipher self-test */
    printf("LOKI97 Self_test\n");

    st = self_test(hexkey, hexplain);

    printf("LOKI97 self_test returned %s (%d)\n", (st ? "OK" : "BAD"), st);
    return 0;
}
