#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>


int TERM_HEIGHT = 24;
int TERM_WIDTH = 80;

char *screen = NULL;
float *zbuffer = NULL;

typedef struct {
    float x, y, z;
} Vertex;

const float PI = 3.141592653589793f;


void getTerminalSize(int *width, int *height);
void initBuffers();
void freeBuffers();
void clearBuffer();
Vertex SpherePoint(float r, float u, float v);
void render(int frame_count);
void rotateY(Vertex *point, float angle);
void drawSphere(float r, float rotation_angle);

void getTerminalSize(int *width, int *height) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    *height = w.ws_row - 2;
    *width = w.ws_col;
    
    if (*height < 10) *height = 10;
    if (*width < 20) *width = 20;
}

void initBuffers() {
    getTerminalSize(&TERM_WIDTH, &TERM_HEIGHT);
    screen = malloc(TERM_HEIGHT * TERM_WIDTH * sizeof(char));
    zbuffer = malloc(TERM_HEIGHT * TERM_WIDTH * sizeof(float));
    clearBuffer();
}

void freeBuffers() {
    free(screen);
    free(zbuffer);
}

void clearBuffer() {
    if (screen && zbuffer) {
        memset(screen, ' ', TERM_HEIGHT * TERM_WIDTH);
        for (int i = 0; i < TERM_HEIGHT * TERM_WIDTH; i++) {
            zbuffer[i] = -1000.0f;
        }
    }
}

Vertex SpherePoint(float r, float u, float v) {
    Vertex P;
    P.x = r * cos(u) * cos(v);
    P.y = r * cos(u) * sin(v);
    P.z = r * sin(u);
    return P;
}

void render(int frame_count) {
    printf("Frame: %04d | Terminal: %dx%d | Ctrl+C to exit | Hamza Libah\n", 
           frame_count, TERM_WIDTH, TERM_HEIGHT);
    
    for (int y = 0; y < TERM_HEIGHT; y++) {
        for (int x = 0; x < TERM_WIDTH; x++) {
            putchar(screen[y * TERM_WIDTH + x]);
        }
        putchar('\n');
    }
    fflush(stdout);
}

void rotateY(Vertex *point, float angle) {
    float new_x = point->x * cos(angle) + point->z * sin(angle);
    float new_z = -point->x * sin(angle) + point->z * cos(angle);
    point->x = new_x;
    point->z = new_z;
}

void drawSphere(float r, float rotation_angle) {
    const int u_steps = 20;
    const int v_steps = 40;
    
   
    float auto_radius = (TERM_HEIGHT < TERM_WIDTH ? TERM_HEIGHT : TERM_WIDTH) * 0.4f;
    if (r <= 0) r = auto_radius;
    
    
    const float aspect_ratio = 2.0f;  
    
    for (int i = 0; i <= u_steps; i++) {
        float u = -PI/2 + PI * i / u_steps;
        
        for (int j = 0; j <= v_steps; j++) {
            float v = 2 * PI * j / v_steps;
            
            Vertex P = SpherePoint(r, u, v);
            rotateY(&P, rotation_angle);
            
         
            P.x *= aspect_ratio;  
            
            float distance = r * 2.5f;
            float scale = (TERM_HEIGHT + TERM_WIDTH) / 4.0f;
            
            int screenx = (int)(TERM_WIDTH/2 + (P.x * scale) / (P.z + distance));
            int screeny = (int)(TERM_HEIGHT/2 - (P.y * scale) / (P.z + distance));
            
            if (screenx >= 0 && screenx < TERM_WIDTH && screeny >= 0 && screeny < TERM_HEIGHT) {
                int index = screeny * TERM_WIDTH + screenx;
                if (P.z > zbuffer[index]) {
                    zbuffer[index] = P.z;
                    
                    float Lx = 0.707f, Ly = 0.5f, Lz = 0.5f;
                    float Nx = cos(u) * cos(v);
                    float Ny = cos(u) * sin(v);
                    float Nz = sin(u);
                    
                    Vertex normal = {Nx, Ny, Nz};
                    rotateY(&normal, rotation_angle);
                    
                    float intensity = normal.x * Lx + normal.y * Ly + normal.z * Lz;
                    intensity = (intensity + 1.0f) / 2.0f;
                    
                    int lum = (int)(intensity * 8);
                    if (lum < 0) lum = 0;
                    if (lum > 8) lum = 8;
                    
                    char shades[] = " hamza ,.-"; 
                    screen[index] = shades[lum];
                }
            }
        }
    }
}

int main() {
    float rotation_angle = 0.0f;
    int frame_count = 0;
    
    initBuffers();
    
    printf(" 3D sphere using my name and characters \n");
 
    usleep(2000000);
    
    int last_width = TERM_WIDTH;
    int last_height = TERM_HEIGHT;
    
    while(1) {
        int current_width, current_height;
        getTerminalSize(&current_width, &current_height);
        
        if (current_width != last_width || current_height != last_height) {
            printf("Terminal resized: %dx%d -> %dx%d\n", 
                   last_width, last_height, current_width, current_height);
            freeBuffers();
            TERM_WIDTH = current_width;
            TERM_HEIGHT = current_height;
            initBuffers();
            last_width = current_width;
            last_height = current_height;
        }
        
        clearBuffer();
        drawSphere(0, rotation_angle);
        render(frame_count);
        
        rotation_angle += 0.03f;
        if (rotation_angle > 2 * PI) rotation_angle -= 2 * PI;
        
        frame_count++;
        usleep(100000);
    }
    
    freeBuffers();
    return 0;
}