// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2021 Amol Surati

#ifndef BITS_H
#define BITS_H

#define bits_size(f)			(1ul << f##_BITS)
#define bits_mask(f)			(bits_size(f) - 1)
#define bits_set(f, v)			(((v) & bits_mask(f)) << f##_POS)
#define bits_get(v, f)			(((v) >> f##_POS) & bits_mask(f))
#define bits_push(f, v)			((v) & (bits_mask(f) << f##_POS))
#define bits_pull(v, f)			bits_push(f, v)
#define bits_on(f)			(bits_mask(f) << f##_POS)
#define bits_off(f)			~bits_on(f)
#endif
