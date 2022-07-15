#if 0

#include "acb_theta.h"

/* Work in dimension 1: add partial exponential sum into th, with two
   multiplications per term only, at just the necessary precision.
   Each term is: cofactor * (to_lin_power)^k * x^(k^2), and square
   powers of x are precomputed
*/
static void worker_dim_1(acb_t th, const arb_eld_t E, acb_srcptr sqr_powers_precomp,
			 const acb_t to_lin_power, const acb_t cofactor, slong prec,
			 slong fullprec)
{
  acb_t start, diff, lin, term, sum;
  slong min = arb_eld_min(E);
  slong mid = arb_eld_mid(E);
  slong max = arb_eld_max(E);
  slong step = arb_eld_step(E);
  slong newprec;
  slong k;

  if (arb_eld_nb_pts(E) == 0) {return;}

  acb_init(start);
  acb_init(diff);
  acb_init(lin);
  acb_init(term);
  acb_init(sum);

  acb_zero(sum);
  acb_pow_si(start, to_lin_power, mid, prec);
  acb_pow_si(diff, to_lin_power, step, prec);
  
  acb_set(lin, start);
  for (k = mid; k <= max; k += step)
    {
      newprec = acb_theta_naive_newprec(prec, k-mid, max-mid, step);
      acb_mul(term, lin, &sqr_powers_precomp[FLINT_ABS(k)/step], newprec);
      acb_add(sum, sum, term, prec);
      if (k < max) acb_mul(lin, lin, diff, newprec);
    }

  acb_set(lin, start);
  acb_inv(diff, diff, prec);
  for (k = mid - step; k >= min; k -= step)
    {
      newprec = acb_theta_naive_newprec(prec, mid-k, mid-min, step);      
      acb_mul(lin, lin, diff, newprec);
      acb_mul(term, lin, &sqr_powers_precomp[FLINT_ABS(k)/step], newprec);
      acb_add(sum, sum, term, prec);
    }

  acb_mul(sum, sum, cofactor, prec);
  acb_add(th, th, sum, fullprec);

  acb_clear(start);
  acb_clear(diff);
  acb_clear(lin);
  acb_clear(term);
  acb_clear(sum);  
}

/* Recursive worker in dimension d: entries (i,j) of lin_powers for
   j>d are considered const
*/   
static void worker_rec(acb_t th, acb_mat_t lin_powers,
		       const arb_eld_t E, acb_srcptr sqr_powers_precomp,
		       const acb_mat_t exp_mat, const acb_t cofactor,
		       slong prec, slong fullprec)
{
  slong d = arb_eld_dim(E);
  slong g = arb_eld_ambient_dim(E);
  slong nr = arb_eld_nr(E);
  slong nl = arb_eld_nl(E);
  slong min = arb_eld_min(E);
  slong mid = arb_eld_mid(E);
  slong max = arb_eld_max(E);
  slong step = arb_eld_step(E);
  acb_t start_cf, diff_cf, new_cf;
  acb_ptr start_lin_powers, diff_lin_powers;
  slong newprec;
  slong k;

  if (arb_eld_nb_pts(E) == 0) {return;}

  acb_init(start_cf);
  acb_init(diff_cf);
  acb_init(new_cf);
  start_lin_powers = _acb_vec_init(d-1);
  diff_lin_powers = _acb_vec_init(d-1);
  
  /* Set up things for new cofactor */
  acb_one(diff_cf);
  for (k = d+1; k <= g; k++)
    {
      acb_mul(diff_cf, diff_cf, acb_mat_entry(lin_powers, d-1, k-1), prec);
    }
  acb_pow_si(start_cf, diff_cf, mid, prec);
  acb_mul(start_cf, start_cf, cofactor, prec);
  acb_pow_si(diff_cf, diff_cf, step, prec);

  /* Set up things to update entries (k,d) of lin_powers, k < d */
  for (k = 1; k < d; k++)
    {
      acb_pow_si(&diff_lin_powers[k-1], acb_mat_entry(exp_mat, k-1, d-1), step, prec);
      acb_pow_si(&start_lin_powers[k-1], acb_mat_entry(exp_mat, k-1, d-1), mid, prec);
    }

  /* Loop over children */
  for (k = 1; k < d; k++) acb_set(acb_mat_entry(lin_powers, k-1, d-1), &start_lin_powers[k-1]);
  acb_set(new_cf, start_cf);
  
  for (k = 0; k < nr; k++)
    {
      worker_rec(th, lin_powers, arb_eld_rchild(E,k), 
      
      if (d == 2) worker_dim_1(th, arb_eld_rchild(E, k), sqr_powers_precomp,
			       
    }

  for (k = 0; k < nl; k++)
    {

    }

  
}

/* Given an ellipsoid slice of dimension d, 
   - enumerate lattice slices of dimension d-1 contained in it,
   - gather the corresponding partial sums of exponentials,
   - write result to th.
   Arguments:
   - d: current dimension
   - Y: Cholesky decomposition of Im(tau), lattice is generated by d first colums of Y
   - offset: vector of length d
   - Rsqr: current ellipsoid square-radius
   - qmat: upper triangular matrix containing q_{ij}^(1/2) (both i,j\leq d, except 1/4 if i=j), 
   q_{ij}^{n_j} (i\leq d, j>d) or undefined (both >d)
   - cofactor: product of q_{ii}^{n_i^2}, and q_{ij}^{2n_in_j}, for d<i<j
   - prec_th: precision for theta series computations
   - prec_R: precision for ellipsoid computations, << prec_th, hopefully negligible
*/

static void
recursive_worker(acb_t th,
		 ulong ab,
		 const arb_mat_t Y,
		 arb_srcptr offset,
		 const arb_t Rsqr,
		 const acb_mat_t qmat,
		 const acb_t cofactor,
		 slong d,
		 slong prec_th,
		 slong prec_R)
{
  arb_t rad, ctr;
  int ad;
  slong nmin, nmax, nmid;
  acb_t q, dq, ddq;
  acb_ptr dq_vec;
  acb_t mul, dmul;
  slong next_prec;
  arb_ptr next_offset;
  arb_t c;
  arb_t next_Rsqr;
  acb_mat_t next_qmat;
  acb_t next_cofactor;
  slong g = arb_mat_nrows(Y);
  slong n;
  slong k;

  /* Init all */

  /* Compute upper and lower bounds for interval where n_d + a_d/2 can lie */
  arb_sqrt(rad, Rsqr, prec_R);
  arb_div(rad, rad, arb_mat_entry(Y, d, d), prec_R);
  arb_div(ctr, &offset[d], arb_mat_entry(Y, d, d), prec_R);
  arb_neg(ctr, ctr);

  /* Deduce integer bounds for n_d: separate into two halves, from
     nmin to nmid1 then nmid2 to nmax */
  ad = acb_theta_char_a_bit(d);
  acb_theta_naive_set_interval(&nmin, &nmid, &nmax, ctr, rad, ad, prec_R);

  if (nmin > nmax)
    {
      /* No lattice point in interval: set to zero and exit. */
      acb_zero(th);
      goto exit;
    }  
  /* Now recursive loops: nmid to nmin, nmid to nmax. */
  
  /* Set up radius, precision, offset at nmid */
  acb_theta_naive_next_Rsqr(next_Rsqr, Rsqr, acb_mat_entry(Y, d, d), ctr, nmid, ad, prec_R);
  next_prec = acb_theta_naive_next_prec(prec, Rsqr, next_Rsqr, prec_R);
  arb_set_si(c, 2*nmid+ad);
  arb_div_si(c, c, 2, prec_R);
  for (k = 0; k < d-1; k++) arb_set(&next_offset[k], arb_mat_entry(Y, k, d-1));
  _arb_vec_scalar_mul(next_offset, next_offset, c, prec_R);
  _arv_vec_add(next_offset, next_offset, offset, d-1, prec_R);
      
  /* Set up new matrix entries at nmid */
  acb_mat_set(next_qmat, qmat);
  acb_theta_naive_qn2(acb_mat_entry(next_qmat, d, d),
		      dq_up, ddq_up, dq_down, ddq_down,
		      acb_mat_entry(qmat, d, d), 2*nmid + ad, next_prec);
  for (k = 0; k < d; k++)
    {
      acb_pow_si(acb_mat_entry(next_qmat, k, d),
		 acb_mat_entry(qmat, k, d), 2*nmid + ad, next_prec);
      acb_pow_si(&dq_vec[k], acb_mat_entry(qmat, k, d), 2, next_prec);
    }
  acb_one(dmul);
  for (k = d+1; k < g; k++)
    {
      acb_mul(dmul, dmul, acb_mat_entry(qmat, d, k), next_prec);
    }
  acb_pow_si(mul, dmul, 2*nmid + ad, next_prec);
  acb_sqr(dmul, dmul, next_prec);
  acb_mul(next_cofactor, cofactor, mul, next_prec);
  acb_mul(next_cofactor, next_cofactor, acb_mat_entry(next_qmat, d, d), next_prec);

  /* Process slices nmid to nmax */
  for (k = nmid; k <= nmax; k++)
    {
      if (d >= 2)
	{
	  recursive_worker(th, ab, Y, next_offset, next_Rsqr, next_qmat, next_cofactor, d-1,
			   next_prec, prec_R);
	}
      else /* d=1: just add cofactor to th, with sign depending on b */
	{
	  acb_theta_naive_sign(c, n_vec, ab);
	  acb_addmul(th, th, next_cofactor, c, next_prec);
	}
      /* Update data for next step */
      if (k < nmax)
	{
	  acb_theta_naive_next_Rsqr(next_Rsqr, acb_mat_entry(Y, d-1, d-1), ctr, k+1,
				    ad, prec_R);
	  next_prec = acb_theta_naive_next_prec(prec, Rsqr, next_Rsqr, prec_R);
	  _acb_vec_add(next_offset, next_offset, add_offset, d-1, prec_R);
	  
	}
    }

  /* Re-setup things for nmid-1 */

  /* Process slides nmid-1 to nmin */
  for (k = 1; k <= nmid - nmin; k++)
    {
      if (d >= 2)
	{
	  recursive_worker(th, ab, Y, next_offset, next_Rsqr, next_qmat, next_cofactor, d-1,
			   next_prec, prec_R);
	}
      else /* d=1: just add cofactor to th, with sign depending on b */
	{
	  acb_theta_naive_sign(c, n_vec, ab);
	  acb_addmul(th, th, next_cofactor, c, next_prec);
	}
      /* Update data for next step */
    }

 exit:
  {
    /* Clear all */
  }
}
		 

void acb_theta_const_ind_naive(acb_t th, ulong ab, const acb_mat_t tau, const acb_theta_enum_t en, slong prec)
{
  arb_ptr v;
  /* Y: Cholesky decomposition */
  /* R: Choice of radius */
  
  slong g = acb_mat_nrows(tau);
  slong d;

  v = _arb_vec_init(g);

  /* Consider coordinate number d:  */
  /* Current vector is v=zero */
  /* Current radius is rad=R */
  
  arb_set(gamma, arb_mat_entry(Y, d, d));
  
  arb_add(ubound, offset, bd, prec);
  arb_sub(lbound, offset, bd, prec);
  
  /* Now consider all elements in Z or Z+1/2 inside that interval */
  /* We want to add q_g^(n^2) to the theta sum */
  newprec = prec; /* Adjust for each term as a function of current norm */

  /* Just consider a=0 for now. */
  nmin = low_end(lbound);
  nmax = high_end(ubound);
  
  acb_set(q, acb_mat_entry(tau, d, d));
  acb_set(q_trans, q);
  
  acb_pow_si(q, q, nmin*nmin, newprec);
  acb_pow_si(q_trans, q_trans, 2*nmin+1, newprec);
  /* For precision issues, we have to distinguish left and right parts always */
  /* We also need all quantities like q_1g to the n; put them in a matrix */

  for (k = 0; k <= nmax - nmin; k++)
    {
      /* We know the current q_gg^(n^2), q_ig^n; n = nmin+k */
      /* Set up recursive call */
      
      /* Compute new radius */
      /* New offset is a vector of length d-1, obtained by adding n*(last column) to previous offset */
      arb_mul(term, qgg, recursive_call(), newprec);
      arb_add(th, th, term, prec);
      /* Just a recursive call that adds the required quantities to the vector */
      /* Transition qgg, etc. */
    }

  
}
  

#endif
