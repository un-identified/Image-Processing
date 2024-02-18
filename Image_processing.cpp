#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>

typedef struct {
    int width;
    int height;
    uint8_t* red_channel;
    uint8_t* green_channel;
    uint8_t* blue_channel;
} Image;

Image* readPPM(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char magic[3];
    fscanf(file, "%2s", magic);

    if (strcmp(magic, "P6") != 0) {
        fprintf(stderr, "Invalid PPM file format\n");
        exit(EXIT_FAILURE);
    }

    Image* image = (Image*)malloc(sizeof(Image));

    fscanf(file, "%d %d\n", &image->width, &image->height);
    int maxVal;
    fscanf(file, "%d\n", &maxVal);

    image->red_channel = (uint8_t*)malloc(image->width * image->height * sizeof(uint8_t));
    image->green_channel = (uint8_t*)malloc(image->width * image->height * sizeof(uint8_t));
    image->blue_channel = (uint8_t*)malloc(image->width * image->height * sizeof(uint8_t));

    fread(image->red_channel, sizeof(uint8_t), image->width * image->height, file);
    fread(image->green_channel, sizeof(uint8_t), image->width * image->height, file);
    fread(image->blue_channel, sizeof(uint8_t), image->width * image->height, file);

    fclose(file);
    return image;
}

void writePPM(const char* filename, const Image* image) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "P6\n%d %d\n255\n", image->width, image->height);
    fwrite(image->red_channel, sizeof(uint8_t), image->width * image->height, file);
    fwrite(image->green_channel, sizeof(uint8_t), image->width * image->height, file);
    fwrite(image->blue_channel, sizeof(uint8_t), image->width * image->height, file);

    fclose(file);
}


void increaseBrightness(const Image* inputImage, Image* outputImage, int value) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < inputImage->width * inputImage->height; i++) {
        int new_red = inputImage->red_channel[i] + value;
        int new_green = inputImage->green_channel[i] + value;
        int new_blue = inputImage->blue_channel[i] + value;

        outputImage->red_channel[i] = (new_red > 255) ? 255 : (uint8_t)new_red;
        outputImage->green_channel[i] = (new_green > 255) ? 255 : (uint8_t)new_green;
        outputImage->blue_channel[i] = (new_blue > 255) ? 255 : (uint8_t)new_blue;
    }
}

void increaseSaturation(const Image* inputImage, Image* outputImage, float factor) {
    #pragma omp parallel for
    for (int i = 0; i < inputImage->width * inputImage->height; i++) {
        float red = inputImage->red_channel[i];
        float green = inputImage->green_channel[i];
        float blue = inputImage->blue_channel[i];

        float avg_color = (red + green + blue) / 3.0f; 
        
        float new_red = avg_color + (red - avg_color) * (factor);
        float new_green = avg_color + (green - avg_color) * (factor);
        float new_blue = avg_color + (blue - avg_color) * (factor);

        outputImage->red_channel[i] = (new_red > 255) ? 255 : (uint8_t)new_red;
        outputImage->green_channel[i] = (new_green > 255) ? 255 : (uint8_t)new_green;
        outputImage->blue_channel[i] = (new_blue > 255) ? 255 : (uint8_t)new_blue;
    }
}
void colorInversion(const Image* inputImage, Image* outputImage) {
    #pragma omp parallel for private(val) reduction(+:sum)
    for (int i = 0; i < inputImage->width * inputImage->height; i++) {
        outputImage->red_channel[i] = 255 - inputImage->red_channel[i];
        outputImage->green_channel[i] = 255 - inputImage->green_channel[i];
        outputImage->blue_channel[i] = 255 - inputImage->blue_channel[i];
    }
}
void adjustContrast(const Image* inputImage, Image* outputImage, float contrastFactor) {

    outputImage->width = inputImage->width;
    outputImage->height = inputImage->height;

    outputImage->red_channel = (uint8_t*)malloc(outputImage->width * outputImage->height * sizeof(uint8_t));
    outputImage->green_channel = (uint8_t*)malloc(outputImage->width * outputImage->height * sizeof(uint8_t));
    outputImage->blue_channel = (uint8_t*)malloc(outputImage->width * outputImage->height * sizeof(uint8_t));

    #pragma omp parallel for
    for (int i = 0; i < inputImage->height; i++) {
        for (int j = 0; j < inputImage->width; j++) {
            int idx = i * inputImage->width + j;

            float red = inputImage->red_channel[idx];
            float green = inputImage->green_channel[idx];
            float blue = inputImage->blue_channel[idx];

            float adjustedRed = (red - 128) * contrastFactor + 128;
            float adjustedGreen = (green - 128) * contrastFactor + 128;
            float adjustedBlue = (blue - 128) * contrastFactor + 128;

            outputImage->red_channel[idx] = (adjustedRed < 0) ? 0 : ((adjustedRed > 255) ? 255 : (uint8_t)adjustedRed);
            outputImage->green_channel[idx] = (adjustedGreen < 0) ? 0 : ((adjustedGreen > 255) ? 255 : (uint8_t)adjustedGreen);
            outputImage->blue_channel[idx] = (adjustedBlue < 0) ? 0 : ((adjustedBlue > 255) ? 255 : (uint8_t)adjustedBlue);
        }
    }
}



int main() {
    Image* inputImage = readPPM("input.ppm");
    Image* outputImage = (Image*)malloc(sizeof(Image));
    outputImage->width = inputImage->width;
    outputImage->height = inputImage->height;
    outputImage->red_channel = (uint8_t*)malloc(inputImage->width * inputImage->height * sizeof(uint8_t));
    outputImage->green_channel = (uint8_t*)malloc(inputImage->width * inputImage->height * sizeof(uint8_t));
    outputImage->blue_channel = (uint8_t*)malloc(inputImage->width * inputImage->height * sizeof(uint8_t));
    int choice;
    printf("Choose an operation:\n");
    printf("1. Increase brightness\n");
    printf("2. Increase saturation\n");
    printf("3. Perform color inversion\n");
    printf("4. Increase contrast\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            if(choice == 1) {
                printf("Increasing brightness...\n");
                increaseBrightness(inputImage, outputImage, 50);
            }
        }
        #pragma omp section
        {
            if(choice == 2) {
                printf("Increasing saturation...\n");
                increaseSaturation(inputImage, outputImage, 1.5);
            }
        }

        #pragma omp section
        {
            if(choice == 3) {
                printf("Performing color inversion...\n");
                colorInversion(inputImage, outputImage);
            }
        }

        #pragma omp section
        {
            if(choice == 4) {
                printf("Increasing contrast...\n");
                adjustContrast(inputImage, outputImage, 2);
            }
        }
    }
    #pragma omp master
    {
        writePPM("output.ppm", outputImage);
    }
    return 0;
}
