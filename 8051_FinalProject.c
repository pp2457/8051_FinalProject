#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int

sbit LATCH1 = P2^2;
sbit LATCH2 = P2^3;
#define DataPort P0

// ? 簡化段碼表：'0'-'9' + 'A'-'Z'
uchar code segtab[91] = {
    // '0'~'9' → ASCII 48~57
    0x3F,0x06,0x5B,0x4F,0x66,
    0x6D,0x7D,0x07,0x7F,0x6F,
    // 'A'~'Z' → ASCII 65~90
    0x77,0x7C,0x39,0x5E,0x79,
    0x71,0x3D,0x76,0x30,0x1E,
    0x75,0x38,0x15,0x54,0x3F,
    0x73,0x67,0x50,0x6D,0x78,
    0x3E,0x1C,0x2A,0x76,0x6E,0x5B
};

// ?? 將字元轉為段碼（支援 0~9、A~Z）
uchar get_seg(char ch) {
    if (ch >= '0' && ch <= '9') return segtab[ch - '0'];
    if (ch >= 'A' && ch <= 'Z') return segtab[ch - 'A' + 10];
    return 0x00;
}

// 位碼
uchar code weimap[8] = {0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};

// 城市名稱（最多 8 字）
char code city_names[8][8] = {
    "TAIPEI", "TOKYO", "SEOUL", "BEIJING",
    "HANOI", "PARIS", "LONDON", "BERLIN"
};

uchar display_buf[8];
uchar city_data[8][8];
uchar city_index = 0;
bit done = 0;

void delay(uint t) {
    uint i,j;
    for(i=0;i<t;i++)
        for(j=0;j<123;j++);
}

void display() {
    uchar i;
    for(i=0;i<8;i++) {
        DataPort = 0x00; LATCH1=1; LATCH1=0;
        DataPort = weimap[i]; LATCH2=1; LATCH2=0;
        DataPort = display_buf[i]; LATCH1=1; LATCH1=0;
        delay(1);
    }
}

// ? 顯示城市名稱
void show_city_name(uchar id) {
    uchar i;
    for(i = 0; i < 8; i++) {
        char ch = city_names[id][i];
        if(ch == '\0') break;
        display_buf[i] = get_seg(ch);
    }
    for(; i < 8; i++) display_buf[i] = 0x00;
}

// ? 顯示資料 GHHMMTT（共 7 字）
void show_data(uchar id) {
    display_buf[0] = get_seg('G');
    display_buf[1] = get_seg(city_data[id][0]);
    display_buf[2] = get_seg(city_data[id][1]);
    display_buf[3] = get_seg(city_data[id][3]);
    display_buf[4] = get_seg(city_data[id][4]);
    display_buf[5] = 0x00;
    display_buf[6] = get_seg(city_data[id][6]);
    display_buf[7] = get_seg(city_data[id][7]);
}

// ? 顯示 DONE
void show_done() {
    display_buf[0] = get_seg('D');
    display_buf[1] = get_seg('O');
    display_buf[2] = get_seg('N');
    display_buf[3] = get_seg('E');
    display_buf[4] = 0x00;
    display_buf[5] = 0x00;
    display_buf[6] = 0x00;
    display_buf[7] = 0x00;
}

// ? 串口中斷接收
void uart_isr() interrupt 4 {
    static uchar idx = 0;
    static uchar buffer[8];
    uchar i;

    if (RI) {
        uchar ch = SBUF;
        RI = 0;

        if (ch == '\n') {
            if(idx == 8 && city_index < 8) {
                for(i = 0; i < 8; i++)
                    city_data[city_index][i] = buffer[i];
                city_index++;
            } else if (idx == 4 && buffer[0] == 'D' && buffer[1] == 'O') {
                done = 1;
                show_done();
            }
            idx = 0;
        } else {
            if(idx < 8)
                buffer[idx++] = ch;
        }
    }
}

// ? 按鍵處理 P1.0 ~ P1.7
void check_key() {
    uchar i;
    uint t;
    for(i=0;i<8;i++) {
        if((P1 & (1 << i)) == 0) {
            delay(5);
            if((P1 & (1 << i)) == 0) {
                show_city_name(i);
                for(t=0;t<200;t++) display();  // 顯示城市名 2 秒
                show_data(i);
                while((P1 & (1 << i)) == 0);
            }
        }
    }
}

// ? 主程式
void main() {
    TMOD = 0x20;
    TH1 = TL1 = 0xFD;
    TR1 = 1;
    SCON = 0x50;
    ES = 1;
    EA = 1;

    while(1) {
        check_key();
        display();
    }
}
