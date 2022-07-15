
#include "acb_theta.h"

void acb_theta_precomp_set(acb_theta_precomp_t D, const acb_mat_t tau,
			   const arb_eld_t E, slong prec)
{
  slong g = acb_theta_precomp_g(D);
  arb_t pi4;
  acb_t c, dc, ddc;
  slong k, j, s;
  slong step, nb_pow;

  if (arb_eld_nb_pts(E) == 0) return;

  arb_init(pi4);
  acb_init(c);
  acb_init(dc);
  acb_init(ddc);
  
  arb_pi(pi4, prec);
  arb_mul_2exp_si(pi4, pi4, -2);
  
  /* Set matrix of exponentials */
  for (k = 1; k <= g; k++)
    {
      for (j = k; j <= g; j++)
	{
	  acb_mul_arb(c, acb_mat_entry(tau,k-1,j-1), pi4, prec);
	  acb_mul_onei(c, c);
	  if (k != j) acb_mul_2exp_si(c, c, 1);
	  acb_exp(c, c);
	  acb_set(acb_mat_entry(acb_theta_precomp_exp_mat(D),k-1,j-1), c);
	}
    }

  /* Set box, steps, indices */
  step = arb_eld_step(E);
  D->step = step;
  D->indices[0] = 0;
  D->nb = 0;
  for (k = 1; k <= g; k++)
    {
      acb_theta_precomp_box(D, k) = acb_eld_box(E, k);
      nb_pow = acb_theta_precomp_box(D,k) / step + 1;
      D->indices[k] = D->indices[k-1] + nb_pow;
      D->nb += nb_pow;
    }

  /* Init and set square powers; addition chains unnecessary */
  _acb_vec_init(D->sqr_powers, D->nb);
  for (k = 1; k <= g; k++)
    {
      acb_set(ddc, acb_mat_entry(acb_theta_precomp_exp_mat(D),k-1,k-1));      
      s = acb_theta_precomp_box(D, k) % step;
      acb_pow_si(c, ddc, s, prec);
      acb_pow_si(dc, ddc, 2*s*step + step*step, prec);
      acb_pow_si(ddc, ddc, 2*step*step, prec);
      for (j = 0; s + step*j <= acb_theta_precomp_box(D,k); j++)
	{
	  acb_set(acb_theta_precomp_sqr_pow(D,k,j), c);
	  acb_mul(c, c, dc, prec);
	  acb_mul(dc, dc, ddc, prec);
	}
    }

  arb_clear(pi4);
  acb_clear(c);
  acb_clear(dc);
  acb_clear(ddc);
}
