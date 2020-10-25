#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F;       /* in ms. */
const double INIT_FRAMES = 12;         /* in frames */
const float ALPHA1 = 3;               /* threshold over noise for k1 */
const float ALPHA2 = 10;              /* threshold over noise for k2 */
const double MIN_FRAMES_SV = 7;       /* Minimun time to change from silence to voice */
const double MIN_FRAMES_VS = 5;      /* Minimun time to change from voice to silence */

/* 
   As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
   only this labels are needed. You need to add all labels, in case
   you want to print the internal state in string format */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MV" ,"MS"
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

Features compute_features(const float *x, int N, float sampling_rate) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  Features feat;
  /*Compute Zero Crossing rate, average power in dB and average amplitude of x[]*/
  //feat.zcr = compute_zcr(x,N,sampling_rate);

  /* Compute average power in dB*/
  feat.p = compute_power(x,N);

  /*Compute average amplitude*/
  //feat.am = compute_am(x,N);
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->lMinSilence = MIN_FRAMES_VS;
  vad_data->lMinVoice = MIN_FRAMES_SV;
  vad_data->initFrames = INIT_FRAMES;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  switch(vad_data->state){
    case ST_MVOICE:
      vad_data->lMinVoice -= 1;
      if((vad_data->last_feature > vad_data->k2) && (vad_data->lMinVoice >= 0)){
        vad_data->state = ST_VOICE;
        break;
      }
      vad_data->state = ST_SILENCE;
      break;
    case ST_MSILENCE:
      vad_data->lMinSilence -= 1;
      if ((vad_data->last_feature > vad_data->k1)  && (vad_data->lMinSilence >= 0)){
        vad_data->state = ST_VOICE;
        break;
      }
      vad_data->state = ST_SILENCE;
      break;
    case ST_SILENCE:
      break;
    case ST_VOICE:
      break;
    case ST_UNDEF:
      break;
    case ST_INIT:
      break;  
  }

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

  /* TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length , vad_data->sampling_rate);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:
    vad_data->initFrames -= 1;
    vad_data->k0 += pow(10,(vad_data->last_feature)/10);
    if(vad_data->initFrames <= 0){
      vad_data->k0 = 10*log10((vad_data->k0)/INIT_FRAMES);
      vad_data->k1 = vad_data->k0 + ALPHA1;
      vad_data->k2 = vad_data->k0 + ALPHA2;
      vad_data->state = ST_SILENCE;
      break;
    } 
    break;

  case ST_SILENCE:
    if (vad_data->last_feature > vad_data->k1){
      vad_data->state = ST_MVOICE;
    } 
    if(vad_data->lMinVoice != MIN_FRAMES_SV){
      vad_data->lMinVoice = MIN_FRAMES_SV;
    }
    break;

  case ST_VOICE:
    if (vad_data->last_feature < vad_data->k1){
      vad_data->state = ST_MSILENCE;
    }  
    if(vad_data->lMinSilence != MIN_FRAMES_VS){
       vad_data->lMinSilence = MIN_FRAMES_VS;
    } 
    break;

  case ST_MVOICE:
    vad_data->lMinVoice -= 1;
    if((vad_data->last_feature > vad_data->k2) && (vad_data->lMinVoice >= 0)){
      vad_data->state = ST_VOICE;
      break;
    }  
    if((f.p > vad_data->k1) && (vad_data->lMinVoice >= 0)){
      break;
    }  
    vad_data->state = ST_SILENCE;
    break;
  
  case ST_MSILENCE:
    vad_data->lMinSilence -= 1;
    if ((vad_data->last_feature > vad_data->k1)  && (vad_data->lMinSilence >= 0)){
      vad_data->state = ST_VOICE;
      break;
    }  
    if(vad_data->lMinSilence < 0 ){
      vad_data->state = ST_SILENCE;
      break;
    }  
    break;
  
  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
