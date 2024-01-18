#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <math.h>
#include "Joueur.h"

#define WINDOW_TITLE "Raycaster in C"
#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 480
#define PI 3.1415f

// on raccourcit les noms des fonctions
#define BLACK_COLOR(x) SDL_SetRenderDrawColor(x, 0, 0, 0, 255)
#define WHITE_COLOR(x) SDL_SetRenderDrawColor(x, 255, 255, 255, 255)
#define RED_COLOR(x) SDL_SetRenderDrawColor(x, 255, 0, 0, 255)
#define GREEN_COLOR(x) SDL_SetRenderDrawColor(x, 0, 255, 0, 255)
#define BLUE_COLOR(x) SDL_SetRenderDrawColor(x, 0, 0, 255, 255)
#define YELLOW_COLOR(x) SDL_SetRenderDrawColor(x, 255, 255, 0, 255)
#define PURPLE_COLOR(x) SDL_SetRenderDrawColor(x, 255, 0, 255, 255)
#define CYAN_COLOR(x) SDL_SetRenderDrawColor(x, 0, 255, 255, 255)
#define ORANGE_COLOR(x) SDL_SetRenderDrawColor(x, 255, 128, 0, 255)
#define GRAY_COLOR(x) SDL_SetRenderDrawColor(x, 128, 128, 128, 255)
#define BROWN_COLOR(x) SDL_SetRenderDrawColor(x, 128, 64, 0, 255)
#define PINK_COLOR(x) SDL_SetRenderDrawColor(x, 255, 128, 128, 255)

#define DEG_TO_RAD(x) x*(PI/180) 

#define DELAY_FRAMES 30
#define MAX_MUR 128

#define JOUEUR_SPAWN_X 50
#define JOUEUR_SPAWN_Y 50
#define JOUEUR_SPAWN_DIR 0
#define JOUEUR_SPEED 5 // nombre de pixels que le joueur peut parcourir par frame
#define JOUEUR_FOV 50 // champ de vision du joueur en degrés
#define JOUEUR_VUE_DIST 1000 // distance maximale que le joueur peut voir


void controle(SDL_Event event,Joueur *joueur, int *ouvert){
    while(SDL_PollEvent(&event) != 0){
        switch(event.type){
            case SDL_QUIT:
                *ouvert=0;
                break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                    case SDLK_UP:
                        if(joueur->y>0)joueur->y-=JOUEUR_SPEED;
                        break;
                    case SDLK_DOWN:
                        if(joueur->y<WINDOW_HEIGHT-JOUEUR_SPEED)joueur->y+=JOUEUR_SPEED;
                        break;
                    case SDLK_LEFT:
                        if(joueur->x>0)joueur->x-=JOUEUR_SPEED;
                        break;
                    case SDLK_RIGHT:
                        if(joueur->x<WINDOW_WIDTH-JOUEUR_SPEED)joueur->x+=JOUEUR_SPEED;
                        break;
                    case SDLK_q:
                        joueur->dir+=JOUEUR_SPEED;
                        break;
                    case SDLK_d:
                        joueur->dir-=JOUEUR_SPEED;
                        break;
                    case SDLK_ESCAPE:
                        *ouvert=0;
                        break;
                }
                break;
        }
    }
}

// renvoie un point à une distance donnée d'un point donné dans une direction donnée
SDL_Point creerPoint(int x, int y, int direction, int distance){
    SDL_Point point;

    double directionRad=DEG_TO_RAD(direction);

    point.x=x+distance*cos(directionRad);
    point.y=y+distance*sin(directionRad);
    return point;
} // creerPoint

// renvoie 1 si le point est sur la ligne, 0 sinon
int pointSurLigne(int x1, int x2, int tx, int y1, int y2, int ty){
    return fmin(x1,x2)<=tx && tx<=fmax(x1,x2) && fmin(y1,y2)<=ty && ty<=fmax(y1,y2);
} // pointSurLigne

// calcule la distance entre deux points
int calculDistance(int x1, int x2, int y1, int y2){
    return sqrt(pow(x2-x1,2)+pow(y2-y1,2));
} // calculDistance

void dessineJoueur(SDL_Renderer *renderer, Joueur joueur){
    RED_COLOR(renderer);
    SDL_RenderDrawPoint(renderer,joueur.x,joueur.y);
    printf("x=%d y=%d dir=%d\n",joueur.x,joueur.y,joueur.dir);
    SDL_Point point=creerPoint(joueur.x,joueur.y,joueur.dir,10);
    SDL_RenderDrawLine(renderer,joueur.x,joueur.y,point.x,point.y);
} // dessineJoueur

// dessine les murs avec le tableau de murs
void dessineMurs(SDL_Renderer *renderer, SDL_Point murs[MAX_MUR][2], int nbMurs){
    WHITE_COLOR(renderer); // on dessine les murs en blanc
    int i;
    for(i=0;i<nbMurs;i++){
        SDL_RenderDrawLine(renderer,murs[i][0].x,murs[i][0].y,murs[i][1].x,murs[i][1].y);
    }
} // dessineMurs

void dessineRayons(SDL_Renderer *renderer, Joueur joueur, SDL_Point murs[MAX_MUR][2], int nbMurs){
    SDL_Point cible;
    SDL_Point point;
    SDL_Rect rectangle; // utilisé pour la vue 3D

    int largeurRayon=WINDOW_WIDTH*0.66/JOUEUR_FOV;
    int nbRayons=0;
    int distance=0; // distance la plus proche
    rectangle.w=largeurRayon; // largeur du rectangle

    // on parcours chaque rayon de le champ de vision du joueur
    for(int i=joueur.dir - JOUEUR_FOV/2; i < joueur.dir + JOUEUR_FOV/2; i++){
        nbRayons++;
        cible=creerPoint(joueur.x,joueur.y,i,JOUEUR_VUE_DIST);
        point=cible;
        distance=JOUEUR_VUE_DIST;

        // on parcours chaque mur
        for(int j=0;j<nbMurs;j++){
            float a1=cible.y-joueur.y;
            float b1=joueur.x-cible.x;
            float c1=a1*joueur.x+b1*joueur.y;
            float a2=murs[j][1].y-murs[j][0].y;
            float b2=murs[j][0].x-murs[j][1].x;
            float c2=a2*murs[j][0].x+b2*murs[j][0].y;
            float determinant=a1*b2-a2*b1; // déterminant du système

            if(determinant!=0){
                int x=(b2*c1-b1*c2)/determinant;
                int y=(a1*c2-a2*c1)/determinant;
                if(pointSurLigne(joueur.x, joueur.y,cible.x,cible.y,x,y) && pointSurLigne(murs[j][0].x,murs[j][0].y, murs[j][1].x,murs[j][1].y,x,y)){
                    int dist=calculDistance(joueur.x,joueur.y,x,y);
                    if(dist<distance){
                        point.x=x;
                        point.y=y;
                        distance=dist;
                    }
                }
            }

        }
        // si le rayon a touché un mur
        if(distance!=JOUEUR_VUE_DIST){
            BLUE_COLOR(renderer);
            SDL_RenderDrawLine(renderer,joueur.x,joueur.y,point.x,point.y);
            // on dessine la vue 3D
            // rectangle.x=largeurRayon*nbRayons+WINDOW_WIDTH/3;
            // rectangle.y=distance/2;
            // rectangle.h=WINDOW_HEIGHT-rectangle.y*2;
            int luminosite=0;
            if(distance/2<255) luminosite=255-distance/2;

            SDL_SetRenderDrawColor(renderer,luminosite,luminosite,luminosite,255);
            SDL_RenderFillRect(renderer,&rectangle);

        }

    }

}



int main(int argc, char const *argv[])
{
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    Joueur joueur=(Joueur){.x=JOUEUR_SPAWN_X,.y=JOUEUR_SPAWN_Y,.dir=JOUEUR_SPAWN_DIR};
    SDL_Point murs[MAX_MUR][2]={
        {{50, 50}, {100, 50}},
        {{100, 50}, {150, 100}},
        {{150, 100}, {200, 200}},
        {{200, 200}, {200, 300}},
        {{200,300},{600,300}},
        {{600,300},{600,100}},
        {{600,100},{50,50}}
    };

    int ouvert=1;

    // ----------- initialisation de la SDL -----------
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        fprintf(stderr,"Erreur SDL_Init : %s",SDL_GetError());
        return EXIT_FAILURE;
    }

    if(SDL_CreateWindowAndRenderer(WINDOW_WIDTH,WINDOW_HEIGHT,0,&window,&renderer) != 0){
        fprintf(stderr,"Erreur SDL_CreateWindowAndRenderer : %s",SDL_GetError());
        return EXIT_FAILURE;
    }
    // -------------------------------------------------

    while(ouvert){
        // ----------- gestion des évènements -----------
        controle(event,&joueur, &ouvert);
        // ----------- dessin -----------
        dessineMurs(renderer,murs,9);
        dessineRayons(renderer,joueur,murs,9);
        dessineJoueur(renderer,joueur);
        // on actualise le rendu
        BLACK_COLOR(renderer);
        SDL_RenderPresent(renderer);
        SDL_RenderClear(renderer);
        // on attend un peu
        SDL_Delay(DELAY_FRAMES);
    }

    // ----------- fermeture de la SDL -----------
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
