#include "mem.h" 
#include "os_config.h"
#include "types.h"
#include <stdio.h>


uint8_t heap_mem[HEAP_SIZE] __attribute__((aligned(2))); // PIC24 exige acesso par


void os_heap_init(void)
{
    block_info_t *blk_info = (block_info_t *)heap_mem;

    blk_info->size = HEAP_SIZE - sizeof(block_info_t); 
    blk_info->free = 1;
}

void *os_malloc(uint16_t nr_bytes)
{
    uint8_t *heap_ptr   = heap_mem; 
    uint8_t *heap_end   = heap_mem + HEAP_SIZE; 

    if (nr_bytes == 0)
        return NULL;

    nr_bytes = (nr_bytes + 1) & ~1; //arredondamento para ser par

    while (heap_ptr + sizeof(block_info_t) <= heap_end) { 
        block_info_t *blk_info = (block_info_t *)heap_ptr;

        // Verifica se o bloco é grande o suficiente para a nova alocação
        if (blk_info->free && blk_info->size >= nr_bytes) { // procurar primeiro bloco viável

            uint16_t remaining = blk_info->size - nr_bytes;

            if (remaining > sizeof(block_info_t)) {

                block_info_t *new_blk_info = (block_info_t *)(heap_ptr + sizeof(block_info_t) + nr_bytes); // cria um novo bloco com o espaço restante
                new_blk_info->size = remaining - sizeof(block_info_t); // atualiza o tamanho do novo bloco
                new_blk_info->free = 1; // marca o novo bloco como livre

                blk_info->size = nr_bytes;
            }

            blk_info->free = 0;

            return heap_ptr + sizeof(block_info_t); // endereço dos dados alocados
        }
        heap_ptr += sizeof(block_info_t) + blk_info->size; // Avança para o próximo bloco
    }

    return NULL;
}

void os_free(void *address) //address aponta pros dados do bloco
{
    if (address == NULL)
        return;

    block_info_t *blk_info = (block_info_t *)((uint8_t *)address - sizeof(block_info_t)); // achar o cabeçalho

    blk_info->free = 1;

    uint8_t *heap_ptr = heap_mem;
    uint8_t *heap_end = heap_mem + HEAP_SIZE;

    while (heap_ptr + sizeof(block_info_t) <= heap_end) { // verifica se ainda tem cabecalho de bloco para ler

        block_info_t *curr = (block_info_t *)heap_ptr;
        uint8_t *next_ptr  = heap_ptr + sizeof(block_info_t) + curr->size; // calcula o endereço do próximo bloco

        if (next_ptr + sizeof(block_info_t) <= heap_end) {

            block_info_t *next = (block_info_t *)next_ptr;

            if (curr->free && next->free) {
                curr->size += sizeof(block_info_t) + next->size; // junta os blocos livres
                continue; // não avança o ponteiro para verificar se há mais blocos livres consecutivos
            }
        }

        heap_ptr = next_ptr;
    }
}