
#include "acb_theta.h"

void acb_theta_agm_ext(acb_t r, acb_srcptr a, acb_srcptr all_roots, const arf_t rel_err,
		       slong nb_bad, slong nb_good, slong g, slong prec)
{  
  acb_ptr v;
  acb_t exp;
  arb_t abs;
  arf_t err;
  slong lowprec = ACB_THETA_AGM_LOWPREC;
  slong n = 1<<g;
  slong k;

  v = _acb_vec_init(2*n);
  acb_init(exp);
  arb_init(abs);
  arf_init(err);
  
  _acb_vec_set(v, a, 2*n);
  
  for (k = 0; k < nb_bad; k++)
    {
      acb_theta_agm_ext_step_bad(v, v, all_roots + k*2*n, g, prec);
    }
  for (k = 0; k < nb_good; k++)
    {
      acb_theta_agm_ext_step_good(v, v, g, prec);
    }

  acb_div(r, &v[0], &v[n], prec);
  acb_one(exp);
  acb_mul_2exp_si(exp, exp, nb_good + nb_bad);
  acb_pow(r, r, exp, prec);

  acb_abs(abs, r, lowprec);
  arb_get_ubound_arf(err, abs, lowprec);
  arf_mul(err, err, rel_err, lowprec, ARF_RND_CEIL);
  acb_add_error_arf(r, err);  

  _acb_vec_clear(v, n);
  acb_clear(exp);
  arb_clear(abs);
  arf_clear(err);
}
