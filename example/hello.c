#include "vg.h"

#define MEM_SIZE 100 * RUNT_KILOBYTE
#define CELL_SIZE 100


int main()
{
    runt_cell *cells = malloc(sizeof(runt_cell) * CELL_SIZE);
    unsigned char *mem = malloc(MEM_SIZE);

    runt_uint word_id;

    runt_vm vm;
    runt_cell *tmp;

    runt_init(&vm);

    runt_cell_pool_set(&vm, cells, CELL_SIZE);
    runt_cell_pool_init(&vm);
    runt_memory_pool_set(&vm, mem, MEM_SIZE);

    runt_load_stdlib(&vm);


    vg_load_dict(&vm);

    runt_compile(&vm, ": cb");
    runt_compile(&vm, "255 255 255 255 vgclr");
    runt_compile(&vm, "255 0 0 255 vgfc");
    runt_compile(&vm, "40 40 40 vgcirc vgf");
    runt_compile(&vm, "0 255 0 255 vgfc");
    runt_compile(&vm, "120 40 40 vgcirc vgf");
    runt_compile(&vm, "0 0 255 255 vgfc");
    runt_compile(&vm, "200 40 40 vgcirc vgf ;");

    runt_set_state(&vm, RUNT_MODE_INTERACTIVE, RUNT_ON);
    runt_compile(&vm, "_cb vgstrt");
    int i;
    for(i = 4; i > 0; i--) {
        runt_print(&vm, "Sleeping in %d\n", i);
        sleep(1);
    }
    runt_compile(&vm, "vgstop");
    free(cells);
    free(mem);
    return 1;
}
