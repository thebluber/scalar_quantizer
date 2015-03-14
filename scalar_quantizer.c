#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

struct Data {
  char magic_num[3];
  int width;
  int height;
  int max_gray;
  int *reconst_levels;
  int *pixels;
};

void die(const char *message) {
  if(errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }
  exit(1);
}

void quantize() {

}

//Helper function to stop repeating code
int get_int(FILE *file) {
  int i = 0;
  char chr = fgetc(file);
  char tmp[4] = {0};
  while(chr == '\n' || chr == ' ') {
    chr = fgetc(file);
  }

  while(chr != '\n' && chr != ' ') {
    tmp[i] = chr;
    i++;
    chr = fgetc(file);
  }
  return atoi(tmp);
}

struct Data *init(FILE *file) {
  struct Data *data = malloc(sizeof(struct Data));
  if(!data) die("Memory Error!");

  int chr, i = 0;
  //magic number
  while((chr = fgetc(file)) != '\n') {
    data->magic_num[i] = chr;
    i++;
  }
  data->magic_num[2] = '\0';
  printf("%s\n", data->magic_num);

  //comments
  chr = fgetc(file);
  if(chr == '#') {
    while(chr != '\n') {
      chr = fgetc(file);
    }
  }

  //width
  data->width = get_int(file);
  printf("%d\n", data->width);

  //height
  data->height = get_int(file);
  printf("%d\n", data->height);

  //max_gray
  data->max_gray = get_int(file);
  printf("%d\n", data->max_gray);

  data->pixels = malloc(data->width * data->height * sizeof(int));
  if(!data->pixels) die("Memory Error!");
  return data;
}

//Helper function for a given sample value to find its level
int find_level(int boundaries[], int val, int level) {
  if(val == boundaries[level]) return level - 1; //val = b_max

  int i;
  for(i = 0; i < level; i++) {
    if(val >= boundaries[i] && val < boundaries[i + 1]) break;
  }
  return i;
}

void quant(struct Data *data, FILE *file, int level) {
  int stepsize = (data->max_gray + 1) / level;
  printf("Stepsize: %d\n", stepsize);

  printf("Boundaries: \n");
  //Store interval boundaries
  int boundaries[level + 1];
  int boundary = 0, i = 0;
  while(i < level) {
    boundaries[i] = boundary;
    printf("b%d: %d\n", i, boundary);
    i++;
    boundary += stepsize;
  }
  boundaries[level] = data->max_gray;

  printf("Quantize data:\n");

  //Choose middle points of the interval as representatives
  printf("Recontruction levels: ");
  data->reconst_levels = malloc(level * sizeof(int));
  if(!data->reconst_levels) die("Memory Error!");
  for(i = 0; i < level; i++) {
    data->reconst_levels[i] = (boundaries[i] + boundaries[i + 1]) / 2;
    printf("%d:%d ", i, data->reconst_levels[i]);
  }
  printf("\n");

  //Store compressed pixels
  printf("Compressed pixels:\n");
  for(i = 0; i < data->width * data->height; i++) {
    int val = get_int(file);
    data->pixels[i] = find_level(boundaries, val, level);
    printf(" %d", data->pixels[i]);
    if(i != 0 && (i + 1) % data->width == 0) printf("\n");
  }

}

void decompress(struct Data *data, const char *output_filename) {
  FILE *f = fopen(output_filename, "w");
  //Write header
  fprintf(f, "%s\n", data->magic_num);
  fprintf(f, "%d %d\n", data->width, data->height);
  fprintf(f, "%d\n", data->max_gray);

  int i;
  for(i = 0; i < data->width * data->height; i++) {
    fprintf(f, " %d", data->reconst_levels[data->pixels[i]]);
    if(i != 0 && (i + 1) % data->width == 0)
      fprintf(f, "\n");
  }
  fclose(f);
}

int main(int argc, char *argv[]) {
  if(argc < 3) die("USAGE: scalar_quantizer <input_file.pgm> <output_file.pgm> [level=4]");
  char *filename_in = argv[1];
  char *filename_out = argv[2];
  int level = 4;
  if(argv[3]) level = atoi(argv[3]);
  FILE *file = fopen(filename_in, "r");
  struct Data *data = init(file);
  quant(data, file, level);
  decompress(data, filename_out);

  return 0;
}
