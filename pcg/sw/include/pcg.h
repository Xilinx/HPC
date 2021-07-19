/**
 * NOT FOR CHECK-IN!
 */

#ifndef PCG_H
#define PCG_H

/**
 * Define this macro to make functions in pcg_loader.cpp inline instead of extern.  You would use this macro
 * when including pcg_loader.cpp in a header file, as opposed to linking with libXilinxCosineSim_loader.a.
 */
#ifdef XILINX_PCG_INLINE
#define XILINX_PCG_LINKAGE_DECL inline
#else
#define XILINX_PCG_LINKAGE_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JPCG_Mode {
    JPCG_MODE_FULL = 0,
    JPCG_MODE_KEEP_NZ_LAYOUT = 1,
    JPCG_MODE_DO_MAGIC = 99
} JPCG_Mode;


XILINX_PCG_LINKAGE_DECL
void *create_JPCG_handle(int deviceId, const char *xclbinPath);

XILINX_PCG_LINKAGE_DECL
void destroy_JPCG_handle(void *handle);

XILINX_PCG_LINKAGE_DECL
void JPCG_coo(void *handle, JPCG_Mode mode, int dummy);


#ifdef __cplusplus
}
#endif

#endif /* PCG_H */

