#ifdef UTILITY_H
#define UTILITY_H

uint64_t hash_func(unsigned char *input, unsigned int pos);
int cdc(unsigned char *buff, unsigned int buff_size, bool * boundary);
void create_chunks(unsigned char ** chunk,bool* boundary,unsigned char *buff, unsigned int buff_size);

#endif