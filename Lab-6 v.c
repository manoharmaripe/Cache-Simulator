#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define LENGTH 6

int replacementcheck = 1;

typedef struct node
{
    int tag;
    int replacepolicy;
} Node;

Node *createnode(int tag)
{
    Node *node;
    node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        printf("Memory allocation failed\n");
        exit(1);
    }
    node->tag = tag;
    return node;
}

char *hexDigitToBinary(char hex_digit)
{
    switch (hex_digit)
    {
    case '0':
        return "0000";
    case '1':
        return "0001";
    case '2':
        return "0010";
    case '3':
        return "0011";
    case '4':
        return "0100";
    case '5':
        return "0101";
    case '6':
        return "0110";
    case '7':
        return "0111";
    case '8':
        return "1000";
    case '9':
        return "1001";
    case 'A':
    case 'a':
        return "1010";
    case 'B':
    case 'b':
        return "1011";
    case 'C':
    case 'c':
        return "1100";
    case 'D':
    case 'd':
        return "1101";
    case 'E':
    case 'e':
        return "1110";
    case 'F':
    case 'f':
        return "1111";
    default:
        return NULL;
    }
}

char *hexTo32BitBinary(char *hex_string)
{
    size_t hex_len = strlen(hex_string);
    size_t bin_len = hex_len * 4;
    size_t leading_zeros = 32 - bin_len;
    char *binary = (char *)malloc(33);

    if (binary == NULL)
    {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    binary[32] = '\0';
    memset(binary, '0', leading_zeros);

    for (size_t i = 0; i < hex_len; ++i)
    {
        char *bin = hexDigitToBinary(hex_string[i]);
        if (bin == NULL)
        {
            printf("Invalid hexadecimal digit encountered: %c\n", hex_string[i]);
            exit(EXIT_FAILURE);
        }
        strcat(binary + leading_zeros, bin);
    }

    return binary;
}

char *hexToBinary(char *hex)
{
    int hexLength = strlen(hex);
    int binaryLength = hexLength * 4;
    char *binary = (char *)malloc(binaryLength + 1);

    if (binary == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    binary[binaryLength] = '\0';
    for (int i = 0, label = 0; i < hexLength; i++)
    {
        char *digitBinary = hexDigitToBinary(hex[i]);
        if (digitBinary)
        {
            strcat(binary + label, digitBinary);
            label += 4;
        }
        else
        {
            free(binary);
            return NULL;
        }
    }
    return binary;
}

int binaryToDecimal(char *binary)
{
    int decimalNumber = 0;
    for (int i = 0; i < strlen(binary); i++)
    {
        if (binary[i] == '1')
        {
            decimalNumber = (decimalNumber << 1) | 1;
        }
        else if (binary[i] == '0')
        {
            decimalNumber = (decimalNumber << 1);
        }
        else
        {
            printf("Invalid character in the binary string: %c\n", binary[i]);
            return -1;
        }
    }
    return decimalNumber;
}

char *breakCharArray(char *input, int startIndex, int endIndex)
{
    int length = strlen(input);
    if (startIndex < 0 || startIndex >= length || endIndex < 0 || endIndex >= length || startIndex > endIndex)
    {
        return NULL;
    }
    int subLength = endIndex - startIndex + 1;
    char *result = (char *)malloc(subLength + 1);

    if (result == NULL)
    {
        return NULL;
    }
    strncpy(result, input + startIndex, subLength);
    result[subLength] = '\0';
    return result;
}

int extracttag(char *binary, int tagbits)
{
    char *tagbinary = breakCharArray(binary, 0, tagbits - 1);
    int tag = binaryToDecimal(tagbinary);
    return tag;
}

int extractsetindex(char *binary, int setindexbits, int tagbits)
{
    char *setindexbinary = breakCharArray(binary, tagbits, tagbits + setindexbits - 1);
    int setindex = binaryToDecimal(setindexbinary);
    return setindex;
}

int main()
{
    int numofhits = 0;
    int numofmisses = 0;
    // printf("Give the name of file: ");
    char filename[256];
    // scanf("%[^\n]s", filename);
    FILE *file;
    char line[100];
    file = fopen("cache.config", "r");
    if (file == NULL)
    {
        printf("Unable to open the file.\n");
        return 1;
    }

    char *config[5];
    for (int i = 0; i < 5; i++)
    {
        config[i] = NULL;
    }

    int j = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        // config[j] = malloc((strlen(line) + 1) * sizeof(char));
        // strcpy(config[j++], line);
        config[j++] = strdup(line);
    }

    int cachesize = atoi(config[0]);
    int blocksize = atoi(config[1]);
    int associativity = atoi(config[2]);
    char policy[7];
    char wpolicy[3];
    strcpy(policy, config[3]);
    strcpy(wpolicy, config[4]);
    int numofsets = cachesize / (blocksize * associativity);
    int setindexbits = log2(numofsets);
    int offsetbits = log2(blocksize);
    int tagbits = 32 - setindexbits - offsetbits;
    // printf("num of sets : %d\nsetindexbits: %d\noffsetbits: %d\ntagbits: %d\n\n", numofsets, setindexbits, offsetbits, tagbits);

    for (int i = 0; i < 5; i++)
    {
        free(config[i]);
    }
    fclose(file);

    file = fopen("trace5.txt", "r");
    if (file == NULL)
    {
        printf("Unable to open the file.\n");
        return 1;
    }

    char *address[2];
    for (int i = 0; i < 5; i++)
    {
        address[i] = NULL;
    }

    Node cache[numofsets][associativity];
    for (int l = 0; l < numofsets; l++)
    {
        for (int k = 0; k < associativity; k++)
        {
            cache[l][k] = *createnode(-1);
        }
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        char *token;
        int i = 0;
        token = strtok(line, ": ");
        while (token != NULL)
        {
            address[i++] = strdup(token);
            token = strtok(NULL, ": ");
        }

        printf("Address: %s, ", address[1]);
        address[1] = address[1] + 2;
        char *binaryaddress = hexTo32BitBinary(address[1]);
        // printf("Binary: %s \n", binaryaddress);
        int setindex = extractsetindex(binaryaddress, setindexbits, tagbits);
        printf("Set: 0x%x, ", setindex);
        int tag = extracttag(binaryaddress, tagbits);
        // printf("0x%x", tag);
        if (strcmp(address[0], "R") == 0 || ((strcmp(wpolicy, "WB") == 0 || strcmp(wpolicy, "WA") == 0) && strcmp(address[0], "W") == 0))
        {
            if (strcmp(policy, "LRU") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        cache[setindex][x].replacepolicy = replacementcheck;
                        replacementcheck++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS NOT FULL, ");
                        numofmisses++;
                        cache[setindex][indextonew].tag = tag;
                        cache[setindex][indextonew].replacepolicy = replacementcheck;
                        replacementcheck++;
                    }
                    else if (indextonew == -1)
                    {
                        int minvalue = cache[setindex][0].replacepolicy;
                        int replaceindex = 0;
                        for (int y = 1; y < associativity; y++)
                        {
                            if (minvalue > cache[setindex][y].replacepolicy)
                            {
                                minvalue = cache[setindex][y].replacepolicy;
                                replaceindex = y;
                            }
                        }
                        // printf("%d  %d --> ", setindex, replaceindex);
                        printf("MISS, ");
                        numofmisses++;
                        cache[setindex][replaceindex].tag = tag;
                        cache[setindex][replaceindex].replacepolicy = replacementcheck;
                        replacementcheck++;
                    }
                }
            }
            else if (strcmp(policy, "FIFO") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS NOT FULL, ");
                        numofmisses++;
                        cache[setindex][indextonew].tag = tag;
                        cache[setindex][indextonew].replacepolicy = replacementcheck;
                        replacementcheck++;
                    }
                    else if (indextonew == -1)
                    {
                        int minvalue = cache[setindex][0].replacepolicy;
                        int replaceindex = 0;
                        for (int y = 1; y < associativity; y++)
                        {
                            if (minvalue > cache[setindex][y].replacepolicy)
                            {
                                minvalue = cache[setindex][y].replacepolicy;
                                replaceindex = y;
                            }
                        }
                        // printf("%d  %d --> ", setindex, replaceindex);
                        printf("MISS FULL, ");
                        numofmisses++;
                        cache[setindex][replaceindex].tag = tag;
                        cache[setindex][replaceindex].replacepolicy = replacementcheck;
                        replacementcheck++;
                    }
                }
            }
            else if (strcmp(policy, "RANDOM") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS NOT FULLclea, ");
                        numofmisses++;
                        cache[setindex][indextonew].tag = tag;
                    }
                    else if (indextonew == -1)
                    {
                        int range = associativity - 0 + 1;
                        srand((unsigned int)time(NULL));
                        int replaceindex = (rand() % range) + 0;
                        // printf("%d  %d-->", setindex, replaceindex);
                        printf("MISS, ");
                        numofmisses++;
                        cache[setindex][replaceindex].tag = tag;
                    }
                }
            }
        }
        else if ((strcmp(wpolicy, "WT") == 0 || strcmp(wpolicy, "NWA") == 0) && strcmp(address[0], "W") == 0)
        {
            if (strcmp(policy, "LRU") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        cache[setindex][x].replacepolicy = replacementcheck;
                        replacementcheck++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS, ");
                        numofmisses++;
                    }
                    else if (indextonew == -1)
                    {
                        int minvalue = cache[setindex][0].replacepolicy;
                        int replaceindex = 0;
                        for (int y = 1; y < associativity; y++)
                        {
                            if (minvalue > cache[setindex][y].replacepolicy)
                            {
                                minvalue = cache[setindex][y].replacepolicy;
                                replaceindex = y;
                            }
                        }
                        // printf("%d  %d --> ", setindex, replaceindex);
                        printf("MISS, ");
                        numofmisses++;
                    }
                }
            }
            else if (strcmp(policy, "FIFO") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS NOT FULL, ");
                        numofmisses++;
                    }
                    else if (indextonew == -1)
                    {
                        int minvalue = cache[setindex][0].replacepolicy;
                        int replaceindex = 0;
                        for (int y = 1; y < associativity; y++)
                        {
                            if (minvalue > cache[setindex][y].replacepolicy)
                            {
                                minvalue = cache[setindex][y].replacepolicy;
                                replaceindex = y;
                            }
                        }
                        // printf("%d  %d --> ", setindex, replaceindex);
                        printf("MISS FULL, ");
                        numofmisses++;
                    }
                }
            }
            else if (strcmp(policy, "RANDOM") == 0)
            {
                int x = 0;
                int found = 0;
                int indextonew = -1;
                for (x = 0; x < associativity; x++)
                {
                    if (cache[setindex][x].tag == tag)
                    {
                        found = 1;
                        printf("HIT, ");
                        numofhits++;
                        break;
                    }
                    if (cache[setindex][x].tag == -1)
                    {
                        indextonew = x;
                        break;
                    }
                }
                if (found == 0)
                {
                    if (indextonew != -1)
                    {
                        // printf("%d  %d --> ", setindex, indextonew);
                        printf("MISS, ");
                        numofmisses++;
                    }
                    else if (indextonew == -1)
                    {
                        int range = associativity - 0 + 1;
                        srand((unsigned int)time(NULL));
                        int replaceindex = (rand() % range) + 0;
                        // printf("%d  %d-->", setindex, replaceindex);
                        printf("MISS, ");
                        numofmisses++;
                    }
                }
            }
        }
        printf("Tag: 0x%x\n", tag);
    }
    printf("Number of Hits : %d\tNumber of Misses : %d\n", numofhits, numofmisses);
    fclose(file);
}