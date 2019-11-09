
def generate_c_code(icosoc_h, icosoc_c, mod):
    icosoc_h.append("""
static inline void icosoc_%s_setpixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    int n = 32*x + y;
    *(uint32_t*)(0x20000000 + %s * 0x10000 + 4*n) = (r << 16) | (g << 8) | b;
}
""" % (mod["name"], mod["addr"]))

