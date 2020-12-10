/*
    Copyright (C) 2020 Daniel Schultz

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "fmpz_vec.h"

void _fmpz_vec_content_chained(fmpz_t res, const fmpz * vec, slong len)
{
    while (--len >= 0)
    {
        if (fmpz_is_one(res))
            return;

        fmpz_gcd(res, res, vec + len);
    }
}