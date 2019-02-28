#include "iapcmd.h"
#include "cstdlib"
#include "cstdio"
#include "cstring"

iapCmd::iapCmd()
{

}

void iapCmd::addPktTail(char *out, int len)
{
    unsigned char chksum = 0;

    for (int i = 0; i < len; i++)
    {
        chksum = (unsigned char)(out[i] + chksum);
    }

    out[len] = chksum;
    out[len+1] = 0x0a;
}

void iapCmd::packet(char *buf, int len, int *outLen, char *out)
{
    if (buf == nullptr || len == 0)
    {
        return;
    }

    IAD_CMD_HDR_S hdr;
    int num = 0;

    hdr.start = 0x80;
    hdr.len = len + 4;
    hdr.cmd = E_IAP_CMD_DOWNLOAD_PACKET;
    hdr.ver = 0;

    memcpy(out, &hdr, sizeof(hdr));
    memcpy(out + sizeof(hdr), buf, len);
    num = sizeof(hdr) + len;

    //算chksum
    addPktTail(out, num);
    *outLen = num + 2;
}
void iapCmd::ConstructStartPkt(char *buf, int *len, unsigned long fileSize, unsigned long crc32)
{
    IAD_CMD_HDR_S hdr;
    int num = 0;

    hdr.cmd = E_IAP_CMD_ENTER_DOWNLOAD;
    hdr.ver = 0;
    hdr.start = 0x80;
    hdr.len = 3 * sizeof(unsigned long);

    memcpy(buf, &hdr, sizeof(hdr));
    num += sizeof(hdr);
    memcpy(buf + num, &fileSize, sizeof(unsigned long));
    num += sizeof(unsigned long);
    memcpy(buf + num, &crc32, sizeof(unsigned long));
    num += sizeof(unsigned long);

    *len = hdr.len + 2;

    addPktTail(buf, num);
}
void iapCmd::ConstructEndPkt(char *buf, int *len)
{
    IAD_CMD_HDR_S hdr;
    int num = 0;

    hdr.cmd = E_IAP_CMD_END_DOWNLOAD;
    hdr.ver = 0;
    hdr.start = 0x80;
    hdr.len = sizeof(unsigned long) * 2;
    *len = hdr.len + 2;

    memcpy(buf, &hdr, sizeof(hdr));
    num += sizeof(hdr);
    memset(buf + num, 0, sizeof(unsigned long));
    num += sizeof(unsigned long);

    addPktTail(buf, num);
}
