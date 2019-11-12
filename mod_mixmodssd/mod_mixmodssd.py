
def generate_c_code(icosoc_h, icosoc_c, mod):
    icosoc_h.append("""
static inline void icosoc_%s_set(uint16_t x)
{
    *(uint32_t*)(0x20000000 + %s * 0x10000) = x;
}
""" % (mod["name"], mod["addr"]))

