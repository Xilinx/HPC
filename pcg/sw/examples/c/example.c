
#include "pcg.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
    if (argc < 3) {
        printf("Usage: %s <deviceId> <xclbinPath>\n", argv[0]);
        return 1;
    }
    
    int deviceId = atoi(argv[1]);
    const char *xclbinPath = argv[2];
    
    void *pHandle = create_JPCG_handle(deviceId, xclbinPath);
    printf("Oha-konban-chiwa World!\n");
    destroy_JPCG_handle(pHandle);
    return 0;
}