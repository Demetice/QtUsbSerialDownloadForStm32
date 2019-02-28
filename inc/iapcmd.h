#ifndef IAPCMD_H
#define IAPCMD_H

typedef enum
{
    E_IAP_CMD_BOOT_AND_RESET = 0x50,
    E_IAP_CMD_ENTER_DOWNLOAD = 0x80,
    E_IAP_CMD_DOWNLOAD_PACKET = 0x81,
    E_IAP_CMD_END_DOWNLOAD = 0x82,
    E_IAP_CMD_BUTT
}IAP_CMD_E;

typedef struct
{
    unsigned char start;
    unsigned char len;
    unsigned char cmd;
    unsigned char ver;
}IAD_CMD_HDR_S;


class iapCmd
{
public:
    iapCmd();
    void packet(char *buf, int len, int *outLen, char *out);
    void ConstructStartPkt(char *buf, int *len, unsigned long fileSize, unsigned long crc32);
    void ConstructEndPkt(char *buf, int *len);
    void ConstructResetPkt(char *buf, int *len);
private:
    void addPktTail(char *out, int len);
};

#endif // IAPCMD_H
