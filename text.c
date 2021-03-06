#include "main.h"

static void drawtexth(int x, int y, char_t *str, uint16_t length, int d, int h, int hlen, uint16_t lineheight)
{
    h -= d;
    if(h + hlen < 0 || h > length) {
        drawtext(x, y, str, length);
        return;
    } else if(hlen == 0) {
        drawtext(x, y, str, length);
        int w =  textwidth(str, h + hlen);
        drawvline(x + w, y, y + lineheight, BLACK);
        return;
    }

    if(h < 0) {
        hlen += h;
        h = 0;
        if(hlen < 0) {
            hlen = 0;
        }
    }

    if(h + hlen > length) {
        hlen = length - h;
    }

    int width;

    width = drawtext_getwidth(x, y, str, h);

    uint32_t color = setcolor(TEXT_HIGHLIGHT);

    int w = textwidth(str + h, hlen);
    drawrectw(x + width, y, w, lineheight, TEXT_HIGHLIGHT_BG);
    drawtext(x + width, y, str + h, hlen);
    width += w;

    setcolor(color);

    drawtext(x + width, y, str + h + hlen, length - (h + hlen));
}

int drawtextmultiline(int x, int right, int y, uint16_t lineheight, char_t *data, uint16_t length, uint16_t h, uint16_t hlen, _Bool multiline)
{
    uint32_t c;
    _Bool greentext = 0;
    int xc = x;
    char_t *a = data, *b = a, *end = a + length;
    while(1) {
        if(*a == '>' && (a == data || *a == '\n')) {
            c = setcolor(RGB(0, 128, 0));
            greentext = 1;
        }

        if(a == end || *a == ' ' || *a == '\n') {
            int count = a - b, w = textwidth(b, count);
            while(x + w > right) {
                if(multiline && x == xc) {
                    int fit = textfit(b, count, right - x);
                    drawtexth(x, y, b, fit, b - data, h, hlen, lineheight);
                    count -= fit;
                    b += fit;
                    y += lineheight;
                } else if(!multiline) {
                    int fit = textfit(b, count, right - x);
                    drawtexth(x, y, b, fit, b - data, h, hlen, lineheight);
                    return y + lineheight;
                } else if(x != xc || *a == '\n') {
                    y += lineheight;
                    int l = utf8_len(b);
                    count -= l;
                    b += l;
                }
                x = xc;
                w = textwidth(b, count);
            }

            drawtexth(x, y, b, count, b - data, h, hlen, lineheight);

            x += w;
            b = a;

            if(a == end) {
                if(greentext) {
                    setcolor(c);
                    greentext = 0;
                }
                break;
            }

            if(*a == '\n') {
                if(greentext) {
                    setcolor(c);
                    greentext = 0;
                }
                y += lineheight;
                b += utf8_len(b);
                x = xc;
            }

        }
        a += utf8_len(a);
    }

    return y + lineheight;
}

uint16_t hittextmultiline(int mx, int right, int my, int height, uint16_t lineheight, char_t *str, uint16_t length, _Bool multiline)
{
    if(my < 0) {
        return 0;
    }

    if(my >= height) {
        return length;
    }

    int x = 0;
    char_t *a = str, *b = str, *end = str + length;
    while(1) {
        if(a == end ||  *a == '\n' || *a == ' ') {
            int count = a - b, w = textwidth(b, a - b);
            while(x + w > right && my >= lineheight) {
                if(multiline && x == 0) {
                    int fit = textfit(b, count, right);
                    count -= fit;
                    b += fit;
                    my -= lineheight;
                    height -= lineheight;
                } else if(!multiline) {
                    break;
                } else if(x != 0 || *a == '\n') {
                    my -= lineheight;
                    height -= lineheight;
                    int l = utf8_len(b);
                    count -= l;
                    b += l;
                }

                if(my >= -lineheight && my < 0) {
                    x = mx;
                    break;
                }

                x = 0;
                w = textwidth(b, count);
            }

            if(a == end) {
                if(my >= lineheight) {
                    return length;
                }
                break;
            }
            if((my >= 0 && my < lineheight) && (mx < 0 || (mx >= x && mx < x + w))) {
                break;
            }
            x += w;
            b = a;

            if(*a == '\n') {
                if(my >= 0 && my < lineheight) {
                    x = mx;
                    return a - str;
                }
                b += utf8_len(b);
                my -= lineheight;
                height -= lineheight;
                x = 0;
            }
        }
        a += utf8_len(a);
    }

    int fit;
    if(mx - x > 0) {
        int len = a - b;
        fit = textfit(b, len + (a != end), mx - x);
    } else {
        fit = 0;
    }

    return (b - str) + fit;
}

int text_height(int right, uint16_t lineheight, char_t *str, uint16_t length)
{
    int x = 0, y = 0;
    char_t *a = str, *b = a, *end = a + length;
    while(1) {
        if(a == end || *a == ' ' || *a == '\n') {
            int count = a - b, w = textwidth(b, count);
            while(x + w > right) {
                if(x == 0) {
                    int fit = textfit(b, count, right);
                    count -= fit;
                    b += fit;
                    y += lineheight;
                } else {
                    y += lineheight;
                    int l = utf8_len(b);
                    count -= l;
                    b += l;
                }
                x = 0;
                w = textwidth(b, count);
            }

            x += w;
            b = a;

            if(a == end) {
                break;
            }

            if(*a == '\n') {
                y += lineheight;
                b += utf8_len(b);
                x = 0;
            }

        }
        a += utf8_len(a);
    }

    y += lineheight;

    return y;
}

static void textxy(int width, uint16_t pp, uint16_t lineheight, char_t *str, uint16_t length, int *outx, int *outy)
{
    int x = 0, y = 0;
    char_t *a = str, *b = str, *end = str + length, *p = str + pp;
    while(1) {
        if(a == end ||  *a == '\n' || *a == ' ') {
            int count = a - b, w = textwidth(b, a - b);
            while(x + w > width) {
                if(x == 0) {
                    int fit = textfit(b, count, width);
                    if(p >= b && p < b + fit) {
                        break;
                    }
                    count -= fit;
                    b += fit;
                    y += lineheight;
                } else {
                    y += lineheight;
                    int l = utf8_len(b);
                    count -= l;
                    b += l;
                }
                x = 0;
                w = textwidth(b, count);
            }

            if(p >= b && p < b + count) {
                w = textwidth(b, p - b);
                a = end;
            }

            x += w;
            if(a == end) {
                break;
            }
            b = a;

            if(*a == '\n') {
                if(p == a) {
                    break;
                }
                b += utf8_len(b);
                y += lineheight;
                x = 0;
            }
        }
        a += utf8_len(a);
    }

    *outx = x;
    *outy = y;
}

uint16_t text_lineup(int width, uint16_t p, uint16_t lineheight, char_t *str, uint16_t length)
{
    //lazy
    int x, y;
    textxy(width, p, lineheight, str, length, &x, &y);
    if(y == 0) {
        return p;
    }
    return hittextmultiline(x, width, y - lineheight, INT_MAX, lineheight, str, length, 1);
}

uint16_t text_linedown(int width, uint16_t p, uint16_t lineheight, char_t *str, uint16_t length)
{
    //lazy
    int x, y;
    textxy(width, p, lineheight, str, length, &x, &y);
    return hittextmultiline(x, width, y + lineheight, INT_MAX, lineheight, str, length, 1);
}
