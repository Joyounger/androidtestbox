
#include <stdio.h>

/*
 * Round up to the next highest power of 2.
 *
 * Found on http://graphics.stanford.edu/~seander/bithacks.html.
 */
unsigned int roundUpPower2(unsigned int val)
{
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
 
    return val;
}


int main()
{
    int initialSize = 8;
    int tableSize = roundUpPower2(initialSize);
    
    printf("initialSize is %d\n", initialSize);
    printf("after roundUpPower2, tableSize is %d\n", tableSize);xx
}


/* 
initialSize is 8
after roundUpPower2, tableSize is 8
*/

