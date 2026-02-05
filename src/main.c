#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curl/curl.h>
#include <tgmath.h>

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "E926 Imager Puller"

typedef struct ImageData {
    unsigned char *image_data;
    size_t size;
} ImageData;

static size_t readImageByteChunk(unsigned char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t total = nmemb * size;
    ImageData *mem = (ImageData *)clientp;

    unsigned char *ptr = realloc(mem->image_data, mem->size + total);
    if(!ptr)
        return 0;  /* out of memory */

    mem->image_data= ptr;
    memcpy(&(mem->image_data[mem->size]), data, total);
    // or
    // memcpy(mem->response + mem->size, data, realsize);
    mem->size += total;

    return total;
}

void writeTofile(const char *filename, ImageData *chunk) {
    FILE *fp;

    size_t size = chunk->size;

    fp = fopen(filename, "wb");
    if(fp == NULL)
    {
        fprintf(stderr,"Error writing to %s\n",filename);
        return;
    }

    fwrite(chunk->image_data, size, 1, fp);

}


int main(void) {
    ImageData chunk = { 0 };
    CURLcode result;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "e621curl/1.0 (by Moneky on e621)");

        curl_easy_setopt(curl, CURLOPT_URL, "https://static1.e621.net/data/sample/df/cb/dfcb38b6c0cf45d5ad543ce96c5d8bc5.jpg");
        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readImageByteChunk);


        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        /* send a request */
        result = curl_easy_perform(curl);

        /* check for errors */
        if(result != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result));

            curl_easy_cleanup(curl);
        }


        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
        SetTargetFPS(60);


        if (result == CURLE_OK) {
            printf("Success!\n");
        }

        writeTofile("gay.png", &chunk);

        Image image = LoadImageFromMemory(".png", (const unsigned char*) chunk.image_data, (int) chunk.size);
        Texture2D texture = LoadTextureFromImage(image);


        // fit to height or fit to width
        const float factor = fminf((float) SCREEN_WIDTH / (float) texture.width, (float) SCREEN_HEIGHT / (float) texture.height);

        // scale the image to the factor
        const float drawW = (float) texture.width * factor;
        const float drawH = (float) texture.height * factor;

        // center the image
        const float drawX = ((float) GetScreenWidth() - drawW) / 2;
        const float drawY = ((float) GetScreenHeight() - drawH) / 2;

        while (!WindowShouldClose())
        {
            BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexturePro(texture, (Rectangle){0, 0, (float) texture.width, (float) texture.height},   // source
               (Rectangle){drawX, drawY, drawW, drawH},   // destination
               (Vector2){0,0},  // origin
               0.0f, WHITE);

            EndDrawing();
        }


        free(chunk.image_data);
        UnloadTexture(texture);
        CloseWindow();

        return 0;
    }
}
