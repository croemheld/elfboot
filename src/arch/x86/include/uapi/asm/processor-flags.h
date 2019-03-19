#ifndef __X86_PROCESSOR_FLAGS_H__
#define __X86_PROCESSOR_FLAGS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <uapi/cr0S/const.h>

#define X86_EFLAGS_CF_BIT                          0 
#define X86_EFLAGS_CF                             _BITUL(X86_EFLAGS_CF_BIT)
#define X86_EFLAGS_FIXED_BIT                       1 
#define X86_EFLAGS_FIXED                          _BITUL(X86_EFLAGS_FIXED_BIT)
#define X86_EFLAGS_PF_BIT                          2 
#define X86_EFLAGS_PF                             _BITUL(X86_EFLAGS_PF_BIT)
#define X86_EFLAGS_AF_BIT                          4 
#define X86_EFLAGS_AF                             _BITUL(X86_EFLAGS_AF_BIT)
#define X86_EFLAGS_ZF_BIT                          6 
#define X86_EFLAGS_ZF                             _BITUL(X86_EFLAGS_ZF_BIT)
#define X86_EFLAGS_SF_BIT                          7 
#define X86_EFLAGS_SF                             _BITUL(X86_EFLAGS_SF_BIT)
#define X86_EFLAGS_TF_BIT                          8 
#define X86_EFLAGS_TF                             _BITUL(X86_EFLAGS_TF_BIT)
#define X86_EFLAGS_IF_BIT                          9 
#define X86_EFLAGS_IF                             _BITUL(X86_EFLAGS_IF_BIT)
#define X86_EFLAGS_DF_BIT                         10 
#define X86_EFLAGS_DF                             _BITUL(X86_EFLAGS_DF_BIT)
#define X86_EFLAGS_OF_BIT                         11 
#define X86_EFLAGS_OF                             _BITUL(X86_EFLAGS_OF_BIT)
#define X86_EFLAGS_IOPL_BIT                       12 
#define X86_EFLAGS_IOPL                           (_AC(3, UL) << X86_EFLAGS_IOPL_BIT)
#define X86_EFLAGS_NT_BIT                         14 
#define X86_EFLAGS_NT                             _BITUL(X86_EFLAGS_NT_BIT)
#define X86_EFLAGS_RF_BIT                         16 
#define X86_EFLAGS_RF                             _BITUL(X86_EFLAGS_RF_BIT)
#define X86_EFLAGS_VM_BIT                         17 
#define X86_EFLAGS_VM                             _BITUL(X86_EFLAGS_VM_BIT)
#define X86_EFLAGS_AC_BIT                         18 
#define X86_EFLAGS_AC                             _BITUL(X86_EFLAGS_AC_BIT)
#define X86_EFLAGS_VIF_BIT                        19 
#define X86_EFLAGS_VIF                            _BITUL(X86_EFLAGS_VIF_BIT)
#define X86_EFLAGS_VIP_BIT                        20 
#define X86_EFLAGS_VIP                            _BITUL(X86_EFLAGS_VIP_BIT)
#define X86_EFLAGS_ID_BIT                         21 
#define X86_EFLAGS_ID                             _BITUL(X86_EFLAGS_ID_BIT)

/*
 * Basic CPU control in CR0
 */

#define X86_CR0_PE_BIT                             0
#define X86_CR0_PE                                _BITUL(X86_CR0_PE_BIT)
#define X86_CR0_MP_BIT                             1
#define X86_CR0_MP                                _BITUL(X86_CR0_MP_BIT)
#define X86_CR0_EM_BIT                             2
#define X86_CR0_EM                                _BITUL(X86_CR0_EM_BIT)
#define X86_CR0_TS_BIT                             3
#define X86_CR0_TS                                _BITUL(X86_CR0_TS_BIT)
#define X86_CR0_ET_BIT                             4
#define X86_CR0_ET                                _BITUL(X86_CR0_ET_BIT)
#define X86_CR0_NE_BIT                             5
#define X86_CR0_NE                                _BITUL(X86_CR0_NE_BIT)
#define X86_CR0_WP_BIT                            16
#define X86_CR0_WP                                _BITUL(X86_CR0_WP_BIT)
#define X86_CR0_AM_BIT                            18
#define X86_CR0_AM                                _BITUL(X86_CR0_AM_BIT)
#define X86_CR0_NW_BIT                            29
#define X86_CR0_NW                                _BITUL(X86_CR0_NW_BIT)
#define X86_CR0_CD_BIT                            30
#define X86_CR0_CD                                _BITUL(X86_CR0_CD_BIT)
#define X86_CR0_PG_BIT                            31
#define X86_CR0_PG                                _BITUL(X86_CR0_PG_BIT)

/*
 * Paging options in CR3
 */

#define X86_CR3_PWT_BIT                            3
#define X86_CR3_PWT                               _BITUL(X86_CR3_PWT_BIT)
#define X86_CR3_PCD_BIT                            4
#define X86_CR3_PCD                               _BITUL(X86_CR3_PCD_BIT)

#define X86_CR3_PCID_BITS                         12
#define X86_CR3_PCID_MASK                         (_AC((1UL << X86_CR3_PCID_BITS) - 1, UL))

#define X86_CR3_PCID_NOFLUSH_BIT                  63
#define X86_CR3_PCID_NOFLUSH                      _BITULL(X86_CR3_PCID_NOFLUSH_BIT)

/*
 * Intel CPU features in CR4
 */

#define X86_CR4_VME_BIT                            0
#define X86_CR4_VME                               _BITUL(X86_CR4_VME_BIT)
#define X86_CR4_PVI_BIT                            1
#define X86_CR4_PVI                               _BITUL(X86_CR4_PVI_BIT)
#define X86_CR4_TSD_BIT                            2
#define X86_CR4_TSD                               _BITUL(X86_CR4_TSD_BIT)
#define X86_CR4_DE_BIT                             3
#define X86_CR4_DE                                _BITUL(X86_CR4_DE_BIT)
#define X86_CR4_PSE_BIT                            4
#define X86_CR4_PSE                               _BITUL(X86_CR4_PSE_BIT)
#define X86_CR4_PAE_BIT                            5
#define X86_CR4_PAE                               _BITUL(X86_CR4_PAE_BIT)
#define X86_CR4_MCE_BIT                            6
#define X86_CR4_MCE                               _BITUL(X86_CR4_MCE_BIT)
#define X86_CR4_PGE_BIT                            7
#define X86_CR4_PGE                               _BITUL(X86_CR4_PGE_BIT)
#define X86_CR4_PCE_BIT                            8
#define X86_CR4_PCE                               _BITUL(X86_CR4_PCE_BIT)
#define X86_CR4_OSFXSR_BIT                         9
#define X86_CR4_OSFXSR                            _BITUL(X86_CR4_OSFXSR_BIT)
#define X86_CR4_OSXMMEXCPT_BIT                    10
#define X86_CR4_OSXMMEXCPT                        _BITUL(X86_CR4_OSXMMEXCPT_BIT)
#define X86_CR4_UMIP_BIT                          11
#define X86_CR4_UMIP                              _BITUL(X86_CR4_UMIP_BIT)
#define X86_CR4_LA57_BIT                          12
#define X86_CR4_LA57                              _BITUL(X86_CR4_LA57_BIT)
#define X86_CR4_VMXE_BIT                          13
#define X86_CR4_VMXE                              _BITUL(X86_CR4_VMXE_BIT)
#define X86_CR4_SMXE_BIT                          14
#define X86_CR4_SMXE                              _BITUL(X86_CR4_SMXE_BIT)
#define X86_CR4_FSGSBASE_BIT                      16
#define X86_CR4_FSGSBASE                          _BITUL(X86_CR4_FSGSBASE_BIT)
#define X86_CR4_PCIDE_BIT                         17
#define X86_CR4_PCIDE                             _BITUL(X86_CR4_PCIDE_BIT)
#define X86_CR4_OSXSAVE_BIT                       18
#define X86_CR4_OSXSAVE                           _BITUL(X86_CR4_OSXSAVE_BIT)
#define X86_CR4_SMEP_BIT                          20
#define X86_CR4_SMEP                              _BITUL(X86_CR4_SMEP_BIT)
#define X86_CR4_SMAP_BIT                          21
#define X86_CR4_SMAP                              _BITUL(X86_CR4_SMAP_BIT)
#define X86_CR4_PKE_BIT                           22
#define X86_CR4_PKE                               _BITUL(X86_CR4_PKE_BIT)

/*
 * EFER
 */

#define X86_EFER_SCE_BIT                           0
#define X86_EFER_SCE                              _BITULL(X86_EFER_SCE_BIT)
#define X86_EFER_LME_BIT                           8
#define X86_EFER_LME                              _BITULL(X86_EFER_LME_BIT)
#define X86_EFER_LMA_BIT                          10
#define X86_EFER_LMA                              _BITULL(X86_EFER_LMA_BIT)
#define X86_EFER_NXE_BIT                          11
#define X86_EFER_NXE                              _BITULL(X86_EFER_NXE_BIT)
#define X86_EFER_SVME_BIT                         12
#define X86_EFER_SVME                             _BITULL(X86_EFER_SVME_BIT)
#define X86_EFER_LMSLE_BIT                        13
#define X86_EFER_LMSLE                            _BITULL(X86_EFER_LMSLE_BIT)
#define X86_EFER_FFXSR_BIT                        14
#define X86_EFER_FFXSR                            _BITULL(X86_EFER_FFXSR_BIT)
#define X86_EFER_TCE_BIT                          15
#define X86_EFER_TCE                              _BITULL(X86_EFER_TCE_BIT)

#endif /* __X86_PROCESSOR_FLAGS_H__ */