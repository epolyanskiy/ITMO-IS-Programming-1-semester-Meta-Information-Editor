#include <stdio.h>
#include <math.h>
#include <string.h>
#include "metadata.h"


int get_frame_size(char *byteArr) {
    int size = 0;
    unsigned char ch;
    for (int i = 0; i < 4; i++) {
        ch = byteArr[i];
        size += ch * pow(2, 7 * (3 - i));
    }
    return size;
}

void set_frame_size(char *frameSize, int valSize) {
    for (int i = 0; i < 4; i++) {
        frameSize[i] = valSize / pow(2, 7 * (3 - i));
        valSize -= frameSize[i] * pow(2, 7 * (3 - i));
    }
}

void show_all(FILE *f) {
    fseek(f, 0, SEEK_SET);
    struct MainHeader header;
    fread(&header, sizeof(header), 1, f);
    long long tag_size = get_frame_size(header.frame_size);
    struct Frame frame;
    long long counter = 0;
    while (counter <= tag_size) {
        fread(&frame, sizeof(frame), 1, f);
        long long frameSize = get_frame_size(frame.frame_size);
        counter += 10 + frameSize;
        printf("---------------------");
        printf("%s ", frame.frame_ID);
        while (frameSize--) {
            char ch = fgetc(f);
            if (ch >= 32 && ch <= 126)
                printf("%c", ch);
        }
        printf("\n");
    }
}

void show_one(FILE *f, char *frame_ID) {
    fseek(f, 0, SEEK_SET);
    struct MainHeader header;
    fread(&header, sizeof(header), 1, f);
    int fileSize = get_frame_size(header.frame_size);
    struct Frame frame;
    int counter = 0;
    while (counter <= fileSize) {
        fread(&frame, sizeof(frame), 1, f);
        int frameSize = get_frame_size(frame.frame_size);
        counter += 10 + frameSize;
        if (strcmp(frame.frame_ID, frame_ID) == 0) {
            printf("%s ", frame.frame_ID);
            while (frameSize--) {
                char ch = fgetc(f);
                if (ch >= 32 && ch <= 126)
                    printf("%c", ch);
            }
        } else {
            fseek(f, get_frame_size(frame.frame_size), SEEK_CUR);
        }
    }
}

void set_value(FILE *fin, FILE *fout, char *someFrame, char *val) {
    fseek(fin, 0, SEEK_SET);
    struct MainHeader header;
    fread(&header, sizeof(header), 1, fin);
    fwrite(&header, sizeof(header), 1, fout);
    int valSize = strlen(val);
    int fileSize = get_frame_size(header.frame_size);
    struct Frame frame;
    int counter = 0;
    int frameSize = 0;
    while (counter <= fileSize) {
        fread(&frame, sizeof(frame), 1, fin);
        frameSize = get_frame_size(frame.frame_size);
        counter += 10 + frameSize;
        if (strcmp(frame.frame_ID, someFrame) != 0 && frameSize >= 0 && frameSize <= 100000) {
            fwrite(&frame, 1, sizeof(frame), fout);
            char info[frameSize];
            fread(&info, frameSize, 1, fin);
            fwrite(info, frameSize, 1, fout);
        } else if (strcmp(frame.frame_ID, someFrame) == 0 && frameSize >= 0) {
            int oldFrameSize = frameSize;
            set_frame_size(header.frame_size, get_frame_size(header.frame_size) - frameSize + valSize);
            int curPos = ftell(fout);
            fseek(fout, 6, SEEK_SET);
            fwrite(header.frame_size, sizeof(header.frame_size), 1, fout);
            fseek(fout, curPos, SEEK_SET);
            set_frame_size(frame.frame_size, valSize);
            int newFrameSize = get_frame_size(frame.frame_size);
            fwrite(&frame, sizeof(frame), 1, fout);
            fseek(fin, oldFrameSize, SEEK_CUR);
            fwrite(val, valSize, 1, fout);
        }
    }
    int ch;
    while ((ch = fgetc(fin)) != EOF) {
        fputc(ch, fout);
    }
    fseek(fout, 0, SEEK_SET);
    fseek(fin, 0, SEEK_SET);
    while ((ch = getc(fout)) != EOF) {
        fputc(ch, fin);
    }
}

//main.exe --filepath=music.mp3 --show
//main.exe --filepath=music.mp3 --get=TIT2
//main.exe --filepath=music.mp3 --set=COMM --value=Test
int main(int argc, char *argv[]) {
    char task[3][10];
    char argument[3][50];
    int k = 0;
    int h = 0;
    int j = 0;

    for (int i = 1; i < argc; i++) {
        k = 0;
        for (j = 0; argv[i][j] != '=' && argv[i][j] != '\0'; j++) {
            task[h][k++] = argv[i][j];
        }
        task[h][k] = '\0';
        j++;
        k = 0;
        while (argv[i][j] != '\0') {
            argument[h][k++] = argv[i][j++];
        }
        argument[h][k] = '\0';
        h++;
    }

    FILE *mp3_in = fopen(argument[0], "r+b");
    FILE *mp3_out = fopen("mp3_out_file.mp3", "r+b");
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(task[i], "--show") == 0) {
            show_all(mp3_in);
        } else if (strcmp(task[i], "--get") == 0) {
            show_one(mp3_in, argument[i]);
        } else if (strcmp(task[i], "--set") == 0) {
            set_value(mp3_in, mp3_out, argument[i], argument[i + 1]);
        }
    }
    fclose(mp3_in);
    fclose(mp3_out);
    return 0;
}