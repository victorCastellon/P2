#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "pav_analysis.h"
#include "vad.h"

const float FRAME_TIME = 10.0F; /* in ms. */
const int N_TRAMAS = 9; //Numero de tramas que cogemos.

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MV", "MS"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {

  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  //feat.zcr = feat.p = feat.am = (float) rand()/RAND_MAX;
  feat.zcr = compute_zcr(x,N,16000);
  feat.am=compute_am(x,N);
  feat.p=compute_power(x,N);
  return feat;
  //Nos da un número aleatorio
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate, float alfa1, float alfa2) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->alfa1 = alfa1;
  vad_data->alfa2 = alfa2;
  vad_data->n=0;
  vad_data->p=0;
  vad_data->timer=0;
  vad_data->count=0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */

  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {
  //Esta funcion se llama una vez por trama. Tiene dos argumentos: la data (nos informa del estado) y un vector de reales.
  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   * tiene dos argumentos : vad_data --> nos informa del estado y c --> señal
   */

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) { //este es el automata
  case ST_INIT:
    //vad_data->state = ST_SILENCE;
    vad_data->count++;
    if( vad_data->n <N_TRAMAS){ //100 ES EL NUMERO DE TRAMAS QUE COJEMOS COMO SILENCIO INICIAL
      vad_data->p=vad_data->p+f.p;
      vad_data->n++;
     // printf("%d  %f\n",vad_data->n,vad_data->p);
    }else{
      vad_data->state=ST_SILENCE;
      vad_data->k0=vad_data->p/N_TRAMAS;
      printf("k0=%f\n",vad_data->k0);
      vad_data->k1 = vad_data->k0 + vad_data->alfa1;
      printf("k1=%f\n",vad_data->k1);
      vad_data->k2 = vad_data->k1 + vad_data->alfa2;
      printf("k2=%f\n",vad_data->k2);
    }

    break;

  case ST_SILENCE:
    if (f.p > vad_data->k1 ){//feature potencia
      vad_data->state = ST_MV;
      vad_data->count = 0;
    }else{
      vad_data->count++;
    }
    break;

  case ST_VOICE:
    if (f.p > vad_data->k0 && f.p <vad_data->k1){
      vad_data->state = ST_MS;
      vad_data->count = 0;
    }else{
      vad_data->count++;
    }
    break;

  case ST_MV:
    vad_data->timer = vad_data->timer + FRAME_TIME;
    if (f.p < vad_data->k0 && vad_data->timer > MINIMUM_TIME_SILENCE) {
      vad_data->state = ST_SILENCE;
      vad_data->timer = 0;
    }
     else if (f.p > vad_data->k2 && vad_data->timer > MINIMUM_TIME_VOICE) {
      vad_data->state = ST_VOICE;
      vad_data->timer = 0;
    }
    else
      vad_data->count++;
    break;

  case ST_MS:
    vad_data->timer = vad_data->timer + FRAME_TIME;
    if (f.p < vad_data->k0 && vad_data->timer > MINIMUM_TIME_SILENCE) {
      vad_data->state = ST_SILENCE;
      vad_data->timer = 0;
    }
    else if (f.p > vad_data->k2 && vad_data->timer > MINIMUM_TIME_VOICE) {
      vad_data->state = ST_VOICE;
      vad_data->timer = 0;
    }
    else
      vad_data->count++;
    break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
   return ST_UNDEF;
 //  return ST_SILENCE; 
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
