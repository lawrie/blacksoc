
def generate_c_code(icosoc_h, icosoc_c, mod):
    name = mod["name"]

    icosoc_h.append("void icosoc_%s_read(void *data, int len);" % name)
    icosoc_h.append("void icosoc_%s_write(const void *data, int len);" % name)
    icosoc_h.append("int icosoc_%s_read_nb(void *data, int maxlen);" % name)
    icosoc_h.append("int icosoc_%s_write_nb(const void *data, int maxlen);" % name)

    code = """
void icosoc_@name@_read(void *data, int len)
{
    while (len > 0) {
        int n = icosoc_@name@_read_nb(data, len);
        data += n, len -= n;
    }
}

void icosoc_@name@_write(const void *data, int len)
{
    while (len > 0) {
        int n = icosoc_@name@_write_nb(data, len);
        data += n, len -= n;
    }
}

int icosoc_@name@_read_nb(void *data, int maxlen)
{
    uint8_t *p = data;
    int len = *(volatile uint32_t*)(0x20000004 + @addr@ * 0x10000);
    if (len > maxlen) len = maxlen;

    for (int i = 0; i < len; i++)
        p[i] = *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000);

    return len;
}

int icosoc_@name@_write_nb(const void *data, int maxlen)
{
    const uint8_t *p = data;
    int len = *(volatile uint32_t*)(0x20000008 + @addr@ * 0x10000);
    if (len > maxlen) len = maxlen;

    for (int i = 0; i < len; i++)
        *(volatile uint32_t*)(0x20000000 + @addr@ * 0x10000) = p[i];

    return len;
}
"""

    code = code.replace("@name@", mod["name"])
    code = code.replace("@addr@", mod["addr"])
    icosoc_c.append(code)

