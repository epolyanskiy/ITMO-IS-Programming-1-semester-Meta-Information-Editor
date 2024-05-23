#ifndef OP_LABORATORY13_METADATA_H
#define OP_LABORATORY13_METADATA_H
struct Frame {
    char frame_ID[4];
    unsigned char frame_size[4];
    char frame_flags[2];
};

struct MainHeader {
    char ID3[3];
    char version[2];
    char flag;
    char frame_size[4];
};

#endif //OP_LABORATORY13_METADATA_H