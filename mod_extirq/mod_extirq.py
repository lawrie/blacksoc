
def generate_c_code(icosoc_h, icosoc_c, mod):
    code = """
#define icosoc_@name@_trigger_hi 8
#define icosoc_@name@_trigger_lo 4
#define icosoc_@name@_trigger_re 2
#define icosoc_@name@_trigger_fe 1

static inline void icosoc_@name@_set_config(uint32_t bitmask) {
    *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = bitmask;
}

static inline uint32_t icosoc_@name@_get_config() {
    return *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000);
}

static inline bool icosoc_@name@_read() {
    return (*(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) & 0x80000000) != 0;
}
"""

    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_h.append(code)

