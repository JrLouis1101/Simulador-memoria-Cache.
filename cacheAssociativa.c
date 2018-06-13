#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define MAX 1024
#define LOG2(valor)  (unsigned)(log(valor)/log(2))

static char memoria_principal[MAX] = "";

struct caminho{
	bool bit_validade;
	unsigned int tag;
	unsigned int sujo;
	void* bloco;
};
typedef struct caminho Caminho;

struct conjunto{
	Caminho *caminho;
};
typedef struct conjunto Conjunto;

struct cache{
	Conjunto *conjuntos;
	unsigned int size_bloco;
	unsigned int num_linhas; 
	unsigned int num_conjuntos;
	unsigned int qtd_caminho;
	int cache_miss;
	int cache_hit;
};
typedef struct cache Cache;

Cache* cria_cache(int num_linhas, int size_bloco, int qtd_caminho){ 
	int i, j;
	Cache *cache = (Cache*)malloc(sizeof(Cache));
	cache->num_linhas    = num_linhas;
	cache->size_bloco    = size_bloco;
	cache->qtd_caminho   = qtd_caminho;
	cache->num_conjuntos = num_linhas/qtd_caminho;
	cache->conjuntos     = (Conjunto*)malloc(sizeof(Conjunto) * cache->num_conjuntos);
	cache->cache_miss    = 0;
	cache->cache_hit     = 0;

	for(i = 0; i < cache->num_conjuntos; i++){
		cache->conjuntos[i].caminho = (Caminho*)malloc(sizeof(Caminho)*qtd_caminho);
		for(j = 0; j < qtd_caminho; j++){
			//cache->conjuntos[i].caminho[j].bloco        = malloc(sizeof(char) * size_bloco);
			cache->conjuntos[i].caminho[j].bloco         = calloc(cache->size_bloco, sizeof(char));
			cache->conjuntos[i].caminho[j].bit_validade  = false;
			cache->conjuntos[i].caminho[j].tag           = 0;
			cache->conjuntos[i].caminho[j].sujo          = 0;
		}
	}

	return cache;
}

void imprime_cache(Cache *cache){
	int i = 0,
		j = 0,
		k = 0;

	for(i = 0; i < cache->num_conjuntos; i++)
	{
		printf("--------------------------- Conjunto %d ---------------------------\n", i);
		for(j = 0; j < cache->qtd_caminho; j++)
		{
			printf("caminho %d:\n", j);
			for(k = 0; k < cache->size_bloco; k++)
			{
				printf("|%c| ", ((char*)cache->conjuntos[i].caminho[j].bloco)[k]);
			}
			printf("\n");
		}
		printf("------------------------------------------------------------------\n");
	}

}

int conjuto_cheio(Cache* cache, unsigned int indice_conjunto){
	int i;
	for(i = 0; i < cache->qtd_caminho; i++){
		if(cache->conjuntos[indice_conjunto].caminho[i].bit_validade == false){
			return i;
		}
	}
	return -1;
}

int busca_conjunto(Cache *cache, int tag, unsigned int indice_conjunto){
	int i;
	for(i = 0; i < cache->qtd_caminho; i++){
		if(cache->conjuntos[indice_conjunto].caminho[i].tag == tag && cache->conjuntos[indice_conjunto].caminho[i].bit_validade){
			return i;
		}
	}


	return -1;
}

void escreve_cache(Cache* cache, unsigned int indice_caminho, unsigned int indice_conjunto, unsigned int tag, unsigned int indice_bloco, unsigned int endereco){
	int i;
	for(i = 0; i < cache->size_bloco; i++){
		if((i+indice_bloco) > cache->size_bloco || (i+endereco) > MAX){
			break;
		}
		((char*)cache->conjuntos[indice_conjunto].caminho[indice_caminho].bloco)[indice_bloco+i] = memoria_principal[endereco+i];
	}
	cache->conjuntos[indice_conjunto].caminho[indice_caminho].tag = tag;
	cache->conjuntos[indice_conjunto].caminho[indice_caminho].bit_validade = true;
	cache->conjuntos[indice_conjunto].caminho[indice_caminho].sujo ++;
}

int FIFO(Cache* cache, unsigned int indice_conjunto){
	int i, sujo_aux = 0;
	for(i = 0; i < cache->qtd_caminho; i++){
		if(cache->conjuntos[indice_conjunto].caminho[i].sujo < sujo_aux > cache->conjuntos[indice_conjunto].caminho[sujo_aux].sujo ) {
			sujo_aux = i;
		}
	}
	return sujo_aux;
}

char le_cache(unsigned int endereco, Cache *cache){
	unsigned int numBit_bloco = LOG2(cache->size_bloco);
	unsigned int numBit_conjunto = LOG2(cache->num_conjuntos);
	unsigned int numBit_tag   = 32 - (numBit_bloco + numBit_conjunto);
	unsigned int indice_caminho = 0;  // primeiro caminho com NULL;
	 	
	unsigned int indice_bloco = (cache->size_bloco - 1) & endereco; //numero do bloco que eu vou acessar em alguma linha.

	unsigned int indice_conjunto = (((cache->num_conjuntos - 1) << numBit_bloco) & endereco) >> numBit_bloco;

	unsigned int tag          = (((((unsigned int)(pow(2,numBit_tag)- 1)) << (numBit_bloco + numBit_conjunto)) & endereco)) >> (numBit_bloco + numBit_conjunto);

	/*printf("%d\n", indice_bloco);
	printf("%d\n", indice_linha);
	printf("%d\n", tag);*/
	indice_caminho = busca_conjunto(cache, tag, indice_conjunto);
	//printf("%d\n", indice_caminho);
	if(indice_caminho != -1){
		cache->cache_hit++;
	}else{
		cache->cache_miss ++;
		indice_caminho = conjuto_cheio(cache, indice_conjunto); // retorna primeiro caminho livre

		if(indice_caminho != -1){

			escreve_cache(cache, indice_caminho, indice_conjunto, tag, indice_bloco, endereco);
		}else{
			indice_caminho = FIFO(cache, indice_conjunto);
			escreve_cache(cache, indice_caminho, indice_conjunto, tag, indice_bloco, endereco);
		}


	}

	return ((char*)cache->conjuntos[indice_conjunto].caminho[indice_caminho].bloco)[indice_bloco];
}

int main()
{
	Cache *cache = cria_cache(16, 4, 4); //linha, tam bloco, qtd caminho
	imprime_cache(cache);


	strcat(memoria_principal,"Arquitetura de computadores II haskjsadhasdhdasjdhasjhasdhjsdajhasdasdjhdasjkhashdvsnks");

	for(int i = 0; i < strlen(memoria_principal); i++){
		printf("%c", le_cache(i, cache));
	}

	printf("\n\n\nESCREVENDO NA CACHE......\n\n\n");

	printf("\nCache miss:%d\nCache hit:%d\n", cache->cache_miss, cache->cache_hit);
	imprime_cache(cache);
	
	return 0;
}