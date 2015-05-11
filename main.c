#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void input(short*, short*, short*, short*);
void init(short**, char***, short***, short***, short, short, short);
void play(short**, char***, short***, short***, short, short, short, short);
short canPlay(short, char**, short**, short, short);
short canPlace(short, char**, short**, short, short, short, short);
void printMap(short**, char**, short);
short generateValue(short, char**, short**, short, short);

int main(int argc, char** argv)
{
    short i;
    short nbPlayers = 0;
    short nbIA = 0;
    short gridSize;
    short gamemode; //0 = octopus, 1 = snake
    short *scores;
    char **positions;
    short **lastPos, **grid;
    
    input(&nbPlayers, &nbIA, &gridSize, &gamemode);
    init(&scores, &positions, &lastPos, &grid, nbPlayers, nbIA, gridSize);
    play(&scores, &positions, &lastPos, &grid, nbPlayers, nbIA, gridSize, gamemode);

#ifdef WIN32
    system("cls");
#endif
#ifdef linux
    system("clear");
#endif
    //Affichage des scores
    for(i=0 ; i < (nbPlayers+nbIA) ; i++)
        printf("Score du joueur %hi : %hi\n", i+1, scores[i]);
    
#ifdef WIN32
    system("pause");
#endif
#ifdef linux
    system("read");
#endif  

    //Désallocation
    for(i=0 ; i < gridSize ; i++)
    {
        free(grid[i]);
        free(positions[i]);
    }
    for(i=0 ; i < (nbPlayers+nbIA) ; i++)
        free(lastPos[i]);
    free(grid);
    free(positions);
    free(scores);
    free(lastPos);
    return EXIT_SUCCESS;
}

void input(short *nbPlayers, short *nbIA, short *gridSize, short *gamemode)
{
    do
    {
        printf("Saisir le nombre de joueurs (2 à 4) : ");
        scanf("%hi", nbPlayers);
    } while(*nbPlayers < 2 || *nbPlayers > 4);
    
    /*do
    {
        printf("Saisir le nombre d'IA (0 à 4-nbPlayers) : ");
        scanf("%hi", nbIA);
    } while(*nbIA < 0 || *nbIA > 4-(*nbPlayers));*/
    
    do
    {
        printf("Saisir la taille de la grille (5 à 26) : ");
        scanf("%hi", gridSize);
    } while(*gridSize < 5 || *gridSize > 26);
    
    do
    {
        printf("Choisir le mode de jeu (0 : pieuvre, 1 : serpent) : ");
        scanf("%hi", gamemode);
    } while(*gamemode < 0 || *gamemode > 1);
}

void init(short **scores, char ***positions, short ***lastPos, short ***grid, short nbPlayers, short nbIA, short gridSize)
{
    short i, j;
    
    //Allocation des grilles
    *scores = (short*)malloc((nbPlayers+nbIA)*sizeof(short));
    *positions = (char**)malloc(gridSize*sizeof(char*));
    *lastPos = (short**)malloc((nbPlayers+nbIA)*sizeof(short*));
    *grid = (short**)malloc(gridSize*sizeof(short*));
    for(i=0 ; i < gridSize ; i++)
    {
        (*grid)[i] = (short*)malloc(gridSize*sizeof(short));
        (*positions)[i] = (char*)malloc(gridSize*sizeof(char));

        //Initialisation de toutes les positions avec le caractère null
        for(j=0 ; j < gridSize ; j++)
            (*positions)[i][j] = '\0';
    }
    //Assignation des valeurs de la map
    for(i=0 ; i < gridSize/2+1 ; i++)
    {
        for(j=0 ; j < gridSize/2+1 ; j++)
        {            
            if((nbPlayers+nbIA) == 2)
            {
                (*grid)[i][j] = i+j;
                (*grid)[i][gridSize-j-1] = gridSize-1-i-j;
                (*grid)[gridSize-i-1][gridSize-j-1] = i+j;
                (*grid)[gridSize-i-1][j] = gridSize-1-i-j;
            }
            else if((nbPlayers+nbIA) == 3)
            {
                (*grid)[i][j] = i+j;
                (*grid)[gridSize-i-1][gridSize-j-1] = i+j;
                (*grid)[i][gridSize-j-1] = i+j;
                (*grid)[gridSize-i-1][j] = gridSize-1-i-j;
            }
            else if((nbPlayers+nbIA) == 4)
            {
                (*grid)[i][j] = i+j;
                (*grid)[gridSize-i-1][gridSize-j-1] = i+j;
                (*grid)[i][gridSize-j-1] = i+j;
                (*grid)[gridSize-i-1][j] = i+j;
            }
        }
    }
    
    //Initialisation des scores à 0 et allocation des positions
    for(i=0 ; i < (nbPlayers+nbIA) ; i++)
    {
        (*scores)[i] = 0;
        (*lastPos)[i] = (short*)malloc(2*sizeof(short)); // 2 -> row, col
    }
    //Placement des joueurs
    switch((nbPlayers+nbIA))
    {
        case 4:
            (*positions)[gridSize-1][0] = 'b';
            (*lastPos)[3][0] = gridSize-1;
            (*lastPos)[3][1] = 0;
        case 3:
            (*positions)[0][gridSize-1] = 'v';
            (*lastPos)[2][0] = 0;
            (*lastPos)[2][1] = gridSize-1;
        case 2:
            (*positions)[0][0] = 'r';
            (*lastPos)[0][0] = 0;
            (*lastPos)[0][1] = 0;

            (*positions)[gridSize-1][gridSize-1] = 'j';
            (*lastPos)[1][0] = gridSize-1;
            (*lastPos)[1][1] = gridSize-1;
            break;
    }
}

void play(short **scores, char ***positions, short ***lastPos, short ***grid, short nbPlayers, short nbIA, short gridSize, short gamemode)
{
    short loop = 1, flag = 0, playing = 1;
    
    //Tant que l'on peut jouer
    while(loop)
    {
        short col, row;

        //Si le joueur ne peut pas jouer, on saute son tour de jeu
        if(canPlay(playing, *positions, *lastPos, gridSize, gamemode))
        {
            //On décrémente le flag
            if(flag > 0)
                flag--;

            //Si c'est un joueur
            if(playing <= nbPlayers)
            {
                //Affichage de l'état actuel de la map
                printMap(*grid, *positions, gridSize);

                //Tant que la positions n'est pas valide, on la demande au joueur
                do
                {
                    printf("Tour du joueur %hi (score : %hi)\n", playing, (*scores)[playing-1]);
                    printf("Ligne : ");
                    scanf("%hi", &row);
                    printf("Colonne : ");
                    scanf("%hi", &col);
                } while(!canPlace(playing, *positions, *lastPos, gridSize, gamemode, col, row));
            }
            //Si c'est une IA
            else
            {
                
            }
            
            //Un fois la position est valide, on place le joueur sur la case et on augmente son score
            (*positions)[row][col] = playing == 1 ? 'r' : playing == 2 ? 'j' : playing == 3 ? 'v' : playing == 4 ? 'b' : '\0';
            (*scores)[playing-1] += (*grid)[row][col];
            (*lastPos)[playing-1][0] = row; //Update last row
            (*lastPos)[playing-1][1] = col; //Update last col
            printf("Nouveau score : %hi\n\n", (*scores)[playing-1]);
        }
        //S'il ne peut pas jouer, on incrémente le flag
        else
        {
            printf("Le joueur %hi ne peut pas jouer, il passe son tour (score : %hi)\n\n", playing, (*scores)[playing-1]);
            flag++;
            //Si le flag atteint le nombre de joueurs, on quitte la boucle de jeu
            if(flag == (nbPlayers+nbIA))
                loop--;
        }

        //On change de joueur pour le tour suivant
        if(playing == (nbPlayers+nbIA))
            playing = 1;
        else
            playing++;
    }
}

short canPlay(short player, char **positions, short **lastPos, short gridSize, short gamemode)
{
    char c = player == 1 ? 'r' : player == 2 ? 'j' : player == 3 ? 'v' : player == 4 ? 'b' : '\0';
    short i, j;

    //Conditions de jouabilité
    switch(gamemode)
    {
        //Octopus
        case 0:
            for(i=0 ; i < gridSize ; i++)
            {
                for(j=0 ; j < gridSize ; j++)
                {
                    //Pour chaque case appartenant à ce joueur...
                    if(positions[i][j] == c)
                    {
                        //... on vérifie que l'une de ses adjacentes est vide. Si oui, renvoie vrai.
                        if((i <= gridSize-2 && positions[i+1][j] == '\0')
                        || (i > 0 && positions[i-1][j] == '\0')
                        || (j <= gridSize-2 && positions[i][j+1] == '\0')
                        || (j > 0 && positions[i][j-1] == '\0')
                        || (i <= gridSize-2 && j <= gridSize-2 && positions[i+1][j+1] == '\0')
                        || (i <= gridSize-2 && j > 0 && positions[i+1][j-1] == '\0')
                        || (i > 0 && j > 0 && positions[i-1][j-1] == '\0')
                        || (i > 0 && j <= gridSize-2 && positions[i-1][j+1] == '\0'))
                            return 1;
                    }
                }
            }

            break;
        //Snake
        case 1:
            if((lastPos[player-1][0] <= gridSize-2 && positions[lastPos[player-1][0]+1][lastPos[player-1][1]] == '\0')
            || (lastPos[player-1][0] > 0 && positions[lastPos[player-1][0]-1][lastPos[player-1][1]] == '\0')
            || (lastPos[player-1][1] <= gridSize-2 && positions[lastPos[player-1][0]][lastPos[player-1][1]+1] == '\0')
            || (lastPos[player-1][1] > 0 && positions[lastPos[player-1][0]][lastPos[player-1][1]-1] == '\0'))
                return 1;
            break;
    }
    //Par défaut, on admet que le joueur ne peut pas jouer
    return 0;
}

short canPlace(short player, char **positions, short **lastPos, short gridSize, short gamemode, short col, short row)
{
    char c = player == 1 ? 'r' : player == 2 ? 'j' : player == 3 ? 'v' : player == 4 ? 'b' : '\0';

    if(positions[row][col] != '\0')
        return 0;

    switch(gamemode)
    {
        //Octopus
        case 0:
            if((row <= gridSize-2 && positions[row+1][col] == c)
            || (row > 0 && positions[row-1][col] == c)
            || (col <= gridSize-2 && positions[row][col+1] == c)
            || (col > 0 && positions[row][col-1] == c)
            || (row <= gridSize-2 && col <= gridSize-2 && positions[row+1][col+1] == c)
            || (row <= gridSize-2 && col > 0 && positions[row+1][col-1] == c)
            || (row > 0 && col > 0 && positions[row-1][col-1] == c)
            || (row > 0 && col <= gridSize-2 && positions[row-1][col+1] == c))
                return 1;
            break;
        //Snake
        case 1:
            if((row+1 == lastPos[player-1][0] && col == lastPos[player-1][1])
            || (row-1 == lastPos[player-1][0] && col == lastPos[player-1][1])
            || (col+1 == lastPos[player-1][1] && row == lastPos[player-1][0])
            || (col-1 == lastPos[player-1][1] && row == lastPos[player-1][0]))
                return 1;
            break;
    }
    //Par défaut, on admet que la position est invalide
    return 0;
}

void printMap(short **grid, char **positions, short gridSize)
{
    short i, j;

    for(i=0 ; i < gridSize ; i++)
    {
        for(j=0 ; j < gridSize ; j++)
        {
            if(positions[i][j] != '\0')
                printf("%c", positions[i][j]);
            else
                printf("%hi", grid[i][j]);
        }
        printf("\n");
    }
}

short generateValue(short player, char **positions, short **lastPos, short gridSize, short gamemode)
{
    srand(time(NULL));
}