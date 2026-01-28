/**
* \file DicoSynonymes.cpp
* \brief Le code des opérateurs du DicoSynonymes.
* \author IFT-2008, Alexis Roberge
* \version 0.1
* \date 22 juillet 2024
*
* Travail pratique numéro 3.
*
*/

#include "DicoSynonymes.h"
#include <algorithm>

namespace TP3
{
      /**
      *\brief     Constructeur
      *
      *\post      Une instance vide de la classe a été initialisée
      *
      */
    DicoSynonymes::DicoSynonymes() : racine(nullptr), nbRadicaux(0){
    }

      /**
      *\brief  Constructeur de dictionnaire à partir d'un fichier
      *
      *\pre    Il y a suffisament de mémoire
      *\pre    Le fichier est ouvert au préalable
      *
      *\post   Si le fichier est ouvert, l'instance de la classe a été initialisée à partir
      *        du fichier de dictionnaire. Sinon, on génère une classe vide.
      *
      *\exception bad_alloc s'il n'y a pas assez de mémoire
      * Cette méthode appelle chargerDicoSynonyme !
      */
    DicoSynonymes::DicoSynonymes(std::ifstream &fichier) : racine(nullptr), nbRadicaux(0){
        try
        {
            chargerDicoSynonyme(fichier);
        }
        catch (const std::bad_alloc& exception)
        {
            throw std::runtime_error("Erreur d'allocation de memoire");
        }
    }

      /**
      * \brief Charge le dictionnaire de synonymes
      *
      * \pre p_fichier est ouvert
      *
      * \post Le dictionnaire est initialisé.
      *
      * \exception invalid_argument si p_fichier n'est pas ouvert
      *
      */
	void DicoSynonymes::chargerDicoSynonyme(std::ifstream& fichier)
	{
        if (!fichier.is_open())
            throw std::logic_error("DicoSynonymes::chargerDicoSynonyme: Le fichier n'est pas ouvert !");

		std::string ligne;
		std::string buffer;
		std::string radical;
		int cat = 1;

		while (!fichier.eof()) // tant qu'on peut lire dans le fichier
		{
            std::getline(fichier, ligne);

			if (ligne == "$")
			{
				cat = 3;
				std::getline(fichier, ligne);
			}
			if (cat == 1)
			{
				radical = ligne;
				ajouterRadical(radical);
				cat = 2;
			}
			else if (cat == 2)
			{
				std::stringstream ss(ligne);
				while (ss >> buffer)
					ajouterFlexion(radical, buffer);
				cat = 1;
			}
			else
			{
				std::stringstream ss(ligne);
				ss >> radical;
				ss >> buffer;
				int position = -1;
				ajouterSynonyme(radical, buffer, position);
				while (ss >> buffer)
					ajouterSynonyme(radical, buffer, position);
			}
		}
	}

      /**
      *\brief     Destructeur.
      *
      *\post      Une instance de la classe est détruite.
      *
      */
    DicoSynonymes::~DicoSynonymes(){
        _auxDetruire(racine);
    }

      /**
      *\brief     Ajouter un radical au dictionnaire des synonymes
      *\brief     tout en s’assurant de maintenir l'équilibre de l'arbre.
      *
      *\pre    Il y a suffisament de mémoire.
      *
      *\post   Le mot est ajouté au dictionnaire des synonymes.
      *
      *\exception bad_alloc si il n'y a pas suffisament de mémoire.
      *\exception logic_error si le radical existe déjà.
      */
    void DicoSynonymes::ajouterRadical(const std::string& motRadical){
          if (_rechercherRadical(racine,motRadical) != nullptr) // si le radical existe déja
              throw std::logic_error("On ne peut pas avoir 2 fois le même radical dans le même arbre");
          _insererAVL(racine, motRadical);
      }

      /**
      *\brief  Ajouter une flexion (motFlexion) d'un radical (motRadical) à sa liste de flexions.
      *
      *\pre    Il y a suffisament de mémoire.
      *
      *\post   La flexion est ajoutée au dictionnaire des synonymes.
      *
      *\exception bad_alloc si il n'y a pas suffisament de mémoire.
      *\exception logic_error si motFlexion existe déjà ou motRadical n'existe pas.
      */
    void DicoSynonymes::ajouterFlexion(const std::string& motRadical, const std::string& motFlexion){
          NoeudDicoSynonymes* noeud = _rechercherRadical(racine, motRadical);
          if (noeud == nullptr) throw std::logic_error("Ce radical n'existe pas dans l'arbre"); // si le radical n'existe pas
          auto itr = std::find(noeud->flexions.begin(), noeud->flexions.end(), motFlexion);
          if (itr != noeud->flexions.end()) throw std::logic_error("On ne peut pas avoir 2 fois la même flexion"); // si la flexion existe déja
          noeud->flexions.push_back(motFlexion); // on ajoute la flexion
    }

      /**
      *\brief  Ajouter un synonyme (motSynonyme) d'un radical (motRadical)
      *\brief  à un de ses groupes de synonymes.
      *
      *\pre    Le radical se trouve déjà dans le dictionnaire
      *\       Il y a suffisament de mémoire
      *
      *\post   Le synonyme est ajouté au dictionnaire des synonymes
      *\       Si numGroupe vaut –1, le synonyme est ajouté dans un nouveau groupe de synonymes
      *\       et retourne le numéro de ce nouveau groupe dans numgroupe par référence.
      *
      *\exception bad_alloc si il n'y a pas suffisament de mémoire
      *\exception logic_error si motSynonyme est déjà dans la liste des synonymes du motRadical
      *\exception logic_error si numGroupe n'est pas correct ou motRadical n'existe pas.
      */
    void DicoSynonymes::ajouterSynonyme(const std::string& motRadical, const std::string& motSynonyme, int& numGroupe) {
          if (_rechercherRadical(racine,motSynonyme) != nullptr) _insererAVL(racine, motSynonyme);
          NoeudDicoSynonymes* noeud = _rechercherRadical(racine, motRadical);
          if (noeud == nullptr) // si le radical n'existe pas
          {
              throw std::logic_error("Le radical n'existe pas dans l'arbre");
          }
          for (const auto& groupeSyn : noeud->appSynonymes)
          {
              for (const auto& synonyme : groupesSynonymes[groupeSyn])
              {
                  if (synonyme->radical == motSynonyme) // si on trouve le synonyme qu'on veut ajouter
                  {
                      throw std::logic_error("Le synonyme existe déjà dans ce groupe");
                  }
              }
          }
          if (numGroupe == -1) // si le groupe n'existe pas
          {
              numGroupe = groupesSynonymes.size();
              groupesSynonymes.push_back(std::list<NoeudDicoSynonymes*>());
          }
          else if (numGroupe < 0 || numGroupe >= groupesSynonymes.size()) // le numéro du groupe est invalide
          {
              throw std::logic_error("Le numéro du groupe n'est pas correct");
          }
          auto itr = std::find(noeud->appSynonymes.begin(), noeud->appSynonymes.end(), numGroupe); // on trouve le groupe
          if (itr == noeud->appSynonymes.end())
          {
              noeud->appSynonymes.push_back(numGroupe);
          }
          _insererAVL(racine, motSynonyme);
          groupesSynonymes[numGroupe].push_back(new NoeudDicoSynonymes(motSynonyme));
      }

      /**
      *\brief     Supprimer un radical du dictionnaire des synonymes
      *\brief     tout en s’assurant de maintenir l'équilibre de l'arbre.
      *
      *\pre    motRadical fait parti de l'arbre.
      *
      *\post   Si le radical appartient au dictionnaire, on l'enlève et on équilibre.
      *\       Il faut libérer la liste des synonymes
      *\       du radical en question ainsi que sa liste des flexions.
      *\       Il faut aussi s’assurer d’éliminer tout pointeur
      *\       pointant sur le nœud contenant le radical.
      *
      *\exception logic_error si l'arbre est vide ou motRadical n'existe pas.
      */
    void DicoSynonymes::supprimerRadical(const std::string& motRadical){
          if (racine == nullptr) // arbre vide
          {
              throw std::logic_error("L'arbre est vide, on ne peut pas supprimer de radical");
          }
          NoeudDicoSynonymes* noeud = _rechercherRadical(racine, motRadical);
          if (noeud == nullptr) // radical n'existe pas
          {
              throw std::logic_error("Le radical que l'on veut supprimer n'existe pas dans l'arbre");
          }
          _auxSupprimerAVL(racine, motRadical);
    }

      /**
      *\brief   Supprimer une flexion (motFlexion) d'un radical
      *\brief   (motRadical) de sa liste de flexions.
      *
      *\pre    motRadical et motFlexion font parti de l'arbre.
      *
      *\post   La flexion est enlevée du dictionnaire des synonymes.
      *
      *\exception logic_error si si l'arbre est vide ou
      *\exception motFlexion n'existe pas ou motRadical n'existe pas.
      */
    void DicoSynonymes::supprimerFlexion(const std::string& motRadical, const std::string& motFlexion){
          if (racine == nullptr) // si arbre est vide
          {
              throw std::logic_error("L'arbre est vide, on ne peut pas effacer de flexion");
          }
          NoeudDicoSynonymes* noeud = _rechercherRadical(racine, motRadical);
          if (noeud == nullptr) // si le radical n'existe pas
          {
              throw std::logic_error("Le radical n'existe pas dans l'arbre");
          }
          auto itr = std::find(noeud->flexions.begin(), noeud->flexions.end(), motFlexion);
          if (itr == noeud->flexions.end()) // si la flexion n'existe pas
          {
              throw std::logic_error("La flexion n'existe pas");
          }
          noeud->flexions.erase(itr);
    }

      /**
      *\brief   Retirer motSynonyme faisant partie du numéro de groupe numGroupe du motRadical.
      *
      *\pre    motRadical et motSynonyme existent et motRadical
      *\pre    a une appartenance au groupe numGroupe
      *
      *\post      Le synonyme est enlevé du dictionnaire des synonymes.
      *
      *\exception logic_error si motSynonyme ou motRadical ou numGroupe n'existent pas.
      */
    void DicoSynonymes::supprimerSynonyme(const std::string& motRadical, const std::string& motSynonyme, int& numGroupe){
          NoeudDicoSynonymes* noeud = _rechercherRadical(racine, motRadical);
          if (noeud == nullptr) // si le radical n'existe pas
          {
              throw std::logic_error("Le radical n'existe pas dans l'arbre");
          }
          if (numGroupe < 0 || numGroupe >= groupesSynonymes.size()) // si le numero de groupe est invalide
          {
              throw std::logic_error("Le numéro du groupe n'est pas correct");
          }
          auto& groupeSyn = groupesSynonymes[numGroupe];
          auto itr = groupeSyn.end();
          for (auto itr2 = groupeSyn.begin(); itr2 != groupeSyn.end(); ++itr2)
          {
              if ((*itr2)->radical == motSynonyme)
              {
                  itr = itr2;
                  break; // on arrête la boucle lorsque le synonyme est trouvé
              }
          }
          if (itr == groupeSyn.end()) // si on a pas trouvé le synonyme, l'iterateur sera == end
          {
              throw std::logic_error("supprimerSynonyme: Le motSynonyme n'existe pas");
          }
          groupeSyn.erase(itr);

          if (groupeSyn.empty())
          {
              groupesSynonymes.erase(groupesSynonymes.begin() + numGroupe);

              for (NoeudDicoSynonymes* Noeud : parcourArbre()) // on parcours tous les noeuds de l'arbre
              {
                  std::vector<int>& synonymes = Noeud->appSynonymes;
                  auto itr3 = std::remove(synonymes.begin(), synonymes.end(), numGroupe);
                  synonymes.erase(itr3, synonymes.end());
                  for (int& groupeSynonyme : synonymes)
                  {
                      if (groupeSynonyme > numGroupe)
                      {
                          groupeSynonyme--;
                      }
                  }
              }
          }
      }


      /**
      *\brief     Vérifier si le dictionnaire est vide
      *
      *\post      Le dictionnaire est inchangée
      *
      */
    bool DicoSynonymes::estVide() const{
        return nbRadicaux == 0;
    }

     /**
      * \brief Retourne le nombre de radicaux dans le dictionnaire
      *
      * \post Le dictionnaire reste inchangé.
      *
      */
    int DicoSynonymes::nombreRadicaux() const{
        return nbRadicaux;
    }

     /**
      * \brief Retourne le radical du mot entré en paramètre
      *
      * \pre mot est dans la liste des flexions d'un radical
      *
      * \post Le dictionnaire reste inchangé.
      * \post Le radical est retournée.
      *
      * \exception logic_error si l'arbre est vide
      * \exception logic_error si la flexion n'est pas dans la liste de flexions du radical
      *
      */
    std::string DicoSynonymes::rechercherRadical(const std::string& mot) const
    {
        if (racine == nullptr) // si l'arbre est vide
        {
            throw std::logic_error("L'arbre est vide");
        }
        float SimilitudePlusElevee = 0;
        std::vector<NoeudDicoSynonymes*> vecteurnoeuds = parcourArbre(); // vecteur de tous les noeuds de l'arbre
        NoeudDicoSynonymes* Noeudchoisi = nullptr; // initialise le noeud qui sera choisi
        for (NoeudDicoSynonymes* noeud : vecteurnoeuds) // on parcours le vecteur de noeuds
        {
            for (const auto& flexion : noeud->flexions)
            {
                if (flexion == mot) // si la flexion est celle qu'on cherche
                {
                    float similitudetemp = similitude(noeud->radical, mot); //on calcule le degré de similitude
                    if (similitudetemp > SimilitudePlusElevee) // si la nouvelle similitude est plus grande
                    {
                        SimilitudePlusElevee = similitudetemp;
                        Noeudchoisi = noeud;
                    }
                }
            }
        }
        if (Noeudchoisi == nullptr)
        {
            throw std::logic_error("La flexion n'existe pas pour ce radical");
        }
        return Noeudchoisi->radical;
    }


     /**
      * \brief Retourne un réel entre 0 et 1 qui représente le degré de similitude entre mot1 et mot2 où
      *        0 représente deux mots complétement différents et 1 deux mots identiques.
      *        Vous pouvez utiliser par exemple la distance de Levenshtein, mais ce n'est pas obligatoire !
      *        ***
      *        J'ai utiliser une fonction de distance de Levenshtein trouvée sur internet et que j'ai modifiée un peu,
      *        puisque le professeur permettait de le faire. Vous pouvez la retrouver sur le lien suivant :
      *        https://github.com/guilhermeagostinelli/levenshtein/blob/master/levenshtein.cpp
      *        ***
      *
      * \post Un réel entre 0 et 1 est retourné
      * \post Le dictionnaire reste inchangé.
      *
      */
    float DicoSynonymes::similitude(const std::string& mot1, const std::string& mot2) const {
         int distance;
         float similitude;
         int size1 = mot1.size();
         int size2 = mot2.size();
         int verif[size1 + 1][size2 + 1];

         for (int i = 0; i <= size1; i++)
             verif[i][0] = i;
         for (int j = 0; j <= size2; j++)
             verif[0][j] = j;

         for (int i = 1; i <= size1; i++) {
             for (int j = 1; j <= size2; j++) {
                 int cost = (mot2[j - 1] == mot1[i - 1]) ? 0 : 1;
                 verif[i][j] = std::min(
                         std::min(verif[i - 1][j] + 1, verif[i][j - 1] + 1),
                         verif[i - 1][j - 1] + cost
                 );
             }
         }
         distance = verif[size1][size2];
         similitude = float((100 - (5 * distance))) / 100;
         return similitude;
     }

     /**
      * \brief Donne le nombre de cellules de appSynonymes.
      *
      * \post Le nombre de cellules de appSynonymes pour le radical entré en paramètre est retourné
      * \post Le dictionnaire reste inchangé.
      *
      */
    int DicoSynonymes::getNombreSens(std::string radical) const{
         NoeudDicoSynonymes* noeud = _rechercherRadical(racine, radical);
         if (noeud == nullptr) throw std::logic_error("Le radical n'existe pas dans l'arbre");
         return noeud->appSynonymes.size();
    }

     /**
      * \brief Donne le premier synonyme du groupe de synonyme de l'emplacement entrée en paramètre.
      *
      * \post un string correspondant au premier synonyme est imprimé
      * \post Le dictionnaire reste inchangé.
      * \exception logic_error Si le radical ou la position n'existe pas
      * \exception logic_error si le numéro de groupe de synonme n'est pas correct ou si celui-ci est vide
      */
    std::string DicoSynonymes::getSens(std::string radical, int position) const{
         NoeudDicoSynonymes* noeud = _rechercherRadical(racine, radical);
         if (noeud == nullptr) throw std::logic_error("Le radical n'existe pas dans l'arbre");
         if (position < 0 || position >= noeud->appSynonymes.size()) throw std::logic_error("La position n'existe pas");
         int groupNumber = noeud->appSynonymes[position];
         if (groupNumber < 0 || groupNumber >= groupesSynonymes.size()) throw std::logic_error("Le numéro du groupe n'est pas correct");
         auto& groupeSyn = groupesSynonymes[groupNumber];
         if (groupeSyn.empty()) throw std::logic_error("Le groupe de synonymes est vide");
         return groupeSyn.front()->radical;
     }

     /**
      * \brief Donne tous les synonymes du mot entré en paramètre du groupeSynonyme du parametre position
      *
      * \post un vecteur est retourné avec tous les synonymes
      * \post Le dictionnaire reste inchangé.
      * \exception logic_error Si le radical ou la position n'existe pas
      * \exception logic_error si le numéro de groupe de synonme n'est pas correct
      */
    std::vector<std::string> DicoSynonymes::getSynonymes(std::string radical, int position) const{
         NoeudDicoSynonymes* noeud = _rechercherRadical(racine, radical);
         if (noeud == nullptr) throw std::logic_error("Le radical n'existe pas dans l'arbre");
         if (position < 0 || position >= noeud->appSynonymes.size()) throw std::logic_error("La position n'existe pas");
         int groupNumber = noeud->appSynonymes[position];
         if (groupNumber < 0 || groupNumber >= groupesSynonymes.size()) throw std::logic_error("Le numéro du groupe n'est  pas correct");
         std::vector<std::string> Vecteursynonymes;
         auto& groupeSyn = groupesSynonymes[groupNumber];
         for (auto synonyme : groupeSyn) // on parcours les synonymes du groupe
         {
             Vecteursynonymes.push_back(synonyme->radical);
         }
         return Vecteursynonymes;
    }

     /**
      * \brief Donne toutes les flexions du mot entré en paramètre
      *
      * \post un vecteur est retourné avec toutes les flexions
      * \post Le dictionnaire reste inchangé.
      * \exception logic_error Si le radical n'existe pas
      */
    std::vector<std::string> DicoSynonymes::getFlexions(std::string radical) const
    {
        std::vector<std::string> flexions;
        NoeudDicoSynonymes* noeud = _rechercherRadical(racine, radical);
        if (noeud == nullptr) throw std::logic_error("Le radical n'existe pas dans l'arbre");
        for (const auto& flexion : noeud->flexions) // on parcours les flexions du radical
        {
            flexions.push_back(flexion);
        }
        return flexions;
    }

     /**
	  * \brief Indique si l'arbre est équilibré selon le facteur HB(1)
	  *
	  * \post L'arbre est inchangé
	  * \return bool indiquant si l'arbre est AVL
      *
	  */
    bool DicoSynonymes::estArbreAVL() const{
         if (this->estVide()) return true;
         std::queue<NoeudDicoSynonymes*> file;
         NoeudDicoSynonymes * Noeud;
         file.push(racine);
         while (!file.empty())
         {
             Noeud = file.front();
             if (_amplitudeDuDebalancement(Noeud) > 1) // si l'arbre n'est pas équilibré
             {
                 return false;
             }
             file.pop();
             if (Noeud->gauche != nullptr) // si il  y a un autre noeud à gauche
             {
                 file.push(Noeud->gauche);
             }
             if (Noeud->droit != nullptr) // si il y a un autre noeud à droite
             {
                 file.push(Noeud->droit);
             }
         }
         return true;
     }

	// Mettez l'implantation des autres méthodes (surtout privées) ici.

    /**
     * \brief Insère un radical dans l'arbre
     * \param[in] noeud le noeud où l'on veut insérer
     * \param[in] Radical le radical que l'on veut insérer
     */
    void DicoSynonymes::_insererAVL(NoeudDicoSynonymes*& noeud, const std::string& Radical)
    {
        if (_rechercherRadical(racine,Radical) != nullptr) return; // si le radical existe déja on sort
        if (noeud == nullptr)
        {
            noeud = new NoeudDicoSynonymes(Radical);
            nbRadicaux++;
            return;
        }
        else if (Radical < noeud->radical) // plus petit donc insertion à gauche
        {
            _insererAVL(noeud->gauche, Radical);
        }
        else if (Radical >= noeud->radical) // plus grand donc on insert à droite
        {
            _insererAVL(noeud->droit, Radical);
        }
        _miseAJourHauteurNoeud(noeud);
        _balancerUnNoeud(noeud); // on garde les hauteurs et l'arbre balancé
    }

    /**
     * \brief Met à jour la hauteur d'un noeud pour que celle-ci soit toujours valide
     * \param[in] noeud le noeud dont on veut mettre à jour la hauteur
     */
    void DicoSynonymes::_miseAJourHauteurNoeud(NoeudDicoSynonymes*& noeud)
    {
        if (noeud != nullptr) {
            noeud->hauteur = 1 + std::max(_hauteur(noeud->droit), _hauteur(noeud->gauche));
        }
    }

    /**
     * \brief Effectue les balancement nécéssaire d'un noeud pour que l'arbre reste équilibré
     * \param[in] noeud le noeud que l'on veut balancer
     */
    void DicoSynonymes::_balancerUnNoeud(NoeudDicoSynonymes*& noeud)
    {
        if (noeud == nullptr) return;
        if (_debalancementAGauche(noeud))
        {
            if (_sousArbrePencheADroite(noeud->gauche)) // gauche -> droite = zig zag gauche
            {
                _zigZagGauche(noeud);
            }
            else // gauche -> gauche = zig zig gauche
            {
                _zigZigGauche(noeud);
            }
        }
        else if (_debalancementADroite(noeud))
        {
            if (_sousArbrePencheAGauche(noeud->droit)) // droite -> gauche = zig zag droit
            {
                _zigZagDroite(noeud);
            }
            else // droite -> droite = zig zig droit
            {
                _zigZigDroite(noeud);
            }
       }
    }

    /**
     * \brief donne la hauteur d'un noeud de l'abre
     * \param[in] noeud le noeud dont on cherche la hauteur
     * \return int la hauteur du noeud
     */
    int DicoSynonymes::_hauteur(NoeudDicoSynonymes* noeud) const
    {
        if (noeud == nullptr) return -1; // si l'arbre est vide, on met -1 comme valeur de hauteur par défaut
        return noeud->hauteur;
    }

    /**
     * \brief vérifie si l'arbre est débalancer vers la gauche
     * \param[in] Arbre l'arbre dont on veut avoir le status de stabilité
     * \return bool true si l'arbre est débalancer vers la gauche, false sinon
     */
    bool DicoSynonymes::_debalancementAGauche(NoeudDicoSynonymes * & Arbre) const
    {
        if (Arbre == nullptr) return false;
        return (_hauteur(Arbre->gauche) - _hauteur(Arbre->droit)) >= 2;
    }

    /**
     * \brief vérifie si l'arbre est débalancer vers la droite
     * \param[in] Arbre l'arbre dont on veut avoir le status de stabilité
     * \return bool true si l'arbre est débalancer vers la droite, false sinon
     */
    bool DicoSynonymes::_debalancementADroite(NoeudDicoSynonymes * & Arbre) const
    {
        if (Arbre == nullptr) return false;
        return (_hauteur(Arbre->droit) - _hauteur(Arbre->gauche)) >= 2;
    }

    /**
     * \brief vérifie si un sous arbre penche vers la gauche
     * \param[in] sousArbre le sous arbre que l'on veut vérifier
     * \return bool true si le sous arbre penche vers la gauche, false sinon
     */
    bool DicoSynonymes::_sousArbrePencheAGauche(NoeudDicoSynonymes * & sousArbre) const
    {
        if (sousArbre == nullptr) return false;
        return (_hauteur(sousArbre->gauche) > _hauteur(sousArbre->droit));
    }

    /**
     * \brief vérifie si un sous arbre penche vers la droite
     * \param[in] sousArbre le sous arbre que l'on veut vérifier
     * \return bool true si le sous arbre penche vers la droite, false sinon
     */
    bool DicoSynonymes::_sousArbrePencheADroite(NoeudDicoSynonymes * & sousArbre) const
    {
        if (sousArbre == nullptr) return false;
        return (_hauteur(sousArbre->droit) > _hauteur(sousArbre->gauche));
    }

    /**
     * \brief Effectue un mouvement de balancement zig zig gauche
     * \param[in] NoeudCritique le noeud critique du balancement
     */
    void DicoSynonymes::_zigZigGauche(NoeudDicoSynonymes * & NoeudCritique)
    {
        NoeudDicoSynonymes * noeudSousCritique = NoeudCritique->gauche;
        NoeudCritique->gauche = noeudSousCritique->droit;
        noeudSousCritique->droit = NoeudCritique;
        _miseAJourHauteurNoeud(NoeudCritique);
        _miseAJourHauteurNoeud(noeudSousCritique);
        NoeudCritique = noeudSousCritique;
    }

    /**
     * \brief Effectue un mouvement de balancement zig zag gauche
     * \param[in] NoeudCritique le noeud critique du balancement
     */
    void DicoSynonymes::_zigZagGauche(NoeudDicoSynonymes * & NoeudCritique)
    {
        _zigZigDroite(NoeudCritique->gauche);
        _zigZigGauche(NoeudCritique);
    }

    /**
     * \brief Effectue un mouvement de balancement zig zig droit
     * \param[in] NoeudCritique le noeud critique du balancement
     */
    void DicoSynonymes::_zigZigDroite(NoeudDicoSynonymes * & NoeudCritique)
    {
        NoeudDicoSynonymes * noeudSousCritique = NoeudCritique->droit;
        NoeudCritique->droit = noeudSousCritique->gauche;
        noeudSousCritique->gauche = NoeudCritique;
        _miseAJourHauteurNoeud(NoeudCritique);
        _miseAJourHauteurNoeud(noeudSousCritique);
        NoeudCritique = noeudSousCritique;
    }

    /**
     * \brief Effectue un mouvement de balancement zig zag droite
     * \param[in] NoeudCritique le noeud critique du balancement
     */
    void DicoSynonymes::_zigZagDroite(NoeudDicoSynonymes * & NoeudCritique)
    {
        _zigZigGauche(NoeudCritique->droit);
        _zigZigDroite(NoeudCritique);
    }

    /**
     * \brief trouve le noeud qui contient le mot recherché
     * \param[in] noeud le noeud qu'on vérifie s'il contient le mot recherché
     * \param[in] mot le mot qu'on cherche
     * \return le noeud qui contient le mot recherché s'il existe, sinon nullptr
     */
    DicoSynonymes::NoeudDicoSynonymes* DicoSynonymes::_rechercherRadical(NoeudDicoSynonymes* noeud, const std::string& mot) const
    {
        if (noeud != nullptr)
        {
            auto itr = std::find(noeud->flexions.begin(), noeud->flexions.end(), mot);
            if (mot == noeud->radical || itr != noeud->flexions.end())
            {
                return noeud;
            }
            else if (mot < noeud->radical) // si le mot est plus petit on doit chercher à gauche
            {
                return _rechercherRadical(noeud->gauche, mot);
            }
            else // si le mot est plus grand on doit chercher à droite
            {
                return _rechercherRadical(noeud->droit, mot);
            }
        }
        return nullptr;
    }

    /**
     * \brief Parcours l'entièreté de l'arbre et garde chaque noeud dans un vecteur
     * \param[in] noeud le noeud visité en ce moment
     * \param[in] vec le vecteur de noeuds
     */
    void DicoSynonymes::_parcourirArbre(NoeudDicoSynonymes* noeud, std::vector<NoeudDicoSynonymes*>& vec) const
    {
        if (noeud != nullptr)
        {
            _parcourirArbre(noeud->gauche, vec);
            vec.push_back(noeud);
            _parcourirArbre(noeud->droit, vec);
        }
    }

    /**
     * \brief détruit chaque noeud de l'arbre
     * \param[in] noeud le noeud à détruire
     */
    void DicoSynonymes::_auxDetruire(NoeudDicoSynonymes*& noeud)
    {
        if (noeud != nullptr) // si le noeud existe
        {
            _auxDetruire(noeud->gauche); // appel récursif pour noeud gauche
            _auxDetruire(noeud->droit); // appel récursif pour noeud droite
            delete noeud; // on détruit le noeud courant
            noeud = nullptr;
        }
    }

    /**
     * \brief Permet de supprimer un noeud de L'arbre
     * \param[in] noeud le noeud que l'on veut supprimer
     * \param[in] Radical le radical que l'on veut supprimer
     * \exception logic_error si le noeud qu'on veut supprimer n'existe pas
     */
    void DicoSynonymes::_auxSupprimerAVL(NoeudDicoSynonymes*& noeud, const std::string& Radical)
    {
        if (noeud == nullptr) throw std::logic_error("Ce noeud n'existe pas");
        if (Radical < noeud->radical) // si radical plus petit on continue à gauche
        {
            _auxSupprimerAVL(noeud->gauche, Radical);
        }
        else if (noeud->radical < Radical) // si radical plus grand on continue à droite
        {
            _auxSupprimerAVL(noeud->droit, Radical);
        }
        else // on est sur celui qu'on doit supprimer
        {
            if (noeud->gauche != nullptr && noeud->droit != nullptr) // on trouve le successeur le plus petit à droite pour remplacer avec celui à enlever
            {
                _enleverSuccMinDroite(noeud);
            }
            else
            {
                NoeudDicoSynonymes* ancienNoeud = noeud;
                if (noeud->gauche != nullptr) {noeud = noeud->gauche;}
                else {noeud = noeud->droit;}
                delete ancienNoeud;
                --nbRadicaux;
            }
        }
        _balancerUnNoeud(noeud); // on garde l'arbre balancé
    }
     /**
      * \brief permet d'enlever le successeur à droite le plus petit à partir d'un noeud
      * \param[in] noeud le point de départ pour trouver son successeur
      */
    void DicoSynonymes::_enleverSuccMinDroite(NoeudDicoSynonymes* noeud)
    {
        NoeudDicoSynonymes* enfant = noeud->droit;
        NoeudDicoSynonymes* parent = noeud;
        while (enfant->gauche != nullptr) // on continue jusqu'à ce qu'on trouve le plus petit enfant
        {
            parent = enfant;
            enfant = enfant->gauche;
        }
        noeud->radical = enfant->radical;
        noeud->flexions = enfant->flexions;
        noeud->appSynonymes = enfant->appSynonymes;
        if (parent->gauche == enfant)
        {
            _auxSupprimerAVL(parent->gauche, enfant->radical);
        }
        else
        {
            _auxSupprimerAVL(parent->droit, enfant->radical);
        }
    }

    /**
     * \brief permet de trouver la différence de hauteur entre le coté gauche et droite
     * \param[in] Arbre l'arbre dont on veut trouver la différence de hauteur entre ses 2 côtés
     * \return int la différence de hauteur entre le côté gauche et droit
     */
    int DicoSynonymes::_amplitudeDuDebalancement(NoeudDicoSynonymes * Arbre) const
    {
        return std::abs(_hauteur(Arbre->gauche) - _hauteur(Arbre->droit));
    }

    /**
     * \brief effectue le parcours de l'arbre complet
     * \return  std::vector<NoeudDicoSynonymes*> un vecteur contenant tous les noeuds de l'arbre parcouru
     */
    std::vector<DicoSynonymes::NoeudDicoSynonymes*> DicoSynonymes::parcourArbre() const
    {
        std::vector<NoeudDicoSynonymes*> vec;
        _parcourirArbre(racine, vec);
        return vec;
    }

}//Fin du namespace