/**
 * Some extra logging functions via DebugMsg.
 * It traces user-defined function calls, such as TryPostEvent, state objects, EDMAC and whatever else you throw at it.
 * 
 * Includes a generic logging function => simply plug the addresses into logged_functions[] and should be ready to go.
 * 
 * TODO:
 * - move to module? (I also need it as core functionality, for researching the startup process)
 * - import the patched addresses from stubs
 */

#include "dryos.h"
#include "gdb.h"
#include "patch.h"
#include "state-object.h"
#include "asm.h"
#include "cache_hacks.h"
#include "console.h"
#include "edmac.h"

static void generic_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void state_transition_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void LockEngineResources_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void UnLockEngineResources_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void fps_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void LockEngineResources_log_r4(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void UnLockEngineResources_log_r7(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void engio_write_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void mpu_send_log(uint32_t* regs, uint32_t* stack, uint32_t pc);
static void mpu_recv_log(uint32_t* regs, uint32_t* stack, uint32_t pc);

struct logged_func
{
    uint32_t addr;                              /* Logged address (usually at the start of the function; will be passed to gdb_add_watchpoint) */
    char* name;                                 /* Breakpoint (function) name (optional, can be NULL) */
    int num_args;                               /* How many arguments does your function have? (will try to print them) */
    patch_hook_function_cbr log_func;           /* if generic_log is not enough, you may use a custom logging function */
};

#ifdef CONFIG_DEBUG_INTERCEPT_STARTUP
static struct logged_func logged_functions[] = {
    #ifdef CONFIG_5D2
    { 0xff9b3cb4, "register_interrupt", 4 },
    //~ { 0xFF87284C, "dma_memcpy", 3 },            // conflicts with mpu_recv
    { 0xff9b989c, "TryPostEvent", 5},
    { 0xff9b8f24, "TryPostStageEvent", 5 },

    /* register-level SIO3/MREQ communication */
    { 0xFF99F318, "MREQ_ISR", 0 },
    { 0xFF99F330, "MREQ_C020302C", 1 },
    { 0xFF99F348, "MREQ_C022009C", 1 },
    { 0xFF99F36C, "MREQ_C0820304", 1 },
    { 0xFF99F3A8, "SIO3_ISR", 0 },
    { 0xFF99F3E4, "SIO3_C022009C", 1 },
    { 0xFF99F404, "SIO3_C0820304", 1 },
    { 0xFF99F658, "SIO3_recv", 1 },
    { 0xFF99F244, "MREQ_SIO3_get_data_to_send", 2 },
    { 0xFF99F418, "SIO3_get_data_to_send_ret", 1 },
    { 0xFF99F424, "SIO3_data_to_send", 1 },
    { 0xFF99F380, "MREQ_get_data_to_send_ret", 1 },
    { 0xFF99F38C, "MREQ_data_to_send", 1 },

    /* message-level SIO3/MREQ communication */
    { 0xFF99F518, "mpu_send", 2, mpu_send_log },
    { 0xFF861840, "mpu_recv", 1, mpu_recv_log },
    #endif

    #ifdef CONFIG_5D3   /* 1.2.3 */
    { 0x17d54,    "TryPostEvent", 5 },
    { 0xFF2E8648, "mpu_send", 2, mpu_send_log }, // here it's OK via GDB hooks
    { 0xFF1226F0, "mpu_recv", 1, mpu_recv_log},  // fixme: first call may be missed, figure out why (seems to be OK at cold boot)
    #endif
	
	#ifdef CONFIG_6D	/* 1.1.3 */
    { 0x39F04,    "TryPostEvent", 5 },
	
	/* message-level SIO3/MREQ communication */
    { 0xFF3A8648, "mpu_send", 2, mpu_send_log }, 
    { 0xFF1E26F0, "mpu_recv", 1, mpu_recv_log}, 
    #endif	
};
#else
static struct logged_func logged_functions[] = {
#ifdef CONFIG_5D2
    { 0xff9b9198, "StateTransition", 4 , state_transition_log },
    { 0xff9b989c, "TryPostEvent", 5 },
    //~ { 0xff9b8f24, "TryPostStageEvent", 5 },                 // conflicts with SetHPTimerAfter

    { 0xFF9A462C, "ConnectReadEDmac", 2 },
    { 0xFF9A4604, "ConnectWriteEDmac", 2 },
    { 0xFF9A4798, "RegisterEDmacCompleteCBR", 3 },
    //~ { 0xFF9A45E8, "SetEDmac", 4 },                          // conflicts with RegisterHead1InterruptHandler
    { 0xFF9A464C, "StartEDmac", 2 },
    
    { 0xff9b3cb4, "register_interrupt", 4 },
    //~ { 0xffb277c8, "register_obinteg_cbr", 2 },              // conflicts with UnLockEngineResources
    { 0xffaf6930, "set_digital_gain_and_related", 3 },
    //~ { 0xffaf68a4, "set_saturate_offset", 1 },               // conflicts with TryPostEvent
    { 0xffaf686c, "set_saturate_offset_2", 1 },
    { 0xff987200, "set_fps_maybe", 1, fps_log },
    { 0xffa38114, "set_fps_maybe_2", 1 },
    { 0xffa366c8, "AJ_FixedPoint_aglrw_related", 4 },
    { 0xffa37de4, "RegisterHead1InterruptHandler", 3 },
    //~ { 0xff986d9c, "RegisterHead2InterruptHandler", 3 },
    //~ { 0xff986ebc, "RegisterHead3InterruptHandler", 3 },     // conflicts with AJ_FixedPoint_aglrw_related
    //~ { 0xff986f74, "RegisterHead4InterruptHandler", 3 },
    { 0xff86c720, "SetHPTimerAfter", 4 },
    { 0xff9a86e0, "LockEngineResources", 1, LockEngineResources_log },
    { 0xff9a87d0, "UnLockEngineResources", 1, UnLockEngineResources_log},
#endif

#ifdef CONFIG_550D
    { 0xff1d84f4, "StateTransition", 4 , state_transition_log },
    { 0xff1d8c30, "TryPostEvent", 5 },
    { 0xff1d82b8, "TryPostStageEvent", 5 },

    { 0xff1c00c0, "ConnectReadEDmac", 2 },
    { 0xff1bfffc, "ConnectWriteEDmac", 2 },
    { 0xFF1C0418, "RegisterEDmacCompleteCBR", 3 },
    { 0xff1bff44, "SetEDmac", 4 },
    { 0xff1c024c, "StartEDmac", 2 },
    
    { 0xff1d2944, "register_interrupt", 4 },
    { 0xff2806f8, "RegisterHead1InterruptHandler", 3 },
    { 0xFF068BB8, "SetHPTimerAfter", 4 },
    { 0xff1c4a34, "LockEngineResources", 1, LockEngineResources_log },
    { 0xff1c4618, "LockEngineResources_0", 1, LockEngineResources_log_r4 },  // note: hooking in the middle of the function to avoid conflict with engio_write
    { 0xff1c4794, "UnLockEngineResources", 1, UnLockEngineResources_log_r7}, // same here, for RegisterHead1InterruptHandler
#endif

#ifdef CONFIG_500D
    { 0xff1a62a0, "StateTransition", 4 , state_transition_log },
    { 0xff1a69a4, "TryPostEvent", 5 },
    { 0xff1a602c, "TryPostStageEvent", 5 },

    { 0xff18fb90, "ConnectReadEDmac", 2 },
    { 0xff18fb68, "ConnectWriteEDmac", 2 },
    { 0xff18fd60, "RegisterEDmacCompleteCBR", 3 },
    { 0xff18fb4c, "SetEDmac", 4 },
    { 0xff18fbf0, "StartEDmac", 2 },
    
    //~ { 0xff1a0b90, "register_interrupt", 4 },                // conflicts with ConnectReadEDmac
    //~ { 0xff32646c, "register_obinteg_cbr", 2 },              // conflicts with set_digital_gain_and_related
    { 0xff2f6c7c, "set_digital_gain_and_related", 3 },
    //~ { 0xff2f6bf0, "set_saturate_offset", 1 },               // conflicts with ConnectReadEDmac
    { 0xff2f6bb8, "set_saturate_offset_2", 1 },
    { 0xff171e94, "set_fps_maybe", 1, fps_log },
    { 0xff23180c, "set_fps_maybe_2", 1 },
    //~ { 0xff2302a8, "AJ_FixedPoint_aglrw_related", 4 },       // conflicts with StateTransition
    { 0xff2314ac, "RegisterHead1InterruptHandler", 3 },
    //~ { 0xff1719a0, "RegisterHead2InterruptHandler", 3 },     // conflicts with TryPostEvent
    { 0xff171acc, "RegisterHead3InterruptHandler", 3 },
    //~ { 0xff171b84, "RegisterHead4InterruptHandler", 3 },     // conflicts with ConnectReadEDmac
    { 0xFF068268, "SetHPTimerAfter", 4 },
    { 0xff194190, "LockEngineResources", 1, LockEngineResources_log },
    { 0xff193ed0, "UnLockEngineResources", 1, UnLockEngineResources_log},
#endif

#ifdef CONFIG_5D3  /* 1.2.3 */
    { 0x178ec, "StateTransition", 4 , state_transition_log },
    { 0x17d54, "TryPostEvent", 5 },
    { 0x17674, "TryPostStageEvent", 5 },

    { 0x12768, "ConnectReadEDmac", 2 },
    { 0x126a4, "ConnectWriteEDmac", 2 },
    { 0x12afc, "RegisterEDmacCompleteCBR", 3 },
    { 0x125f8, "SetEDmac", 4 },
    { 0x12910, "StartEDmac", 2 },
    
    { 0x83b8, "register_interrupt", 4 },
    { 0xff3aa650, "set_digital_gain_maybe", 3 },
    { 0xff4fa6ac, "set_fps_maybe", 1, fps_log },
    { 0xff2170d8, "SetSsgVsize_fps_timer_B", 1 },
    { 0x4588, "SetTgNextState", 2 },
    { 0xff216a20, "RegisterHead1InterruptHandler", 3 },
    { 0xff4fa16c, "RegisterHead2InterruptHandler", 3 },
    //~ { 0xff4fa224, "RegisterHead3InterruptHandler", 3 }, /* conflicts with RegisterHead1InterruptHandler */
    { 0xff4fa4ec, "RegisterHead4InterruptHandler", 3 },
    { 0x7218, "SetHPTimerAfter", 4 },
    { 0xff2953b8, "LockEngineResources", 1, LockEngineResources_log },
    { 0xff29555c, "UnLockEngineResources", 1, UnLockEngineResources_log},
    
    { 0xff290f98, "engio_write", 1, engio_write_log},   /* on 5D3, Canon's engio_write is silent */
    { 0xff290c80, "EngDrvOut", 2 },                     /* same here */
    { 0xff290ca0, "EngDrvIn", 1 },
    { 0xff290d38, "EngDrvBits", 3 },
    { 0xff290cd4, "EngDrvIns", 3 },
    /* only EngDrvOuts is verbose, no need to log it here */
#endif
};
#endif

/* format arg to string and try to guess its type, with snprintf-like usage */
/* (string, ROM function name or regular number) */
static int snprintf_guess_arg(char* buf, int maxlen, uint32_t arg)
{
    if (looks_like_string(arg))
    {
        return snprintf(buf, maxlen, "\"%s\"", arg);
    }
    else if (is_sane_ptr(arg) && looks_like_string(MEM(arg)))
    {
        return snprintf(buf, maxlen, "&\"%s\"", MEM(arg));
    }
    else
    {
        char* guessed_name = 0;
        
        /* ROM function? try to guess its name */
        /* todo: also for RAM functions (how to recognize them quickly?) */
        if ((arg & 0xF0000000) == 0xF0000000)
        {
            guessed_name = asm_guess_func_name_from_string(arg);
        }
        
        if (guessed_name && guessed_name[0])
        {
            return snprintf(buf, maxlen, "0x%x \"%s\"", arg, guessed_name);
        }
        else
        {
            return snprintf(buf, maxlen, "0x%x", arg);
        }
    }
}

static void generic_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    uint32_t caller = PATCH_HOOK_CALLER();
    
    int num_args = 0;
    char* func_name = 0;
    
    for (int i = 0; i < COUNT(logged_functions); i++)
    {
        if (logged_functions[i].addr == pc)
        {
            num_args = logged_functions[i].num_args;
            func_name = logged_functions[i].name;
            break;
        }
    }
    
    char msg[200];
    int len;

    if (func_name)
    {
        len = snprintf(msg, sizeof(msg), "*** %s(", func_name);
    }
    else
    {
        len = snprintf(msg, sizeof(msg), "*** FUNC(%x)(", pc);
    }

    for (int i = 0; i < num_args; i++)
    {
        uint32_t arg = (i < 4) ? regs[i] : stack[i-4];

        len += snprintf_guess_arg(msg + len, sizeof(msg) - len, arg);
        
        if (i < num_args -1)
        {
            len += snprintf(msg + len, sizeof(msg) - len, ", ");
        }
    }
    len += snprintf(msg + len, sizeof(msg) - len, "), from %x", caller);
    
    DryosDebugMsg(0, 0, "%s", msg);
}

static void state_transition_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    struct state_object * state = (void*) regs[0];
    int old_state = state->current_state;
    char* state_name = (char*) state->name;
    int input = regs[2];
    int next_state = state->state_matrix[old_state + state->max_states * input].next_state;
    uint32_t next_function = (uint32_t) state->state_matrix[old_state + state->max_states * input].state_transition_function;
    /* function name is on the next line, no need to print it here */

    DryosDebugMsg(0, 0, 
        "*** %s: (%d) --%d--> (%d)          "
        "%x (x=%x z=%x t=%x)", state_name, old_state, input, next_state,
        next_function, regs[1], regs[3], stack[0]
    );
}

static void LockEngineResources_log_base(uint32_t* regs, char* name, uint32_t* arg0)
{
    uint32_t* resLock = arg0;
    uint32_t* resIds = (void*) resLock[5];
    int resNum = resLock[6];
    uint32_t caller = PATCH_HOOK_CALLER();

    DryosDebugMsg(0, 0, "*** %s(%x) x%d from %x:", name, resLock, resNum, caller);
    for (int i = 0; i < resNum; i++)
    {
        uint32_t class = resIds[i] & 0xFFFF0000;
        uint32_t entry = resIds[i] & 0x0000FFFF;
        
        char* desc = 
            class == 0x00000000 ? "write channel 0x%x" :
            class == 0x00010000 ? "read channel 0x%x" :
            class == 0x00020000 ? "write connection 0x%x" :
            class == 0x00030000 ? "read connection 0x%x" :
            class == 0x00110000 ? "%sPBAccessHandle" :
            "?";
        
        uint32_t arg =
            class == 0x00000000 ? edmac_index_to_channel(entry, EDMAC_DIR_WRITE) :
            class == 0x00010000 ? edmac_index_to_channel(entry, EDMAC_DIR_READ) :
            class == 0x00110000 ? (uint32_t)(entry ? "Bitmap" : "Image") :
            entry;
        
        if (strchr(desc, '%'))
        {
            char msg[100];
            snprintf(msg, sizeof(msg), desc, arg);
            DryosDebugMsg(0, 0, "    %2d) %8x (%s)", (i+1), resIds[i], msg);
        }
        else
        {
            DryosDebugMsg(0, 0, "    %2d) %8x (%s)", (i+1), resIds[i], desc);
        }
    }
}

static void LockEngineResources_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    LockEngineResources_log_base(regs, "LockEngineResources", (void*) regs[0]);
}

static void UnLockEngineResources_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    LockEngineResources_log_base(regs, "UnLockEngineResources", (void*) regs[0]);
}

static void LockEngineResources_log_r4(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    LockEngineResources_log_base(regs, "LockEngineResources", (void*) regs[4]);
}

static void UnLockEngineResources_log_r7(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    LockEngineResources_log_base(regs, "UnLockEngineResources", (void*) regs[7]);
}

static void fps_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    /* log the original call as usual */
    generic_log(regs, stack, pc);

    /* force very slow FPS to avoid lockup */
    regs[0] = 8191;
}

static void engio_write_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    uint32_t* data_buf = (uint32_t*) regs[0];
    uint32_t caller = PATCH_HOOK_CALLER();

    DryosDebugMsg(0, 0, "*** engio_write(%x) from %x:", data_buf, caller);
    
    /* log all ENGIO register writes */
    while(*data_buf != 0xFFFFFFFF)
    {
        uint32_t reg = *data_buf;
        data_buf++;
        uint32_t val = *data_buf;
        data_buf++;
        DryosDebugMsg(0, 0, "    0x%x: %x", reg, val);
    }
}

static void mpu_decode(char* in, char* out, int max_len)
{
    int len = 0;
    int size = in[0];

    /* print each byte as hex */
    for (char* c = in; c < in + size; c++)
    {
        len += snprintf(out+len, max_len-len, "%02x ", *c);
    }
    
    /* trim the last space */
    if (len) out[len-1] = 0;
}

static void mpu_send_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    uint32_t caller = PATCH_HOOK_CALLER();
    char* buf = (char*) regs[0];
    int size = regs[1];                 /* message size */
    int size_ex = (size + 2) & 0xFE;    /* packet size, prepended to the message */
                                        /* must be multiple of 2, so it's either size+1 or size+2 */
    char msg[256];
    mpu_decode(buf, msg, sizeof(msg));
    DryosDebugMsg(0, 0, "*** mpu_send(%02x %s), from %x", size_ex, msg, caller);
}

static void mpu_recv_log(uint32_t* regs, uint32_t* stack, uint32_t pc)
{
    uint32_t caller = PATCH_HOOK_CALLER();
    char* buf = (char*) regs[0];
    int size = buf[-1];
    char msg[256];
    mpu_decode(buf, msg, sizeof(msg));
    DryosDebugMsg(0, 0, "*** mpu_recv(%02x %s), from %x", size, msg, caller);
}

static int check_no_conflicts(int i)
{
    /* two memory addresses can't be patched at the same time if their index bits are the same */
    /* fixme: hardcoded masks */
    int index_mask = 0x7E0;
    
    /* but if the higher bits are identical, it's all fine */
    int high_mask = 0xFFFFF800;

    int a = logged_functions[i].addr;

    if (!IS_ROM_PTR(a))
    {
        /* this function is patched in RAM - no cache conflicts possible */
        return 1;
    }

    for (int j = 0; j < i; j++)
    {
        int b = logged_functions[j].addr;

        if (!IS_ROM_PTR(b))
        {
            /* this function is patched in RAM - no cache conflicts possible */
            continue;
        }

        if (((a & index_mask) == (b & index_mask)) && ((a & high_mask) != (b & high_mask)))
        {
            console_show();
            printf("Birthday paradox: %x %s\n"
                   "  conflicts with: %x %s.\n", 
                logged_functions[i].addr, logged_functions[i].name,
                logged_functions[j].addr, logged_functions[j].name
            );
            return 0;
        }
    }
    
    /* all fine; let's also check the other cache patches, unrelated to our hooks */
    if (!cache_locked())
    {
        /* no conflicts possible */
        return 1;
    }
    
    if (cache_is_patchable(logged_functions[i].addr,   TYPE_ICACHE, 0))
    {
        /* seems OK */
        return 1;
    }

    /* NOK, conflict with some other patch */
    console_show();
    printf("%x %s not patchable.\n", logged_functions[i].addr, logged_functions[i].name);
    return 0;
}

void dm_spy_extra_install()
{
    printf("ICache: %db, idx=%x tag=%x word=%x seg=%x\n",
        1<<CACHE_SIZE_BITS(TYPE_ICACHE),
        CACHE_INDEX_ADDRMASK(TYPE_ICACHE),
        CACHE_TAG_ADDRMASK(TYPE_ICACHE),
        CACHE_WORD_ADDRMASK(TYPE_ICACHE),
        CACHE_SEGMENT_ADDRMASK(TYPE_ICACHE)
    );

    for (int i = 0; i < COUNT(logged_functions); i++)
    {
        if (logged_functions[i].addr)
        {
            if (check_no_conflicts(i))
            {
                patch_hook_function(
                    logged_functions[i].addr, MEM(logged_functions[i].addr),
                    logged_functions[i].log_func ? logged_functions[i].log_func : generic_log,
                    logged_functions[i].name
                );
            }
        }
    }
}

void dm_spy_extra_uninstall()
{
    for (int i = 0; i < COUNT(logged_functions); i++)
    {
        unpatch_memory(logged_functions[i].addr);
    }
}
