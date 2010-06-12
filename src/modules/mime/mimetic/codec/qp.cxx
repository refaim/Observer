/***************************************************************************
    copyright            : (C) 2002-2008 by Stefano Barbato
    email                : stefano@codesink.org

    $Id: qp.cxx,v 1.3 2008-10-07 11:06:26 tat Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "StdAfx.h"
#include <mimetic/codec/qp.h>

using namespace mimetic;


char QP::sTb[] = {
   4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 
   3, 4, 4, 3, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 

//                      %, &, ', 
   4, 4, 2, 5, 5, 5, 5, 0, 0, 0, 

// (, ), *, +, ,, -, ., /, 0, 1, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// 2, 3, 4, 5, 6, 7, 8, 9, :, ;, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// <,    >, ?,    A, B, C, D, E, 
   0, 5, 0, 0, 5, 0, 0, 0, 0, 0, 

// F, G, H, I, J, K, L, M, N, O, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// P, Q, R, S, T, U, V, W, X, Y, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// Z,             _,    a, b, c, 
   0, 5, 5, 5, 5, 0, 5, 0, 0, 0, 

// d, e, f, g, h, i, j, k, l, m, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// n, o, p, q, r, s, t, u, v, w, 
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 

// x, y, z,                      
   0, 0, 0, 5, 5, 5, 5, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
   4, 4, 4, 4, 4, 4, 
};

