#ifndef __TEMPERATURE__
#define __TEMPERATURE__

#include <time.h>
#include <stdio.h>

#include <native/queue.h>
#include <native/sem.h>
#include <native/mutex.h>

#define MIN_TEMPERATURE -50 //! temperature minimale
#define MAX_TEMPERATURE 50 //! temperature maximale

/**
 * Structure défininssant un relevé
 * @struct 
 */
struct _releve {
  time_t temps ;		//! la date du relevé
  int val ;			//! la valeur du relevé
} ;

/**
 * Donne la température courante
 * @return une valeur t tel que : MIN_TEMPERATURE <= t <= MAX_TEMPERATURE
 */
extern int get_temperature () ;

/**
 * Creer un relevé à la dat courante
 * @return un relevé
 */
extern struct _releve get_releve () ;

extern RT_QUEUE file_releves ;		//! file de relevé
extern RT_SEM semSynchro ;		//! semaphore de synchronisation
extern RT_MUTEX mutex ;		

#define TIMERTICKS 500000000	// 500 ms

/**
 * Tache periodique effectuant les relevés toutes le 0.5s soit 500 ms
 * et les inscrits dans une file
 */

extern void releve_temperature (void *);

//! jours de la semaine
static char* jours [] = {
  "Dimanche",
  "Lundi",
  "Mardi",
  "Mercredi",
  "Jeudi",
  "Vendredi",
  "Samedi",
};

//! mois dans l'annee
static char* mois [] = {
  "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet",
  "Août", "Septembre", "Octobre", "Novembre", "Decembre"
} ;

/**
 * ecrit sur un flux un relevé
 * @pram f flux d'ecriture
 * @param r le relevé
 */
extern void ecrit_releve(FILE* f, struct _releve * r);

/**
 * Tache aperiodique effectuant la lecture de relevé depuis une file 
 * @param arg le flux d'ecriture
 */
extern void ecrit_temperature (void *arg);

//! fichier d'ecriture 
extern FILE* file_ecriture ;

/**
 * gestion des signaux du signal SIGINT (soir Crtl C) 
 * pour refermer le fichier ouvert
 * @param sig le signal gérer
 */
void gestionnaire (int sig) ;

#endif
