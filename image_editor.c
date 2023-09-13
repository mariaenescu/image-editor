//Copyright Maria Enescu 311CAa 2022-2023
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#define N 2000

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} pixel;

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
} img_selection;

typedef struct {
	int type; // 1 - white/black; 2 - grey; 3 - color
	int max_value;
	int height;
	int width;
	pixel *matrix;
} img_color;

void case1(FILE *f, img_color *img, int type)
{
	int number;
	img->type = type;
	// Uncompressed ascii file (B/W)
	for (int i = img->height - 1; i >= 0; i--) {
		for (int j = 0; j < img->width; j++) {
			fscanf(f, "%d", &number);
			img->matrix[i * img->width + j].red = (unsigned char)number;
		}
	}
}

void case5(FILE *f, img_color *img, int type)
{
	img->type = type - 3;
	while (getc(f) != '\n')
		;// Give skip at the end of the line
	for (int i = img->height - 1; i >= 0; i--)
		for (int j = 0; j < img->width; j++)
			img->matrix[i * img->width + j].red = getc(f);
}

void case3(FILE *f, img_color *img)
{
	int number;
	img->type = 3;
	// Uncompressed ascii file (color)
	for (int i = img->height - 1; i >= 0; i--) {
		for (int j = 0; j < img->width; j++) {
			fscanf(f, "%d", &number);
			img->matrix[i * img->width + j].red = (unsigned char)number;
			fscanf(f, "%d", &number);
			img->matrix[i * img->width + j].green = (unsigned char)number;
			fscanf(f, "%d", &number);
			img->matrix[i * img->width + j].blue = (unsigned char)number;
		}
	}
}

int read_file(char *name_file, img_color *img)
{
	FILE *f = fopen(name_file, "r");
	if (!f)
		return 0;
	char character;
	int type;
	character = getc(f);
	if (character != 'P') {
		fclose(f);
		return 0;
	}
	character = getc(f);
	type = character - '0';
	if (type < 0 || type > 6) {
		fclose(f);
		return 0;
	}
	while (getc(f) != '\n')
		; // Give skip at the end of the line
	while (getc(f) == '#') // skip the comments
		while (getc(f) != '\n')
			;
	fseek(f, -1, SEEK_CUR); // moves one position to the left
	fscanf(f, "%d %d", &img->width, &img->height);
	img->max_value = 1;
	if (img->width > N || img->height > N) {
		fclose(f);
		return 0;
	}
	if (type != 1 && type != 4)
		fscanf(f, "%d", &img->max_value);
	img->matrix = malloc(img->width * img->height * sizeof(pixel));
	switch (type) {
	case 2:
	case 1:
	{
		case1(f, img, type);
	}
	break;
	case 4:
	case 5:
	{
		case5(f, img, type);
	}
	break;
	case 3:
	{
		case3(f, img);
	}
	break;
	case 6:
	{
		img->type = 3;
		// Compressed file (color)
		while (getc(f) != '\n')
			; // Give skip at the end of the line
		for (int i = img->height - 1; i >= 0; i--) {
			for (int j = 0; j < img->width; j++) {
				img->matrix[i * img->width + j].red = getc(f);
				img->matrix[i * img->width + j].green = getc(f);
				img->matrix[i * img->width + j].blue = getc(f);
			}
		}
	}
	break;
	}
	fclose(f);
	return 1;
}

void save_file(char *name, img_color *img, int binary)
{
	FILE *fs = fopen(name, "w");
	int type = img->type;
	if (binary == 1)
		type += 3;
	fprintf(fs, "P%d\n", type);
	fprintf(fs, "%d %d\n", img->width, img->height);
	if (type != 4)
		fprintf(fs, "%d\n", img->max_value);
	switch (type) {
	case 1:
	case 2:
		for (int i = img->height - 1; i >= 0; i--) {
			for (int j = 0; j < img->width; j++)
				fprintf(fs, "%d ", img->matrix[i * img->width + j].red);
			fprintf(fs, "\n");
		}
		break;
	case 4:
	case 5:
		for (int i = img->height - 1; i >= 0; i--)
			for (int j = 0; j < img->width; j++)
				putc(img->matrix[i * img->width + j].red, fs);
		fprintf(fs, "\n");
		break;
	case 3:
		for (int i = img->height - 1; i >= 0; i--) {
			for (int j = 0; j < img->width; j++) {
				pixel p = img->matrix[i * img->width + j];
				fprintf(fs, "%d %d %d ", p.red, p.green, p.blue);
			}
			fprintf(fs, "\n");
		}
		break;
	case 6:
		for (int i = img->height - 1; i >= 0; i--) {
			for (int j = 0; j < img->width; j++) {
				pixel p = img->matrix[i * img->width + j];

				putc(p.red, fs);
				putc(p.green, fs);
				putc(p.blue, fs);
			}
		}
		fprintf(fs, "\n");
		break;
	}
	fclose(fs);
}

int clamp(int x, int min, int max)
{
	if (x < min)
		x = min;
	else if (x > max)
		x = max;
	return x;
}

// Function to perform histogram equalisation on an image
void equalisation(img_color *image)
{
	// Declaring 2 arrays for storing histogram values (frequencies) and
	// new gray level values (newly mapped pixel values as per algorithm)
	int hist[256] = {0};
	int new_gray_level[256] = {0};
	for (int i = image->height - 1; i >= 0; i--) {
		for (int j = 0; j < image->width; j++) {
			unsigned char val = image->matrix[i * image->width + j].red;
			hist[val]++;
		}
	}
	// Calculating total number of pixels
	int area = image->width * image->height;
	double curr = 0;
	// Calculating cumulative frequency and new gray levels
	for (int i = 0; i < 256; i++) {
		curr += hist[i]; // cumulative frequency
		// Calculating new gray level after multypelying by maximum gray count
		// which is 255 and dividing by total number of pixels
		new_gray_level[i] = (int)round((curr * 255) / area);
	}
	for (int i = image->height - 1; i >= 0; i--) {
		for (int j = 0; j < image->width; j++) {
			unsigned char val = image->matrix[i * image->width + j].red;
			image->matrix[i * image->width + j].red =
			(unsigned char)clamp(new_gray_level[val], 0, image->max_value);
		}
	}
}

// Function to perform histogram equalisation on an image
void histogram(img_color *image, int x, const int y)
{
	// Declaring array for storing histogram values (frequencies)
	int hist[256] = {0};
	for (int i = image->height - 1; i >= 0; i--) {
		for (int j = 0; j < image->width; j++) {
			unsigned char val = image->matrix[i * image->width + j].red;
			hist[val]++;
		}
	}
	int hist_beans[y];
	memset(hist_beans, 0, y * sizeof(int));
	int max = 0;
	int step = 256 / y;
	// calculating cumulative frequency and new gray levels
	int k = 0;
	for (int i = 0, j = 0; i < 256; i++, j++) {
		if (j == step) {
			j = 0;
			if (max < hist_beans[k])
				max = hist_beans[k];
			k++;
		}
		hist_beans[k] += hist[i];
	}
	if (max < hist_beans[k])
		max = hist_beans[k];
	for (int i = 0; i < y; i++) {
		int n = (int)trunc(hist_beans[i] * 1.0 / max * x);
		printf("%d\t|\t", n);
		for (int j = 0; j < n; j++)
			printf("*");
		printf("\n");
	}
}

void crop(img_color *image, img_selection selection)
{
	int width = selection.x2 - selection.x1;
	int height = selection.y2 - selection.y1;
	pixel *new_image = malloc(width * height * sizeof(pixel));
	for (int i = selection.y1, k = 0; i < selection.y2; i++, k++) {
		for (int j = selection.x1, m = 0; j < selection.x2; ++j, ++m) {
			pixel p = image->matrix[i * image->width + j];
			new_image[k * width + m] = p;
		}
	}
	image->width = width;
	image->height = height;
	free(image->matrix);
	image->matrix = new_image;
}

void apply_filter(img_color *image, int kernel[3][3],
				  double const_kernel, img_selection selection)
{
	pixel *new_image = malloc(image->width * image->height * sizeof(pixel));
	for (int i = image->height - 1; i >= 0; i--) { // rows
		for (int j = 0; j < image->width; ++j) { // columns
			if (i == 0 || i == image->height - 1 || j == 0 ||
				j == image->width - 1 || i < selection.y1 ||
				i >= selection.y2 || j < selection.x1 || j >= selection.x2) {
				new_image[i * image->width + j] =
				image->matrix[i * image->width + j];
			} else {
				int red = 0;
				int green = 0;
				int blue = 0;
				for (int m = 0; m < 3; ++m) { // kernel rows
					for (int n = 0; n < 3; ++n) { // kernel columns
						// index of input signal, used for checking boundary
						int ii = i + (m - 1);
						int jj = j + (n - 1);
						pixel p = image->matrix[ii * image->width + jj];
						red += p.red * kernel[m][n];
						green += p.green * kernel[m][n];
						blue += p.blue * kernel[m][n];
					}
				}
				pixel new_p = new_image[i * image->width + j];
				new_p.red = clamp((int)(const_kernel * red), 0,
								  image->max_value);
				new_p.green = clamp((int)(const_kernel * green), 0,
									image->max_value);
				new_p.blue = clamp((int)(const_kernel * blue), 0,
								   image->max_value);
				new_image[i * image->width + j] = new_p;
			}
		}
	}
	free(image->matrix);
	image->matrix = new_image;
}

pixel *rotate_clock(pixel *image, int rows, int cols)
{
	pixel *new_img = malloc(rows * cols * sizeof(pixel));
	for (int r = rows - 1, i = 0; r >= 0; r--, i++)
		for (int c = 0, j = 0; c < cols; c++, j++)
			new_img[i * cols + j] = image[c * rows + r];
	return new_img;
}

pixel *rotate_anticlock(pixel *image, int rows, int cols)
{
	pixel *new_img = malloc(rows * cols * sizeof(pixel));
	for (int r = 0, i = 0; r < rows; r++, i++)
		for (int c = cols - 1, j = 0; c >= 0; c--, j++)
			new_img[i * cols + j] = image[c * rows + r];
	return new_img;
}

void rotate_img(img_color *image, int angle, img_selection selection)
{
	int width = selection.x2 - selection.x1;
	int height = selection.y2 - selection.y1;
	if (width == height) {
		pixel *new_image = malloc(width * height * sizeof(pixel));
		for (int i = selection.y2 - 1, k = height - 1; i >= selection.y1;
			 i--, k--) {
			for (int j = selection.x1, m = 0; j < selection.x2; ++j, ++m) {
				pixel p = image->matrix[i * image->width + j];
				new_image[k * width + m] = p;
			}
		}
		if (angle > 0)
			for (int i = angle / 90 - 1; i >= 0; i--) {
				pixel *img = rotate_clock(new_image, width, height);
				free(new_image);
				new_image = img;
			}
		else
			for (int i = -angle / 90 - 1; i >= 0; i--) {
				pixel *img = rotate_anticlock(new_image, width, height);
				free(new_image);
				new_image = img;
			}
		for (int i = selection.y2 - 1, k = height - 1; i >= selection.y1;
			 i--, k--) {
			for (int j = selection.x1, m = 0; j < selection.x2; ++j, ++m) {
				pixel p = new_image[k * width + m];
				image->matrix[i * image->width + j] = p;
			}
		}
		free(new_image);
	} else {
		if (angle > 0) {
			for (int i = angle / 90 - 1; i >= 0; i--) {
				pixel *img = rotate_clock(image->matrix, image->width,
										  image->height);
				free(image->matrix);
				image->matrix = img;
				int aux = image->width;
				image->width = image->height;
				image->height = aux;
			}
		} else {
			for (int i = -angle / 90 - 1; i >= 0; i--) {
				pixel *img = rotate_anticlock(image->matrix, image->width,
											  image->height);
				free(image->matrix);
				image->matrix = img;
				int aux = image->width;
				image->width = image->height;
				image->height = aux;
			}
		}
	}
}

char *read_command(void)
{
	char character;
	int i = 0; // counts the number of characters read
	char *rez = malloc(100 * sizeof(char));
	character = getchar();
	while (character != '\n' && character != EOF) {
		rez[i++] = character;
		character = getchar();
	}
	rez[i] = '\0';
	return rez;
}

// This function checks if each character in the given text is a digit or not
int is_number(char *text)
{
	if (!text)
		return 0;
	int size = strlen(text);
	for (int i = 0; i < size; i++)
		if (!isdigit(text[i]) && text[i] != '-')
			return 0;
	return 1;
}

void command_load(img_color **image, img_selection *selection, char *name_file)
{
	if (*image) {
		free((*image)->matrix);
		free(*image);
		selection->x1 = 0;
		selection->y1 = 0;
		selection->x2 = 0;
		selection->y2 = 0;
	}
	*image = malloc(sizeof(img_color));
	int val = read_file(name_file, *image);
	if (val == 1) {
		selection->x1 = 0;
		selection->y1 = 0;
		selection->x2 = (*image)->width;
		selection->y2 = (*image)->height;
		printf("Loaded %s\n", name_file);
	} else {
		printf("Failed to load %s\n", name_file);
		free(*image);
		*image = NULL;
	}
}

void command_select(img_color *image, img_selection *selection,
					char *select_all)
{
	if (!select_all) {
		if (!image)
			printf("No image loaded\n");
		else
			printf("Invalid command\n");
	} else {
		if (strcmp(select_all, "ALL") == 0) {
			if (image) {
				selection->x1 = 0;
				selection->y1 = 0;
				selection->x2 = image->width;
				selection->y2 = image->height;
				printf("Selected ALL\n");
			} else {
				printf("No image loaded\n");
			}
		} else {
			int x1, x2, y1, y2, ok = 1;
			if (!is_number(select_all))
				ok = 0;
			else
				x1 = atoi(select_all);
			char *token = strtok(NULL, " ");
			if (ok) {
				if (!is_number(token))
					ok = 0;
				else
					y1 = atoi(token);
			}
			if (ok) {
				token = strtok(NULL, " ");
				if (!is_number(token))
					ok = 0;
				else
					x2 = atoi(token);
			}
			if (ok) {
				token = strtok(NULL, " ");
				if (!is_number(token))
					ok = 0;
				else
					y2 = atoi(token);
			}
			token = strtok(NULL, " ");
			if (!ok || token) {
				if (!image)
					printf("No image loaded\n");
				else
					printf("Invalid command\n");
			} else {
				if (image) {
					if (x1 >= 0 && x1 <= image->width &&
						x2 >= 0 && x2 <= image->width &&
						y1 >= 0 && y1 <= image->height &&
						y2 >= 0 && y2 <= image->height &&
						x1 != x2 && y1 != y2) {
						int cx1 = x1 < x2 ? x1 : x2, cy1 = y1 < y2 ? y1 : y2;
						int cx2 = x1 > x2 ? x1 : x2, cy2 = y1 > y2 ? y1 : y2;
						printf("Selected %d %d %d %d\n", cx1, cy1, cx2,
							   cy2);
						y1 = image->height - y1;
						y2 = image->height - y2;
						cy1 = y1 < y2 ? y1 : y2;
						cy2 = y1 > y2 ? y1 : y2;
						selection->x1 = cx1;
						selection->y1 = cy1;
						selection->x2 = cx2;
						selection->y2 = cy2;
					} else {
						printf("Invalid set of coordinates\n");
					}
			} else {
				printf("No image loaded\n");
			}
			}
		}
	}
}

void command_equalize(img_color *image)
{
	if (image) {
		if (image->type == 3) {
			printf("Black and white image needed\n");
		} else {
			equalisation(image);
			printf("Equalize done\n");
		}
	} else {
		printf("No image loaded\n");
	}
}

void command_histogram(img_color *image, int ok, char *token)
{
	int x, y;
	if (!is_number(token))
		ok = 0;
	else
		x = atoi(token);
	if (ok) {
		token = strtok(NULL, " ");
		if (!is_number(token))
			ok = 0;
		else
			y = atoi(token);
	}
	token = strtok(NULL, " ");
	if (!ok || token) {
		if (!image)
			printf("No image loaded\n");
		else
			printf("Invalid command\n");
	} else {
		if (image) {
			if (image->type == 3)
				printf("Black and white image needed\n");
			else
				histogram(image, x, y);
		} else {
			printf("No image loaded\n");
		}
	}
}

void command_crop(img_color *image, img_selection *selection)
{
	if (image) {
		if (selection->x1 != 0 || selection->y1 != 0 ||
			selection->x2 != image->width ||
			selection->y2 != image->height) {
			crop(image, *selection);
			selection->x1 = 0;
			selection->y1 = 0;
			selection->x2 = image->width;
			selection->y2 = image->height;
		}
		printf("Image cropped\n");
	} else {
		printf("No image loaded\n");
	}
}

void command_apply(img_color *image, img_selection *selection, char *parameter)
{
	if (!parameter) {
		if (!image)
			printf("No image loaded\n");
		else
			printf("Invalid command\n");
	} else {
		if (image) {
			if (image->type != 3) {
				printf("Easy, Charlie Chaplin\n");
			} else {
				if (strcmp(parameter, "EDGE") == 0) {
					int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1},
										{-1, -1, -1}};
					apply_filter(image, kernel, 1, *selection);
					printf("APPLY EDGE done\n");
				} else if (strcmp(parameter, "SHARPEN") == 0) {
					int kernel[3][3] = {{0, -1, 0}, {-1, 5, -1},
										{0, -1, 0}};
					apply_filter(image, kernel, 1, *selection);
					printf("APPLY SHARPEN done\n");
				} else if (strcmp(parameter, "BLUR") == 0) {
					int kernel[3][3] = {{1, 1, 1}, {1, 1, 1},
										{1, 1, 1}};
					apply_filter(image, kernel, 1.0 / 9, *selection);
					printf("APPLY BLUR done\n");
				} else if (strcmp(parameter, "GAUSSIAN_BLUR") == 0) {
					int kernel[3][3] = {{1, 2, 1}, {2, 4, 2},
										{1, 2, 1}};
					apply_filter(image, kernel, 1.0 / 16, *selection);
					printf("APPLY GAUSSIAN_BLUR done\n");
				} else {
					printf("APPLY parameter invalid\n");
				}
			}
		} else {
			printf("No image loaded\n");
		}
	}
}

void command_rotate(img_color *image, img_selection *selection, int angle)
{
	if (image) {
		if (angle % 90 != 0) {
			printf("Unsupported rotation angle\n");
		} else if (selection->x2 - selection->x1 !=
				   selection->y2 - selection->y1 &&
				   (selection->x1 != 0 || selection->y1 != 0 ||
				   selection->y2 != image->height ||
				   selection->x2 != image->width)) {
			printf("The selection must be square\n");
		} else {
			if (angle != 0 && angle != 360) {
				int all = (selection->x1 == 0 && selection->y1 == 0 &&
						   selection->y2 == image->height &&
						   selection->x2 == image->width);
				rotate_img(image, angle, *selection);
				if (all) {
					selection->x1 = 0;
					selection->y1 = 0;
					selection->x2 = image->width;
					selection->y2 = image->height;
				}
			}
			printf("Rotated %d\n", angle);
		}
	} else {
		printf("No image loaded\n");
	}
}

void command_save(img_color *image, char *file, char *ascii)
{
	if (image) {
		int binary = (ascii && strcmp(ascii, "ascii") == 0) ? 0 : 1;
		save_file(file, image, binary);
		printf("Saved %s\n", file);
	} else {
		printf("No image loaded\n");
	}
}

void command_exit(img_color *image, char *line)
{
	if (image) {
		free(image->matrix);
		free(image);
	}  else {
		printf("No image loaded\n");
	}
	free(line);
}

int verif(char *command)
{
	if (strcmp(command, "LOAD") == 0)
		return 1;
	if (strcmp(command, "SELECT") == 0)
		return 1;
	if (strcmp(command, "EQUALIZE") == 0)
		return 1;
	if (strcmp(command, "HISTOGRAM") == 0)
		return 1;
	if (strcmp(command, "CROP") == 0)
		return 1;
	if (strcmp(command, "SAVE") == 0)
		return 1;
	if (strcmp(command, "EXIT") == 0)
		return 1;
	return 0;
}

int main(void)
{
	img_color *image = NULL;
	img_selection selection;
	while (1) {
		char *line = read_command();
		char *command = strtok(line, " ");
		if (!command)
			continue;
		if (strcmp(command, "LOAD") == 0) {
			char *name_file = strtok(NULL, " ");
			command_load(&image, &selection, name_file);
		} else if (strcmp(command, "SELECT") == 0) {
			char *select_all = strtok(NULL, " ");
			command_select(image, &selection, select_all);
		} else if (strcmp(command, "EQUALIZE") == 0) {
			command_equalize(image);
		} else if (strcmp(command, "HISTOGRAM") == 0) {
			int ok = 1;
			char *token = strtok(NULL, " ");
			command_histogram(image, ok, token);
		} else if (strcmp(command, "CROP") == 0) {
			command_crop(image, &selection);
		} else if (strcmp(command, "APPLY") == 0) {
			char *parameter = strtok(NULL, " ");
			command_apply(image, &selection, parameter);
		} else if (strcmp(command, "SAVE") == 0) {
			char *file = strtok(NULL, " ");
			char *ascii = strtok(NULL, " ");
			command_save(image, file, ascii);
		} else if (strcmp(command, "ROTATE") == 0) {
			int angle = atoi(strtok(NULL, " "));
			command_rotate(image, &selection, angle);
		} else if (strcmp(command, "EXIT") == 0) {
			command_exit(image, line);
			break;
		} else if (verif(command) == 0) {
			printf("Invalid command\n");
		}
		free(line);
	}
	return 0;
}
