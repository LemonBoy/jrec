#include <stdio.h>

typedef unsigned char u8;

int valid_marker_id (u8 id)
{
    return (id == 0xE0 || id == 0xE1 || id == 0xE2 || id == 0xE3 ||
            id == 0xE4 || id == 0xE5 || id == 0xE6 || id == 0xE7 ||
            id == 0xE8 || id == 0xE9 || id == 0xEA || id == 0xEB ||
            id == 0xEC || id == 0xED || id == 0xEE || id == 0xEF ||
            id == 0xFE || id == 0xC4 || id == 0xDB || id == 0xDD ||
            id == 0xD9 || id == 0xC0 || id == 0xD8 || id == 0xDA);
}

int main (int argc, char **argv)
{
    FILE *f;
    FILE *rf;
    int data_len;
    
    u8 *data, *data_end, *head;
    
    int     recov_f;
    char    recov_n[0xFF];
    
    printf("jrec 0.1\nThe Lemon Man (C) 2011\n");
    if (argc != 2) {
        printf("Usage:\n\t%s dumpfile\n", argv[0]);
        return 1;
    }
    printf("Working on dump : %s\n", argv[1]);
    f = fopen(argv[1], "rb");
    if (!f) {
        printf("Cannot open the file\n");
        return 0;
    }
    fseek(f, 0, SEEK_END);
    data_len = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(data_len);
    if (!data) {
        printf("Cannot allocate the memory\n");
        return 0;
    }
    fread(data, 1, data_len, f);
    fclose(f);
    printf("Dump size : %i bytes\n", data_len);
    
    data_end = data + data_len;
    recov_f = 0;
    rf = NULL;
    
    head = data;
    
    do {
        if (data[0] == 0xFF && valid_marker_id(data[1])) {
            if (data[1] == 0xD8 && data[2] == 0xFF) {
                sprintf(recov_n, "%08d.jpg", ++recov_f);
                printf("Found SOI mark...saving as %s\n", recov_n);
                rf = fopen(recov_n, "w+b");
                if (!rf) {
                    printf("Cannot create the file %s...\n", recov_n);
                    return 0;
                }
                fwrite(data, 2, 1, rf);
                data += 2;
            }
            else if (rf && data[1] == 0xD9) {
                printf("Found EOI mark\n");
                fwrite(data, 2, 1, rf);
                data += 2;
                fclose(rf);
                rf = NULL;  
            }
            else if (rf) {
                int mark_len;
                u8 mark_type;
                mark_type = data[1];
                mark_len = (data[2] << 8) | data[3];
                printf("Saving 0x%02x mark\n", mark_type);
                printf("Mark len %i\n", mark_len);
                fwrite(data, 2, 1, rf);
                data += 2;
                fwrite(data, mark_len, 1, rf);
                data += mark_len;

                if (mark_type == 0xDA) {
                    while (!(data[0] == 0xFF && data[1] == 0xD9)) {
                        fwrite(data++, 1, 1, rf);
                    }
                }
            }
            else {
                data++;
            }
        } else {
            data++;
        }

skip:
        data++;
    } while (data < data_end);
    
    free(head);
    
    return 1;
}
