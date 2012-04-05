/**
 * @file Simulation d'un relevé périodique de température en tamps réel
 * @author Diallo Algassimou , Andoni olen
 */

#include "temperature.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>

#include <native/timer.h>
#include <native/task.h>

FILE* file_ecriture = NULL ;

void gestionnaire (int sig) {
  if (file_ecriture != NULL)
    fclose (file_ecriture);
  
  exit (0);
}

int get_temperature () {
  int tmp = rand () % (MAX_TEMPERATURE - MIN_TEMPERATURE) ; 
  return MIN_TEMPERATURE + tmp ;
}

struct _releve get_releve () {
  struct _releve r ;
  if (time(&(r.temps)) == ((time_t)-1)) {
    perror ("Erreur prise de temps");
    exit (1);
  }
  r.val = get_temperature() ;
  return r ;
}

RT_QUEUE file_releves ;		//! file de relevé
RT_SEM semSynchro ;		//! semaphore de synchronisation
RT_MUTEX mutex ;		

void releve_temperature (void *arg) {  
  // On rend la tache periodique
  if (rt_task_set_periodic (NULL, TM_NOW, TIMERTICKS) != 0) {
    fprintf (stderr , "Erreur sur rt_task_set_periodic\n");
    return ;
  }

  // boucle de travail
  while (1) {
    struct _releve r = get_releve() ;

    rt_mutex_acquire(&mutex, TM_INFINITE);
    rt_queue_write(&file_releves, &r, sizeof(struct _releve), Q_NORMAL);
    rt_mutex_release(&mutex);

    rt_sem_v(&semSynchro);
    rt_task_wait_period (NULL);
  }
}

void ecrit_releve(FILE* f, struct _releve * r){
  struct tm *t = localtime(&(r->temps));
  fprintf (f , "Date: %s %d %s %d Heure : %d h %d min %d sec temperature %d\n", 
	   jours[t->tm_wday],
	   t->tm_mday,
	   mois[t->tm_mon],
	   1900 + t->tm_year,
	   t->tm_hour,
	   t->tm_min,
	   t->tm_sec,
	   r->val);
}

void ecrit_temperature (void *arg) {
  // on cast le flux

  FILE * f = (FILE*) arg ;

  while (1) {
    struct _releve r ;
    rt_sem_p(&semSynchro , TM_INFINITE);
    
    rt_mutex_acquire(&mutex, TM_INFINITE);
    rt_queue_read(&file_releves, &r, sizeof(struct _releve), TM_INFINITE);
    rt_mutex_release(&mutex);
    ecrit_releve (f, &r);
  }
}


int main (int argc, char*argv[]) {
  RT_TASK releve, ecriture;  
  
  if (argc != 2) {
    fprintf (stderr , "Usage %s fichier\n", argv[0]);
    return -1 ;
  }

  if ((file_ecriture = fopen (argv[1], "w")) == NULL) {
    perror ("Erreur") ;
    return -1 ;
  }

  if (signal (SIGINT , gestionnaire) == SIG_ERR) {
    fprintf (stderr, "Impossible d'etablir le gestionnaire de signal\n");
    return -1 ;
  }

  // création du semaphore de synchronisation
  if (rt_sem_create(&semSynchro, "semaphore de synchronisation", 0, S_FIFO) !=0){
    fprintf (stderr , "Erreur création de semphore de synchronisation\n");
    return -1 ;
  }

  // création du mutex d'exclusion mutuelle
  if (rt_mutex_create(&mutex, "exclusion mutuelle") !=0){
    fprintf (stderr , "Erreur création du mutex\n");
    return -1 ;
  }

  //on cree la file de message
  if (rt_queue_create (&file_releves, "file de relevé", 100 * sizeof (struct _releve),
		       Q_UNLIMITED, Q_FIFO) != 0) {

    fprintf (stderr , "Erreur création de la file\n");
    return -1 ;
  }

  // on bloque les pages mémoire de nos taches
  mlockall(MCL_CURRENT | MCL_FUTURE);
  
  if (rt_task_spawn (&ecriture, "Tache aperiodique", 2000, 
		     98, T_JOINABLE, &ecrit_temperature, file_ecriture) != 0) {
    fprintf (stderr , "Impossible de creer la tâche ap\n") ;
    return -1 ;
  }

  if (rt_task_spawn (&releve, "Tache periodique", 2000, 
		     99, T_JOINABLE, &releve_temperature, NULL) != 0) {
    fprintf (stderr , "Impossible de creer la tâche p\n") ;
    return -1 ;
  }
  
  rt_task_join(&releve);
  rt_task_join(&ecriture);
  return 0 ;
}
