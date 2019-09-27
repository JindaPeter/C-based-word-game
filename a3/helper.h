#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
void error_message();
struct rec* sort(char *f, int loc, int num_of_process);
int check_end(struct rec *r, int len);
int merge(struct rec *r, int len, FILE* f);

#endif /* _HELPER_H */
