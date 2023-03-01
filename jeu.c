/*
	Canvas pour algorithmes de jeux à 2 joueurs

	joueur 0 : humain
	joueur 1 : ordinateur

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
// Paramètres du jeu
#define LARGEUR_MAX 7 // nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 1 // temps de calcul pour un coup avec MCTS (en secondes)
#define C sqrt(2) // Constante C (dans le calcul de la valeur B d'un noeud)
// macros
#define AUTRE_JOUEUR(i) (1 - (i))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum
{
	NON,
	MATCHNUL,
	ORDI_GAGNE,
	HUMAIN_GAGNE
} FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt
{

	int joueur; // à qui de jouer ?

	//Pour le puissance 4
	int pile[7]; //Création de 7 piles pour chaque colonne (permettra d'avoir le numéro de ligne pour poser un pion)
	char plateau[6][7]; //Taille du plateau (6 ligne et 7 colonnes)

} Etat;

// Definition du type Coup
typedef struct
{
	int ligne;
	int colonne;

} Coup;

// Copier un état
Etat *copieEtat(Etat *src)
{
	Etat *etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;

	int i, j;
	for (i = 0; i < 7; i++)
		etat->pile[i] = src->pile[i];
	for (i = 0; i < 6; i++)
		for (j = 0; j < 7; j++)
			etat->plateau[i][j] = src->plateau[i][j];
	return etat;
}

// Etat initial
Etat *etat_initial(void)
{
	Etat *etat = (Etat *)malloc(sizeof(Etat));

	int i, j;
	for (i = 0; i < 7; i++)
		etat->pile[i] = 5; //Numéro de ligne ou le pion sera posé, initialement à la ligne la plus basse
	for (i = 0; i < 6; i++)
		for (j = 0; j < 7; j++)
			etat->plateau[i][j] = ' '; //Initialisation de chaque case du plateau
	return etat;
}

void afficheJeu(Etat *etat)
{

	int i, j;
	printf("   |");
	for (j = 0; j < 7; j++)
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");

	for (i = 0; i < 6; i++)
	{
		printf(" %d |", i);
		for (j = 0; j < 7; j++)
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}

// Nouveau coup
Coup *nouveauCoup(int i, Etat *etat)
{
	Coup *coup = (Coup *)malloc(sizeof(Coup));

	coup->ligne = etat->pile[i]; //Prend le numéro de la ligne
	coup->colonne = i; //Prend le numéro de la colonne
	return coup;
}

// Demander à l'humain quel coup jouer
Coup *demanderCoup(Etat *etat)
{

	int colonne;
	printf(" quelle colonne ? ");
	scanf("%d", &colonne);

	return nouveauCoup(colonne, etat);
}

// Modifier l'état en jouant un coup
// retourne 0 si le coup n'est pas possible
int jouerCoup(Etat *etat, Coup *coup)
{

	// TODO: à compléter

	/* par exemple : */
	if (etat->plateau[0][coup->colonne] != ' ')
		return 0;
	else
	{
		etat->plateau[coup->ligne][coup->colonne] = etat->joueur ? 'O' : 'X';
		etat->pile[coup->colonne]--;
		// à l'autre joueur de jouer
		etat->joueur = AUTRE_JOUEUR(etat->joueur);

		return 1;
	}
}

// Retourne une liste de coups possibles à partir d'un etat
// (tableau de pointeurs de coups se terminant par NULL)
Coup **coups_possibles(Etat *etat)
{

	Coup **coups = (Coup **)malloc((1 + LARGEUR_MAX) * sizeof(Coup *));

	int k = 0;


	int i, j;
	for (i = 0; i < 7; i++)
	{
		if (etat->plateau[0][i] == ' ') //Si la colonne choisie n'est pas remplie entièrement
		{
			coups[k] = nouveauCoup(i, etat); //Joue un nouveau coup
			k++;
		}
	}

	coups[k] = NULL;

	return coups;
}

// Definition du type Noeud
typedef struct NoeudSt
{

	int joueur; // joueur qui a joué pour arriver ici
	Coup *coup; // coup joué par ce joueur pour arriver ici

	Etat *etat; // etat du jeu

	struct NoeudSt *parent;
	struct NoeudSt *enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;						  // nb d'enfants présents dans la liste

	// POUR MCTS:
	int nb_victoires;
	int nb_simus;

} Noeud;

// Créer un nouveau noeud en jouant un coup à partir d'un parent
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud *nouveauNoeud(Noeud *parent, Coup *coup)
{
	Noeud *noeud = (Noeud *)malloc(sizeof(Noeud));

	if (parent != NULL && coup != NULL)
	{
		noeud->etat = copieEtat(parent->etat);
		jouerCoup(noeud->etat, coup);
		noeud->coup = coup;
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);
	}
	else
	{
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0;
	}
	noeud->parent = parent;
	noeud->nb_enfants = 0;

	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;

	return noeud;
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud *ajouterEnfant(Noeud *parent, Coup *coup)
{
	Noeud *enfant = nouveauNoeud(parent, coup);
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud(Noeud *noeud)
{
	if (noeud->etat != NULL)
		free(noeud->etat);

	while (noeud->nb_enfants > 0)
	{
		freeNoeud(noeud->enfants[noeud->nb_enfants - 1]);
		noeud->nb_enfants--;
	}
	if (noeud->coup != NULL)
		free(noeud->coup);

	free(noeud);
}

// Test si l'état est un état terminal
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin(Etat *etat)
{

	// tester si un joueur a gagné
	int i, j, k, n = 0;
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 7; j++)
		{
			if (etat->plateau[i][j] != ' ')
			{
				n++; // nb coups joués

				// lignes
				k = 0;
				while (k < 4 && i + k < 6 && etat->plateau[i + k][j] == etat->plateau[i][j])
					k++;
				if (k == 4)
					return etat->plateau[i][j] == 'O' ? ORDI_GAGNE : HUMAIN_GAGNE;

				// colonnes
				k = 0;
				while (k < 4 && j + k < 7 && etat->plateau[i][j + k] == etat->plateau[i][j])
					k++;
				if (k == 4)
					return etat->plateau[i][j] == 'O' ? ORDI_GAGNE : HUMAIN_GAGNE;

				// diagonales
				k = 0;
				while (k < 4 && i + k < 6 && j + k < 7 && etat->plateau[i + k][j + k] == etat->plateau[i][j])
					k++;
				if (k == 4)
					return etat->plateau[i][j] == 'O' ? ORDI_GAGNE : HUMAIN_GAGNE;

				k = 0;
				while (k < 4 && i + k < 6 && j - k >= 0 && etat->plateau[i + k][j - k] == etat->plateau[i][j])
					k++;
				if (k == 4)
					return etat->plateau[i][j] == 'O' ? ORDI_GAGNE : HUMAIN_GAGNE;
			}
		}
	}

	// et sinon tester le match nul
	if (n == 6 * 7) //Lorsque le plateau est remplie entièrement
		return MATCHNUL;

	return NON;
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat *etat, int tempsmax)
{

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup **coups;
	Coup *meilleur_coup;

	// Créer l'arbre de recherche
	Noeud *racine = nouveauNoeud(NULL, NULL);
	racine->etat = copieEtat(etat);
	// créer les premiers noeuds:
	coups = coups_possibles(racine->etat);
	Noeud *enfant;

	int iter = 0;
	do
	{
		Noeud *noeudActuel = racine;
		FinDePartie fin_de_partie = testFin(noeudActuel->etat);
		while (fin_de_partie == NON)//Tant qu'il n'y a pas de gagnant ou que le plateau soit complet selon le cgemin emprunté par l'ordinateur
		{
			noeudActuel->nb_simus++;
			if (noeudActuel->nb_enfants == 0)
			{
				Coup **coup_possible = coups_possibles(noeudActuel->etat);
				int nbCoup = 0;
				while (coup_possible[nbCoup] != NULL)
				{
					// Coup* _fils = coup_possible[nbCoup];
					ajouterEnfant(noeudActuel, coup_possible[nbCoup]);
					nbCoup++;
				}
			}
			float B = -1;
			int numNoeud = 0;
			int i = 0;
			for (i; i < noeudActuel->nb_enfants; i++)
			{
				if (noeudActuel->enfants[i]->nb_simus == 0) //Si un noeud n'a pas été fait par l'ordi
				{
					numNoeud = i;
					break;
				}

				float muValeur = (float)noeudActuel->enfants[i]->nb_victoires / (float)noeudActuel->enfants[i]->nb_simus; // μ dans la formule du MCTS
				if (noeudActuel->joueur != 0)
				{
					muValeur = -muValeur;
				}
				float B_Noeud = muValeur + C * sqrt((float)log2(noeudActuel->nb_simus) / (float)noeudActuel->enfants[i]->nb_simus); // B(i) dans la formule
				if (B_Noeud > B)
				{
					numNoeud = i;
					B = B_Noeud;
				}
			}

			noeudActuel = noeudActuel->enfants[numNoeud];
			fin_de_partie = testFin(noeudActuel->etat);//Vérifie si l'un des deux joueurs a gagnés
		}
		if (fin_de_partie == ORDI_GAGNE) //Lorsque que dans une simulation l'ordination gagne
		{
			while (noeudActuel != NULL)
			{
				noeudActuel->nb_victoires++; //incrémente le nombre de victoire à 1 sur les noeuds dans le chemin choisi par l'ordinateur
				noeudActuel = noeudActuel->parent; //Remonte le chemin jusqu'à la racine
			}
		}

		toc = clock();
		temps = (int)(((double)(toc - tic)) / CLOCKS_PER_SEC);
		iter++;
	} while (temps < tempsmax);
	int nbVictoire;
	int nbSimus;
	float ratio;
	int meilleurNoeud = 0;
	float meilleurNoeudNum = 0;
	for (int numColonne = 0; numColonne < racine->nb_enfants; numColonne++) //Pour chaque colonne
	{
		nbVictoire = racine->enfants[numColonne]->nb_victoires; //Nombre de victoire sur un noeud
		nbSimus = racine->enfants[numColonne]->nb_simus; //Nombre de simulation sur un noeud
		ratio = (double)nbVictoire/nbSimus; //ratio entre le nombre de victoire et le nombre de simulation sur un même noeud
		printf("colonne %d nbVictoire %d nbSimus %d ratio %f \n", numColonne, nbVictoire, nbSimus, ratio); //Affichage des données pour chaque colonne

		if (nbVictoire > meilleurNoeudNum)
		{
			meilleurNoeud = numColonne; //Prend le numéro de la colonne
			meilleurNoeudNum = nbSimus; 
		}
	}
	meilleur_coup = racine->enfants[meilleurNoeud]->coup;
	/* fin de l'algorithme  */

	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup);

	// Penser à libérer la mémoire :
	freeNoeud(racine);
	free(coups);
}

int main(void)
{

	Coup *coup;
	FinDePartie fin;

	// initialisation
	Etat *etat = etat_initial();

	// Choisir qui commence :
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur));

	// boucle de jeu
	do
	{
		printf("\n");
		afficheJeu(etat);

		if (etat->joueur == 0)
		{
			// tour de l'humain

			do
			{
				coup = demanderCoup(etat);
			} while (!jouerCoup(etat, coup));
		}
		else
		{
			// tour de l'Ordinateur
			
			ordijoue_mcts(etat, TEMPS);
		}

		fin = testFin(etat);
	} while (fin == NON);

	printf("\n");
	afficheJeu(etat);

	if (fin == ORDI_GAGNE)
		printf("** L'ordinateur a gagné **\n");
	else if (fin == MATCHNUL)
		printf(" Match nul !  \n");
	else
		printf("** BRAVO, l'ordinateur a perdu  **\n");
	return 0;
}
