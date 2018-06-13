#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define MAX 1024
#define LOG2(valor)  (unsigned)(log(valor)/log(2))
static char memoria_principal[MAX] = "";

struct linha
{
	unsigned int tag;
	bool bit_validade;
	void* bloco;
};
typedef struct linha Linha;

struct cache
{
	Linha* linhas;
	int cache_hit;
	int cache_miss;
	unsigned int size_bloco;
	unsigned int num_linhas; 
};
typedef struct cache Cache;


Cache* cria_cache(int num_linhas, int size_bloco){
	
	int i; 
	Cache* cache = (Cache*) malloc(sizeof(Cache));
	cache->num_linhas = num_linhas;
	cache->size_bloco = size_bloco;

	cache->linhas = (Linha*)malloc(sizeof(Linha)*num_linhas);
	for(i = 0; i< num_linhas; i++){
		cache->linhas[i].bloco = malloc(sizeof(char)*size_bloco);
		cache->linhas[i].tag = 0;
		cache->linhas[i].bit_validade = false;
	}

	return cache;
}

void write_cache(unsigned int endereco, unsigned int tag, unsigned int indice_bloco, unsigned int indice_linha, char *memoria_principal, Cache *cache){
	int i;
	for(i = 0; i < cache->size_bloco; i++){
		if((i + indice_bloco) < cache->size_bloco && (i + endereco) < MAX){
			((char*)cache->linhas[indice_linha].bloco)[i + indice_bloco] = memoria_principal[i+endereco];
		}else{
			break;
		}
	}
	cache->linhas[indice_linha].tag = tag;
	cache->linhas[indice_bloco].bit_validade = true;

}

char read_cache(unsigned int endereco, Cache *cache){
	unsigned int numBit_bloco = LOG2(cache->size_bloco);
	unsigned int numBit_Linha = LOG2(cache->num_linhas);
	unsigned int numBit_tag   = 32 - (numBit_bloco + numBit_Linha);
	 	
	unsigned int indice_bloco = (cache->size_bloco - 1) & endereco; //numero do bloco que eu vou acessar em alguma linha.
	unsigned int indice_linha = (((cache->num_linhas - 1) << numBit_bloco) & endereco) >> numBit_bloco;
	unsigned int tag          = (((((unsigned int)(pow(2,numBit_tag)- 1)) << (numBit_Linha + numBit_bloco)) & endereco)) >> (numBit_Linha + numBit_bloco);

	/*printf("%d\n", indice_bloco);
	printf("%d\n", indice_linha);
	printf("%d\n", tag);*/

	if(cache->linhas[indice_linha].tag == tag && cache->linhas[indice_linha].bit_validade)
	{
		cache->cache_hit ++;

	}else{
		cache->cache_miss ++;
		write_cache(endereco, tag, indice_bloco, indice_linha, memoria_principal, cache);
		
	}

	return ((char*)cache->linhas[indice_linha].bloco)[indice_bloco];
}

int main(){

	strcat(memoria_principal,"Arquitetura de computadores II");

	Cache *cache = cria_cache(4, 32);
	for(int i = 0; i < strlen(memoria_principal); i++){
		read_cache(i, cache);	
	}

	for(int i = 0; i < strlen(memoria_principal); i++){
		read_cache(i, cache);	
	}
	printf("\nCache miss:%d\nCache hit:%d\n", cache->cache_miss, cache->cache_hit);
	return 0;
}
