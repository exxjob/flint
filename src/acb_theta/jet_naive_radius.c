/*
    Copyright (C) 2023 Jean Kieffer

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_theta.h"

void
acb_theta_jet_naive_radius(arf_t R2, arf_t eps, arb_srcptr offset,
    const arb_mat_t cho, slong ord, slong prec)
{
    slong g = arb_mat_nrows(cho);
    slong lp = ACB_THETA_LOW_PREC;
    arb_mat_t mat;
    arb_t na, nx, t, u;
    arf_t cmp;
    mag_t norm;

    arb_mat_init(mat, g, g);
    arb_init(na);
    arb_init(nx);
    arb_init(t);
    arb_init(u);
    arf_init(cmp);
    mag_init(norm);

    /* Get norms of cho^{-1} and offset */
    arb_mat_one(mat);
    arb_mat_solve_triu(mat, cho, mat, 0, lp);
    arb_mat_bound_inf_norm(norm, mat);
    arf_set_mag(arb_midref(na), norm);
    _arb_vec_get_mag(norm, offset, g);
    arf_set_mag(arb_midref(nx), norm);

    /* Get R2, eps assuming R <= nx/na */
    acb_theta_naive_radius(R2, eps, cho, 0, prec);
    arb_mul_2exp_si(t, nx, 1);
    arb_one(u);
    arb_max(t, t, u, lp);
    arb_pow_ui(t, t, ord, lp);
    arb_mul_arf(t, t, eps, lp);
    arb_get_ubound_arf(eps, t, lp);

    /* If R too large, assume R >= nx/na instead */
    arb_div(t, nx, na, lp);
    arb_sqr(t, t, lp);
    arb_get_lbound_arf(cmp, t, lp);
    if (arf_cmp(cmp, R2) <= 0)
    {
        acb_theta_naive_radius(R2, eps, cho, ord, prec);
        arb_div(t, nx, na, lp);
        arb_get_ubound_arf(cmp, t, lp);
        arf_max(R2, R2, cmp);
        arb_mul_2exp_si(t, na, 1);
        arb_one(u);
        arb_max(t, t, u, lp);
        arb_pow_ui(t, t, ord, lp);
        arb_mul_arf(t, t, eps, lp);
        arb_get_ubound_arf(eps, t, lp);
    }

    arb_mat_clear(mat);
    arb_clear(na);
    arb_clear(nx);
    arb_clear(t);
    arb_clear(u);
    arf_clear(cmp);
    mag_clear(norm);
}
