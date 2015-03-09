/* --------------------------------------------------------- */
/* --------------- A Very Short Example -------------------- */
/* --------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h> /* free() */
#include <stddef.h> /* NULL */
#include <math.h>
#include "main.h"
#include "main2.h"
#include "cmaes_interface.h"

extern SIM sim;

double run_sim( SIM *sim )
{
  int i;

  for( i = 0; sim->time < sim->duration; i++ )
    {
      controller( sim );
      save_data( sim );
      if ( sim->status == CRASHED )
	break;
      integrate_one_time_step( sim );
    }

  // write_the_mrdplot_file( sim );
  return get_score( sim );
}

void write_sampled_sol(char *filename, double const *x,
		char const *param_names[], int N) {
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if(fp == NULL) {
		printf("%sFile cannot be opened for writing: \n", filename);
		return;
	}

	fprintf(fp, "\n");
	for(i = 0; i < N; i++) {
		fprintf(fp, "%s %f %s\n", param_names[i], x[i], "opt end");
	}
	fclose(fp);
}

/* the optimization loop */
int main(int argc, char **argv) {
  cmaes_t evo; /* an CMA-ES type struct or "object" */
  double *arFunvals, *const*pop, *xfinal;
  int i;
  char *filename = "sampled_sol.par";
  char *best_filename = "best_sol.par";

  /* Initialize everything into the struct evo, 0 means default */
  arFunvals = cmaes_init(&evo, 0, NULL, NULL, 0, 0, "cmaes_initials.par");
  printf("%s\n", cmaes_SayHello(&evo));
  cmaes_ReadSignals(&evo, "cmaes_signals.par");  /* write header and initial values */

  /* Initialize the simulator */
  PARAMETER *params;
  int n_parameters;
  double init_score, new_score;
  int found_sol = 0;

  init_default_parameters( &sim );
  sim.rand_scale = 0;
  sim.controller_print = 1;

  /* Parameter file argument? */
  if ( argc > 1 )
    {
      params = read_parameter_file( argv[1] );
      n_parameters = process_parameters( params, &sim, 1 );
      if ( n_parameters > MAX_N_PARAMETERS )
	{
	  fprintf( stderr, "Too many parameters %d > %d\n",
		   n_parameters, MAX_N_PARAMETERS );
	  exit( -1 );
	}
    }

  init_sim( &sim );
  init_data( &sim );

  sim.controller_print = 0;
  init_score = run_sim( &sim );

  char const *param_names[21];
  param_names[0] = "swing_time";
  param_names[1] = "thrust1";
  param_names[2] = "swing_hip_target";
  param_names[3] = "swing_hv1";
  param_names[4] = "swing_ha1";
  param_names[5] = "swing_knee1";
  param_names[6] = "swing_kv1";
  param_names[7] = "swing_ka1";
  param_names[8] = "swing_knee_target";
  param_names[9] = "swing_kv2";
  param_names[10] = "swing_ka2";
  param_names[11] = "stance_hip_target";
  param_names[12] = "stance_hv1";
  param_names[13] = "stance_ha1";
  param_names[14] = "pitch_d";
  param_names[15] = "stance_kv1";
  param_names[16] = "stance_ka1";
  param_names[17] = "stance_knee_target";
  param_names[18] = "stance_kv2";
  param_names[19] = "stance_ka2";
  param_names[20] = "stance_ankle_torque";

  /* Iterate until stop criterion holds */
  while(!cmaes_TestForTermination(&evo))
    { 
      /* generate lambda new search points, sample population */
      pop = cmaes_SamplePopulation(&evo); /* do not change content of pop */

      /* evaluate the new search points using fitfun */
      for (i = 0; i < cmaes_Get(&evo, "lambda"); ++i) {
    	  double *sampled_sol = pop[i];
    	  reinit_sim(&sim);

    	  sim.swing_time = sampled_sol[0];
    	  sim.thrust1 = sampled_sol[1];
    	  sim.swing_hip_target = sampled_sol[2];
    	  sim.swing_hv1 = sampled_sol[3];
    	  sim.swing_ha1 = sampled_sol[4];
    	  sim.swing_knee1 = sampled_sol[5];
    	  sim.swing_kv1 = sampled_sol[6];
    	  sim.swing_ka1 = sampled_sol[7];
    	  sim.swing_knee_target = sampled_sol[8];
    	  sim.swing_kv2 = sampled_sol[9];
    	  sim.swing_ka2 = sampled_sol[10];
    	  sim.stance_hip_target = sampled_sol[11];
    	  sim.stance_hv1 = sampled_sol[12];
    	  sim.stance_ha1 = sampled_sol[13];
    	  sim.pitch_d = sampled_sol[14];
    	  sim.stance_kv1 = sampled_sol[15];
    	  sim.stance_ka1 = sampled_sol[16];
    	  sim.stance_knee_target = sampled_sol[17];
    	  sim.stance_kv2 = sampled_sol[18];
    	  sim.stance_ka2 = sampled_sol[19];
    	  sim.stance_ankle_torque = sampled_sol[20];

    	  new_score = run_sim(&sim);
    	  arFunvals[i] = new_score;
    	  if(new_score < init_score && sim.status != CRASHED) {
    		  found_sol = 1;
    	  }
      }

      /* update the search distribution used for cmaes_SamplePopulation() */
      cmaes_UpdateDistribution(&evo, arFunvals);  

      /* read instructions for printing output or changing termination conditions */ 
      cmaes_ReadSignals(&evo, "cmaes_signals.par");
      fflush(stdout); /* useful in MinGW */
      if(found_sol) {
    	  break;
      }
    }

  printf("Stop:\n%s\n",  cmaes_TestForTermination(&evo)); /* print termination reason */
  cmaes_WriteToFile(&evo, "all", "allcmaes.dat");         /* write final results */

  /* get best estimator for the optimum, xmean */
  xfinal = cmaes_GetNew(&evo, "xbest"); /* "xbestever" might be used as well */
  write_sampled_sol(best_filename, xfinal,
      			  param_names, (int) cmaes_Get(&evo, "dim"));
  cmaes_exit(&evo); /* release memory */ 

  /* do something with final solution and finally release memory */
  free(xfinal); 

  return 0;
}

