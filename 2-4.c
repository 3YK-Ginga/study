#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CH 3
#define Ych 0
#define ROW 3
#define COL 3
#define N 2

void get_data(void);
int cal_value(int lsb_index, int bytes);
void processing(void);
void put_data(void);
void rgb_to_ybr(void);
void ybr_to_rgb(void);

unsigned char header[54];
unsigned char imgin[3][512][512];
unsigned char imgout[3][512][512];
unsigned char header_base[54];
unsigned char img_base[3][512][512];
int width, height, alignment, n;

void save_img(void) {
  int i, w, h;
  for(i=0; i<54; i++)
    header_base[i] = header[i];
  for(i=0; i<CH; i++) {
    for(h=0;h<height;h++) {
      for(w=0;w<width;w++)
        img_base[i][w][h] = imgin[i][w][h];
    }
  }
}

void load_img(void) {
  int i, w, h;
  for(i=0; i<54; i++)
    header[i] = header_base[i];
  for(i=0; i<CH; i++) {
    for(h=0;h<height;h++) {
      for(w=0;w<width;w++)
        imgout[i][w][h] = img_base[i][w][h];
    }
  }
  width = cal_value(18, 4);
  height = cal_value(22, 4);
}

void swap_img(void) {
  int i, w, h;
  unsigned char header_tmp[54];
  unsigned char img_tmp[CH][512][512];
  for(i=0; i<54; i++) {
    header_tmp[i] = header_base[i];
    header_base[i] = header[i];
    header[i] = header_tmp[i];
  }
  for(i=0; i<CH; i++) {
    for(h=0;h<height;h++) {
      for(w=0;w<width;w++) {
        img_tmp[i][w][h] = img_base[i][w][h];
        img_base[i][w][h] = imgout[i][w][h];
        imgout[i][w][h] = img_tmp[i][w][h];
      }
    }
  }
  width = cal_value(18, 4);
  height = cal_value(22, 4);
}

int main(void) {
  char text[2][10] = {"注目\0", "参照\0"};

  for(n=0;n<N;n++) {
    printf("<%s画像(入力)>\n", text[n]);
    get_data();
    rgb_to_ybr();
    if(n==0) save_img();
  }

  processing();

  for(n=0;n<N;n++) {
    if(n==0) swap_img();
    else load_img();

    ybr_to_rgb();
    printf("<%s画像(出力)>\n", text[n]);
    put_data();
  }

  return 0;
}

void get_data(void) {
  FILE* fp;
  char name[20];
  int i, h, w;

  printf("入力ファイル名を入力して下さい: ");
  scanf("%s", name);
  fp = fopen(name, "rb");
  if(fp == NULL) {
    printf("ファイルをオープンできません.\n\n");
    exit(1);
  } else printf("ファイルをオープンしました.\n");

  fread(header, 1, 54, fp);

  printf("--- width : ");
  width = cal_value(18, 4);
  printf("%d\n", width);
  printf("--- height: ");
  height = cal_value(22, 4);
  printf("%d\n", height);

  for(h=height-1; h>=0; h--) {
    for(w=0; w<width; w++) {
      for(i=2; i>=0; i--)
        imgin[i][w][h] = fgetc(fp);
    }
  }

  fclose(fp);
  printf("ファイルをクローズしました.\n\n");
}

int cal_value(int lsb_index, int bytes) {
  int i;
  int value = header[lsb_index + bytes - 1];
  for (i = bytes - 2; i >= 0; i--) {
    value <<= 8;
    value += header[lsb_index + i];
  }
  return (value);
}

void processing(void) {
  int i, h, w, x, y, hy, wx, sum=0, min, cw=0, ch=0;

  for(i=0;i<3;i++) {
    for(h=0;h<height;h++) {
      for(w=0;w<width;w++)
        imgout[i][w][h] = imgin[i][w][h];
    }
  }
  printf("<パラメータ入力>\n");
  printf("対象ブロックの左上端画素の位置を入力して下さい\n");
  printf("--- 水平:");
  scanf("%d", &x);
  printf("--- 垂直:");
  scanf("%d", &y);
  printf("\n");
  printf("マッチング処理を開始します\n\n");
  for(h=0;h<height-16;h++) {
    for(w=0;w<width-16;w++) {

      for(hy=0; hy<16; hy++) {
        for(wx=0; wx<16; wx++) {
          sum += abs(img_base[0][x+wx][y+hy] - imgin[0][w+wx][h+hy]);
        }
      }
      if(h == 0 && w == 0)
        min = sum;
      if(sum < min) {
        min = sum;
        cw = w;
        ch = h;
      }
      sum = 0;
    }
  }
  printf("<マッチング結果>\n");
  printf("マッチング位置:(%d,%d)\n", cw, ch);
  printf("マッチング誤差:%d\n\n", min);
  for(h=0; h<16; h++) {
    for(w=0; w<16; w++) {
      if(h == 0 || w == 0 || h == 15 || w == 15) {
        img_base[1][w+x][h+y] = imgout[1][w+cw][h+ch] = 128;
        img_base[2][w+x][h+y] = imgout[2][w+cw][h+ch] = 255;
      }
    }
  }
}

void rgb_to_ybr(void) {
  int i, h, w, ch, itemp;
  double dtemp[CH];
  double rgb_to_ybr[ROW][COL] = {{ 0.2990, 0.5870, 0.1140},
                                 {-0.1687,-0.3313, 0.5000},
                                 { 0.5000,-0.4187,-0.0813}};

  for(h=0; h<height; h++) {
    for(w=0; w<width; w++) {
      for (ch = 0; ch < CH; ch++) {
        dtemp[ch] = 0.0;
        for (i = 0; i < COL; i++)
          dtemp[ch] += rgb_to_ybr[ch][i] * (double)imgin[i][w][h];
      }

      for (ch = 0; ch < CH; ch++) {
        if (dtemp[ch] > 0.0)
          itemp = (int)(dtemp[ch] + 0.5);
        else
          itemp = (int)(dtemp[ch] - 0.5);
        if (ch != Ych)
          itemp += 128;
        if (itemp > 255)
          itemp = 255;
        else if (itemp < 0)
          itemp = 0;
        imgin[ch][w][h] = itemp;
      }
    }
  }
}

void ybr_to_rgb(void) {
  int i, h, w, ch, itemp;
  double dtemp[CH];
  double ybr_to_rgb[ROW][COL] = {{ 1.0000, 0.0000, 1.4020},
                                 { 1.0000,-0.3441,-0.7141},
                                 { 1.0000, 1.7720, 0.0000}};

  for(h=0; h<height; h++) {
    for(w=0; w<width; w++) {
      for (ch = 0; ch < CH; ch++) {
        dtemp[ch] = 0.0;
        for (i = 0; i < COL; i++) {
          if (i == Ych)
            dtemp[ch] += ybr_to_rgb[ch][i] * (double)imgout[i][w][h];
          else
            dtemp[ch] += ybr_to_rgb[ch][i] * (double)(imgout[i][w][h] - 128);
        }
      }

      for (ch = 0; ch < CH; ch++) {
        if (dtemp[ch] > 0.0)
          itemp = (int)(dtemp[ch] + 0.5);
        else
          itemp = (int)(dtemp[ch] - 0.5);
        if (itemp > 255)
          itemp = 255;
        else if (itemp < 0)
          itemp = 0;
        imgout[ch][w][h] = itemp;
      }
    }
  }
}

void put_data(void) {
  int i, w, h;
  FILE* fp;
  char name[20];

  printf("出力ファイル名を入力して下さい: ");
  scanf("%s", name);
  fp = fopen(name, "wb");
  if(fp == NULL) {
    printf("ファイルをオープンできません.\n\n");
    exit(1);
  } else printf("ファイルをオープンしました.\n");

  fwrite(header, 1, 54, fp);

  for(h=height-1; h>=0; h--) {
    for(w=0; w<width; w++) {
      for(i=2; i>=0; i--)
        fputc(imgout[i][w][h], fp);
    }
  }

  for(i=0; i<alignment; i++)
    fputc('\0', fp);

  fclose(fp);
  printf("ファイルをクローズしました.\n\n");
}