/* 包含头文件 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stdefine.h"
#include "bitstr.h"
#include "huffman.h"
#include "quant.h"
#include "zigzag.h"
#include "dct.h"
#include "bmp.h"
#include "color.h"
#include "jfif.h"

// 预编译开关
#define DEBUG_JFIF  0

// 内部类型定义
typedef struct {
    // width & height
    int       width;
    int       height;

    // quantization table
    int      *pqtab[16];

    // huffman codec ac
    HUFCODEC *phcac[16];

    // huffman codec dc
    HUFCODEC *phcdc[16];

    // components
    int comp_num;
    struct {
        int id;
        int samp_factor_v;
        int samp_factor_h;
        int qtab_idx;
        int htab_idx_ac;
        int htab_idx_dc;
    } comp_info[4];

    int   datalen;
    BYTE *databuf;
} JFIF;

/* 内部函数实现 */
#if DEBUG_JFIF
static void jfif_dump(JFIF *jfif)
{
    int i, j;

    printf("++ jfif dump ++\n");
    printf("width : %d\n", jfif->width );
    printf("height: %d\n", jfif->height);
    printf("\n");

    for (i=0; i<16; i++) {
        if (!jfif->pqtab[i]) continue;
        printf("qtab%d\n", i);
        for (j=0; j<64; j++) {
            printf("%3d,%c", jfif->pqtab[i][j], j%8 == 7 ? '\n' : ' ');
        }
        printf("\n");
    }

    for (i=0; i<16; i++) {
        int size = 16;
        if (!jfif->phcac[i]) continue;
        printf("htabac%d\n", i);
        for (j=0; j<16; j++) {
            size += jfif->phcac[i]->huftab[j];
        }
        for (j=0; j<size; j++) {
            printf("%3d,%c", jfif->phcac[i]->huftab[j], j%16 == 15 ? '\n' : ' ');
        }
        printf("\n\n");
    }

    for (i=0; i<16; i++) {
        int size = 16;
        if (!jfif->phcdc[i]) continue;
        printf("htabdc%d\n", i);
        for (j=0; j<16; j++) {
            size += jfif->phcdc[i]->huftab[j];
        }
        for (j=0; j<size; j++) {
            printf("%3d,%c", jfif->phcdc[i]->huftab[j], j%16 == 15 ? '\n' : ' ');
        }
        printf("\n\n");
    }

    printf("comp_num : %d\n", jfif->comp_num);
    for (i=0; i<jfif->comp_num; i++) {
        printf("id:%d samp_factor_v:%d samp_factor_h:%d qtab_idx:%d htab_idx_ac:%d htab_idx_dc:%d\n",
            jfif->comp_info[i].id,
            jfif->comp_info[i].samp_factor_v,
            jfif->comp_info[i].samp_factor_h,
            jfif->comp_info[i].qtab_idx,
            jfif->comp_info[i].htab_idx_ac,
            jfif->comp_info[i].htab_idx_dc);
    }
    printf("\n");

    printf("datalen : %d\n", jfif->datalen);
    printf("-- jfif dump --\n");
}

static void dump_du(int *du)
{
    int i;
    for (i=0; i<64; i++) {
        printf("%3d%c", du[i], i % 8 == 7 ? '\n' : ' ');
    }
    printf("\n");
}
#endif

static int ALIGN(int x, int y) {
    // y must be a power of 2.
    return (x + y - 1) & ~(y - 1);
}

static void category_encode(int *code, int *size)
{
    unsigned absc = abs(*code);
    unsigned mask = (1 << 15);
    int i    = 15;
    if (absc == 0) { *size = 0; return; }
    while (i && !(absc & mask)) { mask >>= 1; i--; }
    *size = i + 1;
    if (*code < 0) *code = (1 << *size) - absc - 1;
}

static int category_decode(int code, int  size)
{
    return code >= (1 << (size - 1)) ? code : code - (1 << size) + 1;
}

/* 函数实现 */
void* jfif_load(char *file)
{
    JFIF *jfif   = NULL;
    FILE *fp     = NULL;
    int   header = 0;
    int   type   = 0;
    WORD  size   = 0;
    BYTE *buf    = NULL;
    BYTE *end    = NULL;
    BYTE *dqt, *dht;
    int   ret    =-1;
    long  offset = 0;
    int   i;

    jfif = calloc(1, sizeof(JFIF));
    buf  = calloc(1, 0x10000);
    end  = buf + 0x10000;
    if (!jfif || !buf) goto done;

    fp = fopen(file, "rb");
    if (!fp) goto done;

    while (1) {
        do { header = fgetc(fp); } while (header != EOF && header != 0xff); // get header
        do { type   = fgetc(fp); } while (type   != EOF && type   == 0xff); // get type
        if (header == EOF || type == EOF) {
            printf("file eof !\n");
            break;
        }

        if ((type == 0xd8) || (type == 0xd9) || (type == 0x01) || (type >= 0xd0 && type <= 0xd7)) {
            size = 0;
        } else {
            size  = fgetc(fp) << 8;
            size |= fgetc(fp) << 0;
            size -= 2;
        }

        size = fread(buf, 1, size, fp);
        switch (type) {
        case 0xc0: // SOF0
            jfif->width    = (buf[3] << 8) | (buf[4] << 0);
            jfif->height   = (buf[1] << 8) | (buf[2] << 0);
            jfif->comp_num =  buf[5] < 4 ? buf[5] : 4;
            for (i=0; i<jfif->comp_num; i++) {
                jfif->comp_info[i].id = buf[6 + i * 3];
                jfif->comp_info[i].samp_factor_v = (buf[7 + i * 3] >> 0) & 0x0f;
                jfif->comp_info[i].samp_factor_h = (buf[7 + i * 3] >> 4) & 0x0f;
                jfif->comp_info[i].qtab_idx      =  buf[8 + i * 3] & 0x0f;
            }
            break;

        case 0xda: // SOS
            jfif->comp_num = buf[0] < 4 ? buf[0] : 4;
            for (i=0; i<jfif->comp_num; i++) {
                jfif->comp_info[i].id = buf[1 + i * 2];
                jfif->comp_info[i].htab_idx_ac = (buf[2 + i * 2] >> 0) & 0x0f;
                jfif->comp_info[i].htab_idx_dc = (buf[2 + i * 2] >> 4) & 0x0f;
            }
            offset = ftell(fp);
            ret    = 0;
            goto read_data;

        case 0xdb: // DQT
            dqt = buf;
            while (size > 0 && dqt < end) {
                int idx = dqt[0] & 0x0f;
                int f16 = dqt[0] & 0xf0;
                if (!jfif->pqtab[idx]) jfif->pqtab[idx] = malloc(64 * sizeof(int));
                if (!jfif->pqtab[idx]) break;
                if (dqt + 1 + 64 + (f16 ? 64 : 0) < end) {
                    for (i=0; i<64; i++) {
                        jfif->pqtab[idx][ZIGZAG[i]] = f16 ? ((dqt[1 + i * 2] << 8) | (dqt[2 + i * 2] << 0)) : dqt[1 + i];
                    }
                }
                dqt += 1 + 64 + (f16 ? 64 : 0);
                size-= 1 + 64 + (f16 ? 64 : 0);
            }
            break;

        case 0xc4: // DHT
            dht = buf;
            while (size > 0 && dht + 17 < end) {
                int idx = dht[0] & 0x0f;
                int fac = dht[0] & 0xf0;
                int len = 0;
                for (i=1; i<1+16; i++) len += dht[i];
                if (len > end - dht - 17) len = end - dht - 17;
                if (len > 256) len = 256;
                if (fac) {
                    if (!jfif->phcac[idx]) jfif->phcac[idx] = calloc(1, sizeof(HUFCODEC));
                    if ( jfif->phcac[idx]) memcpy(jfif->phcac[idx]->huftab, &dht[1], 16 + len);
                } else {
                    if (!jfif->phcdc[idx]) jfif->phcdc[idx] = calloc(1, sizeof(HUFCODEC));
                    if ( jfif->phcdc[idx]) memcpy(jfif->phcdc[idx]->huftab, &dht[1], 16 + len);
                }
                dht += 17 + len;
                size-= 17 + len;
            }
            break;
        }
    }

read_data:
    fseek(fp, 0, SEEK_END);
    jfif->datalen = ftell(fp) - offset;
    jfif->databuf = malloc(jfif->datalen);
    if (jfif->databuf) {
        fseek(fp, offset, SEEK_SET);
        fread(jfif->databuf, 1, jfif->datalen, fp);
    }

done:
    if (buf) free  (buf);
    if (fp ) fclose(fp );
    if (ret == -1) {
        jfif_free(jfif);
        jfif = NULL;
    }
    return jfif;
}

int jfif_save(void *ctxt, char *file)
{
    JFIF *jfif = (JFIF*)ctxt;
    FILE *fp   = NULL;
    int   len  = 0;
    int   i, j;
    int   ret  = -1;

    fp = fopen(file, "wb");
    if (!fp) goto done;

    // output SOI
    fputc(0xff, fp);
    fputc(0xd8, fp);

    // output DQT
    for (i=0; i<16; i++) {
        if (!jfif->pqtab[i]) continue;
        len = 2 + 1 + 64;
        fputc(0xff, fp);
        fputc(0xdb, fp);
        fputc(len >> 8, fp);
        fputc(len >> 0, fp);
        fputc(i   , fp);
        for (j=0; j<64; j++) {
            fputc(jfif->pqtab[i][ZIGZAG[j]], fp);
        }
    }

    // output SOF0
    len = 2 + 1 + 2 + 2 + 1 + 3 * jfif->comp_num;
    fputc(0xff, fp);
    fputc(0xc0, fp);
    fputc(len >> 8, fp);
    fputc(len >> 0, fp);
    fputc(8   , fp); // precision 8bit
    fputc(jfif->height >> 8, fp); // height
    fputc(jfif->height >> 0, fp); // height
    fputc(jfif->width  >> 8, fp); // width
    fputc(jfif->width  >> 0, fp); // width
    fputc(jfif->comp_num, fp);
    for (i=0; i<jfif->comp_num; i++) {
        fputc(jfif->comp_info[i].id, fp);
        fputc((jfif->comp_info[i].samp_factor_v << 0)|(jfif->comp_info[i].samp_factor_h << 4), fp);
        fputc(jfif->comp_info[i].qtab_idx, fp);
    }

    // output DHT AC
    for (i=0; i<16; i++) {
        if (!jfif->phcac[i]) continue;
        fputc(0xff, fp);
        fputc(0xc4, fp);
        len = 2 + 1 + 16;
        for (j=0; j<16; j++) len += jfif->phcac[i]->huftab[j];
        fputc(len >> 8, fp);
        fputc(len >> 0, fp);
        fputc(i + 0x10, fp);
        fwrite(jfif->phcac[i]->huftab, len - 3, 1, fp);
    }

    // output DHT DC
    for (i=0; i<16; i++) {
        if (!jfif->phcdc[i]) continue;
        fputc(0xff, fp);
        fputc(0xc4, fp);
        len = 2 + 1 + 16;
        for (j=0; j<16; j++) len += jfif->phcdc[i]->huftab[j];
        fputc(len >> 8, fp);
        fputc(len >> 0, fp);
        fputc(i + 0x00, fp);
        fwrite(jfif->phcdc[i]->huftab, len - 3, 1, fp);
    }

    // output SOS
    len = 2 + 1 + 2 * jfif->comp_num + 3;
    fputc(0xff, fp);
    fputc(0xda, fp);
    fputc(len >> 8, fp);
    fputc(len >> 0, fp);
    fputc(jfif->comp_num, fp);
    for (i=0; i<jfif->comp_num; i++) {
        fputc(jfif->comp_info[i].id, fp);
        fputc((jfif->comp_info[i].htab_idx_ac << 0)|(jfif->comp_info[i].htab_idx_dc << 4), fp);
    }
    fputc(0x00, fp);
    fputc(0x00, fp);
    fputc(0x00, fp);

    // output data
    if (jfif->databuf) {
        fwrite(jfif->databuf, jfif->datalen, 1, fp);
    }
    ret = 0;

done:
    if (fp) fclose(fp);
    return ret;
}

void jfif_free(void *ctxt)
{
    JFIF *jfif = (JFIF*)ctxt;
    int   i;
    if (!jfif) return;
    for (i=0; i<16; i++) {
        if (jfif->pqtab[i]) free(jfif->pqtab[i]);
        if (jfif->phcac[i]) free(jfif->phcac[i]);
        if (jfif->phcdc[i]) free(jfif->phcdc[i]);
    }
    if (jfif->databuf) free(jfif->databuf);
    free(jfif);
}

int jfif_decode(void *ctxt, BMP *pb)
{
    JFIF *jfif    = (JFIF*)ctxt;
    void *bs      = NULL;
    int  *ftab[16]= {0};
    int   dc[4]   = {0};
    int   mcuw, mcuh, mcuc, mcur, mcui, jw, jh;
    int   i, j, c, h, v, x, y;
    int   sfh_max = 0;
    int   sfv_max = 0;
    int   yuv_stride[3] = {0};
    int   yuv_height[3] = {0};
    int  *yuv_datbuf[3] = {0};
    int  *idst, *isrc;
    int  *ysrc, *usrc, *vsrc;
    BYTE *bdst;
    int   ret = -1;

    if (!ctxt || !pb) {
        printf("invalid input params !\n");
        return -1;
    }

    // init dct module
    init_dct_module();

    //++ init ftab
    for (i=0; i<16; i++) {
        if (jfif->pqtab[i]) {
            ftab[i] = malloc(64 * sizeof(int));
            if (ftab[i]) {
                init_idct_ftab(ftab[i], jfif->pqtab[i]);
            } else {
                goto done;
            }
        }
    }
    //-- init ftab

    //++ calculate mcu info
    for (c=0; c<jfif->comp_num; c++) {
        if (sfh_max < jfif->comp_info[c].samp_factor_h) {
            sfh_max = jfif->comp_info[c].samp_factor_h;
        }
        if (sfv_max < jfif->comp_info[c].samp_factor_v) {
            sfv_max = jfif->comp_info[c].samp_factor_v;
        }
    }
    mcuw = sfh_max * 8;
    mcuh = sfv_max * 8;
    jw = ALIGN(jfif->width , mcuw);
    jh = ALIGN(jfif->height, mcuh);
    mcuc = jw / mcuw;
    mcur = jh / mcuh;
    //-- calculate mcu info

    // create yuv buffer for decoding
    yuv_stride[0] = jw;
    yuv_stride[1] = jw * jfif->comp_info[1].samp_factor_h / sfh_max;
    yuv_stride[2] = jw * jfif->comp_info[2].samp_factor_h / sfh_max;
    yuv_height[0] = jh;
    yuv_height[1] = jh * jfif->comp_info[1].samp_factor_v / sfv_max;
    yuv_height[2] = jh * jfif->comp_info[2].samp_factor_v / sfv_max;
    yuv_datbuf[0] = malloc(yuv_stride[0] * yuv_height[0] * sizeof(int));
    yuv_datbuf[1] = malloc(yuv_stride[1] * yuv_height[1] * sizeof(int));
    yuv_datbuf[2] = malloc(yuv_stride[2] * yuv_height[2] * sizeof(int));
    if (!yuv_datbuf[0] || !yuv_datbuf[1] || !yuv_datbuf[2]) {
        goto done;
    }

    // open bit stream
    bs = bitstr_open(jfif->databuf, "mem", jfif->datalen);
    if (!bs) {
        printf("failed to open bitstr for jfif_decode !");
        return -1;
    }

    // init huffman codec
    for (i=0; i<16; i++) {
        if (jfif->phcac[i]) {
            jfif->phcac[i]->input = bs;
            huffman_decode_init(jfif->phcac[i]);
        }
        if (jfif->phcdc[i]) {
            jfif->phcdc[i]->input = bs;
            huffman_decode_init(jfif->phcdc[i]);
        }
    }

    for (mcui=0; mcui<mcuc*mcur; mcui++) {
        for (c=0; c<jfif->comp_num; c++) {
            for (v=0; v<jfif->comp_info[c].samp_factor_v; v++) {
                for (h=0; h<jfif->comp_info[c].samp_factor_h; h++) {
                    HUFCODEC *hcac = jfif->phcac[jfif->comp_info[c].htab_idx_ac];
                    HUFCODEC *hcdc = jfif->phcdc[jfif->comp_info[c].htab_idx_dc];
                    int       fidx = jfif->comp_info[c].qtab_idx;
                    int size, znum, code;
                    int du[64] = {0};

                    //+ decode dc
                    size = huffman_decode_step(hcdc) & 0xf;
                    if (size) {
                        code = bitstr_get_bits(bs  , size);
                        code = category_decode(code, size);
                    }
                    else {
                        code = 0;
                    }
                    dc[c] += code;
                    du[0]  = dc[c];
                    //- decode dc

                    //+ decode ac
                    for (i=1; i<64; ) {
                        code = huffman_decode_step(hcac);
                        if (code <= 0) break;
                        size = (code >> 0) & 0xf;
                        znum = (code >> 4) & 0xf;
                        i   += znum;
                        code = bitstr_get_bits(bs  , size);
                        code = category_decode(code, size);
                        if (i < 64) du[i++] = code;
                    }
                    //- decode ac

                    // de-zigzag
                    zigzag_decode(du);

                    // idct
                    idct2d8x8(du, ftab[fidx]);

                    // copy du to yuv buffer
                    x    = ((mcui % mcuc) * mcuw + h * 8) * jfif->comp_info[c].samp_factor_h / sfh_max;
                    y    = ((mcui / mcuc) * mcuh + v * 8) * jfif->comp_info[c].samp_factor_v / sfv_max;
                    idst = yuv_datbuf[c] + y * yuv_stride[c] + x;
                    isrc = du;
                    for (i=0; i<8; i++) {
                        memcpy(idst, isrc, 8 * sizeof(int));
                        idst += yuv_stride[c];
                        isrc += 8;
                    }
                }
            }
        }
    }

    // close huffman codec
    for (i=0; i<16; i++) {
        if (jfif->phcac[i]) huffman_decode_done(jfif->phcac[i]);
        if (jfif->phcdc[i]) huffman_decode_done(jfif->phcdc[i]);
    }

    // close bit stream
    bitstr_close(bs);

    // create bitmap, and convert yuv to rgb
    bmp_create(pb, jfif->width, jfif->height);
    bdst = (BYTE*)pb->pdata;
    ysrc = yuv_datbuf[0];
    for (i=0; i<jfif->height; i++) {
        int uy = i * jfif->comp_info[1].samp_factor_v / sfv_max;
        int vy = i * jfif->comp_info[2].samp_factor_v / sfv_max;
        for (j=0; j<jfif->width; j++) {
            int ux = j * jfif->comp_info[1].samp_factor_h / sfh_max;
            int vx = j * jfif->comp_info[2].samp_factor_h / sfh_max;
            usrc = yuv_datbuf[2] + uy * yuv_stride[2] + ux;
            vsrc = yuv_datbuf[1] + vy * yuv_stride[1] + vx;
            yuv_to_rgb(*ysrc, *vsrc, *usrc, bdst + 2, bdst + 1, bdst + 0);
            bdst += 3;
            ysrc += 1;
        }
        bdst -= jfif->width * 3;
        bdst += pb->stride;
        ysrc -= jfif->width * 1;
        ysrc += yuv_stride[0];
    }

    // success
    ret = 0;

done:
    if (yuv_datbuf[0]) free(yuv_datbuf[0]);
    if (yuv_datbuf[1]) free(yuv_datbuf[1]);
    if (yuv_datbuf[2]) free(yuv_datbuf[2]);
    //++ free ftab
    for (i=0; i<16; i++) {
        if (ftab[i]) {
            free(ftab[i]);
        }
    }
    //-- free ftab
    return ret;
}

#define DU_TYPE_LUMIN  0
#define DU_TYPE_CHROM  1

typedef struct {
    unsigned runlen   : 4;
    unsigned codesize : 4;
    unsigned codedata : 16;
} RLEITEM;

static void jfif_encode_du(JFIF *jfif, int type, int du[64], int *dc)
{
    HUFCODEC *hfcac = jfif->phcac[type];
    HUFCODEC *hfcdc = jfif->phcdc[type];
    int      *pqtab = jfif->pqtab[type];
    void     *bs    = hfcac->output;
    int       diff, code, size;
    RLEITEM   rlelist[63];
    int       i, j, n, eob;

    // fdct
    fdct2d8x8(du, NULL);

    // quant
    quant_encode(du, pqtab);

    // zigzag
    zigzag_encode(du);

    // dc
    diff = du[0] - *dc;
    *dc  = du[0];

    // category encode for dc
    code = diff;
    category_encode(&code, &size);

    // huffman encode for dc
    huffman_encode_step(hfcdc, size);
    bitstr_put_bits(bs, code, size);

    // rle encode for ac
    for (i=1, j=0, n=0, eob=0; i<64 && j<63; i++) {
        if (du[i] == 0 && n < 15) {
            n++;
        } else {
            code = du[i]; size = 0;
            category_encode(&code, &size);
            rlelist[j].runlen   = n;
            rlelist[j].codesize = size;
            rlelist[j].codedata = code;
            n = 0;
            j++;
            if (size != 0) eob = j;
        }
    }

    // set eob
    if (du[63] == 0) {
        rlelist[eob].runlen   = 0;
        rlelist[eob].codesize = 0;
        rlelist[eob].codedata = 0;
        j = eob + 1;
    }

    // huffman encode for ac
    for (i=0; i<j; i++) {
        huffman_encode_step(hfcac, (rlelist[i].runlen << 4) | (rlelist[i].codesize << 0));
        bitstr_put_bits(bs, rlelist[i].codedata, rlelist[i].codesize);
    }
}

void* jfif_encode(BMP *pb)
{
    JFIF *jfif = NULL;
    void *bs   = NULL;
    int   jw, jh;
    int  *yuv_datbuf[3] = {0};
    int  *ydst, *udst, *vdst;
    int  *isrc, *idst;
    BYTE *bsrc;
    int   du[64]= {0};
    int   dc[4 ]= {0};
    int   i, j, m, n;
    int   failed = 1;

    // check input params
    if (!pb) {
        printf("invalid input params !\n");
        return NULL;
    }

    // allocate jfif context
    jfif = calloc(1, sizeof(JFIF));
    if (!jfif) return NULL;

    // init dct module
    init_dct_module();

    // init jfif context
    jfif->width    = pb->width;
    jfif->height   = pb->height;
    jfif->pqtab[0] = malloc(64*sizeof(int));
    jfif->pqtab[1] = malloc(64*sizeof(int));
    jfif->phcac[0] = calloc(1, sizeof(HUFCODEC));
    jfif->phcac[1] = calloc(1, sizeof(HUFCODEC));
    jfif->phcdc[0] = calloc(1, sizeof(HUFCODEC));
    jfif->phcdc[1] = calloc(1, sizeof(HUFCODEC));
    jfif->datalen  = jfif->width * jfif->height * 2;
    jfif->databuf  = malloc(jfif->datalen);
    if (  !jfif->pqtab[0] || !jfif->pqtab[1]
       || !jfif->phcac[0] || !jfif->phcac[1]
       || !jfif->phcdc[0] || !jfif->phcdc[1]
       || !jfif->databuf ) {
        goto done;
    }

    // init qtab
    memcpy(jfif->pqtab[0], STD_QUANT_TAB_LUMIN, 64*sizeof(int));
    memcpy(jfif->pqtab[1], STD_QUANT_TAB_CHROM, 64*sizeof(int));

    // open bit stream
    bs = bitstr_open(jfif->databuf, "mem", jfif->datalen);
    if (!bs) {
        printf("failed to open bitstr for jfif_decode !");
        goto done;
    }

    // init huffman codec
    memcpy(jfif->phcac[0]->huftab, STD_HUFTAB_LUMIN_AC, MAX_HUFFMAN_CODE_LEN + 256);
    memcpy(jfif->phcac[1]->huftab, STD_HUFTAB_CHROM_AC, MAX_HUFFMAN_CODE_LEN + 256);
    memcpy(jfif->phcdc[0]->huftab, STD_HUFTAB_LUMIN_DC, MAX_HUFFMAN_CODE_LEN + 256);
    memcpy(jfif->phcdc[1]->huftab, STD_HUFTAB_CHROM_DC, MAX_HUFFMAN_CODE_LEN + 256);
    jfif->phcac[0]->output = bs; huffman_encode_init(jfif->phcac[0], 1);
    jfif->phcac[1]->output = bs; huffman_encode_init(jfif->phcac[1], 1);
    jfif->phcdc[0]->output = bs; huffman_encode_init(jfif->phcdc[0], 1);
    jfif->phcdc[1]->output = bs; huffman_encode_init(jfif->phcdc[1], 1);

    // init comp_num & comp_info
    jfif->comp_num                   = 3;
    jfif->comp_info[0].id            = 1;
    jfif->comp_info[0].samp_factor_v = 2;
    jfif->comp_info[0].samp_factor_h = 2;
    jfif->comp_info[0].qtab_idx      = 0;
    jfif->comp_info[0].htab_idx_ac   = 0;
    jfif->comp_info[0].htab_idx_dc   = 0;
    jfif->comp_info[1].id            = 2;
    jfif->comp_info[1].samp_factor_v = 1;
    jfif->comp_info[1].samp_factor_h = 1;
    jfif->comp_info[1].qtab_idx      = 1;
    jfif->comp_info[1].htab_idx_ac   = 1;
    jfif->comp_info[1].htab_idx_dc   = 1;
    jfif->comp_info[2].id            = 3;
    jfif->comp_info[2].samp_factor_v = 1;
    jfif->comp_info[2].samp_factor_h = 1;
    jfif->comp_info[2].qtab_idx      = 1;
    jfif->comp_info[2].htab_idx_ac   = 1;
    jfif->comp_info[2].htab_idx_dc   = 1;

    // init jw & jw, init yuv data buffer
    jw = ALIGN(pb->width , 16);
    jh = ALIGN(pb->height, 16);
    yuv_datbuf[0] = calloc(1, jw * jh / 1 * sizeof(int));
    yuv_datbuf[1] = calloc(1, jw * jh / 4 * sizeof(int));
    yuv_datbuf[2] = calloc(1, jw * jh / 4 * sizeof(int));
    if (!yuv_datbuf[0] || !yuv_datbuf[1] || !yuv_datbuf[2]) {
        goto done;
    }

    // convert rgb to yuv
    bsrc = pb->pdata;
    ydst = yuv_datbuf[0];
    udst = yuv_datbuf[1];
    vdst = yuv_datbuf[2];
    for (i=0; i<pb->height; i++) {
        for (j=0; j<pb->width; j++) {
            rgb_to_yuv(bsrc[2], bsrc[1], bsrc[0], ydst, udst, vdst);
            bsrc += 3;
            ydst += 1;
            if (j & 1) {
                udst += 1;
                vdst += 1;
            }
        }
        bsrc -= pb->width * 3; bsrc += pb->stride;
        ydst -= pb->width * 1; ydst += jw;
        udst -= pb->width / 2;
        vdst -= pb->width / 2;
        if (i & 1) {
            udst += jw / 2;
            vdst += jw / 2;
        }
    }

    for (m=0; m<jh/16; m++) {
        for (n=0; n<jw/16; n++) {
            //++ encode mcu, yuv 4:2:0
            //+ y du0
            isrc = yuv_datbuf[0] + (m * 16 + 0) * jw + n * 16 + 0;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_LUMIN, du, &(dc[0]));
            //- y du0

            //+ y du1
            isrc = yuv_datbuf[0] + (m * 16 + 0) * jw + n * 16 + 8;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_LUMIN, du, &(dc[0]));
            //- y du1

            //+ y du2
            isrc = yuv_datbuf[0] + (m * 16 + 8) * jw + n * 16 + 0;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_LUMIN, du, &(dc[0]));
            //- y du2

            //+ y du3
            isrc = yuv_datbuf[0] + (m * 16 + 8) * jw + n * 16 + 8;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_LUMIN, du, &(dc[0]));
            //- y du3

            //+ u du
            isrc = yuv_datbuf[1] + m * 8 * (jw/2) + n * 8;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw/2; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_CHROM, du, &(dc[1]));
            //- u du

            //+ v du
            isrc = yuv_datbuf[2] + m * 8 * (jw/2) + n * 8;
            idst = du;
            for (i=0; i<8; i++) {
                memcpy(idst, isrc, 8 * sizeof(int));
                isrc += jw/2; idst += 8;
            }
            jfif_encode_du(jfif, DU_TYPE_CHROM, du, &(dc[2]));
            //- v du
            //-- encode mcu, yuv 4:2:0
        }
    }
    failed = 0;

done:
    // free yuv data buffer
    if (yuv_datbuf[0]) free(yuv_datbuf[0]);
    if (yuv_datbuf[1]) free(yuv_datbuf[1]);
    if (yuv_datbuf[2]) free(yuv_datbuf[2]);

    // close huffman codec
    huffman_encode_done(jfif->phcac[0]);
    huffman_encode_done(jfif->phcac[1]);
    huffman_encode_done(jfif->phcdc[0]);
    huffman_encode_done(jfif->phcdc[1]);
    jfif->datalen = bitstr_tell(bs);

    // close bit stream
    bitstr_close(bs);

    // if failed free context
    if (failed) {
        jfif_free(jfif);
        jfif = NULL;
    }

    // return context
    return jfif;
}







