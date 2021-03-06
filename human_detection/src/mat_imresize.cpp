/***************************************************************************
* IMRESAMPLE.C
*
* Fast bilinear down/up sampling. Inspired by resize.cpp from Deva Ramanan.
*
* Piotr's Image&Video Toolbox      Version NEW
* Copyright 2009 Piotr Dollar.  [pdollar-at-caltech.edu]
* Please email me if you find bugs, or have suggestions or questions!
* Licensed under the Lesser GPL [see external/lgpl.txt]
****************************************************************************/
#include "mat_imresize.h"
#include <math.h>
#include <cassert>
#include <vector>
using namespace std;

/* struct used for caching interpolation values for single column */
typedef struct { int yb, ya0, ya1; double wt0, wt1; } InterpInfo;

int interpInfoDn( int ha, int hb, vector<InterpInfo>& ii)
{
  /* compute interpolation values for single column for DOWN sampling */
  int c, ya, yb, ya0, ya1; double sc, scInv, ya0f, ya1f;
  sc = (double)hb/(double)ha; scInv = 1.0/sc;
  ya0f=0; ya1f=scInv; c=0;
  ii.resize((int)ceil(hb*scInv)+2*hb);
  for( yb=0; yb<hb; yb++ ) {
    ya0=(int)ceil(ya0f); ya1=(int)floor(ya1f);
    if( ya0-ya0f > 1e-3 ) { /* upper elt contributing to yb */
      ii[c].yb=yb; ii[c].ya0=ya0-1; ii[c].wt0=(ya0-ya0f)*sc; c++; }
    for( ya=ya0; ya<ya1; ya++ ) { /* main elts contributing to yb */
      ii[c].yb=yb; ii[c].ya0=ya; ii[c].wt0=sc; c++; }
    if( ya1f-ya1 > 1e-3 ) { /* lower elt contributing to yb */
      ii[c].yb=yb; ii[c].ya0=ya1; ii[c].wt0=(ya1f-ya1)*sc; c++; }
    ya0f=ya1f; ya1f+=scInv;
  }
  return c;
}

int interpInfoUp( int ha, int hb, vector<InterpInfo>& ii)
{
  /* compute interpolation values for single column for UP sampling */
  int ya, yb; double sc, scInv, yaf;
  sc = (double)hb/(double)ha; scInv = 1.0/sc; yaf=.5*scInv-.5;
  ii.resize(hb);
  for( yb=0; yb<hb; yb++ ) {
    ii[yb].yb=yb; ya=(int) floor(yaf);
    if(ya<0) { /* if near start of column */
      ii[yb].wt1=0; ii[yb].ya0=ii[yb].ya1=0;
    } else if( ya>=ha-1 ) { /* if near end of column */
      ii[yb].wt1=0; ii[yb].ya0=ii[yb].ya1=ha-1;
    } else { /* interior */
      ii[yb].wt1=yaf-ya; ii[yb].ya0=ya; ii[yb].ya1=ya+1;
    }
    ii[yb].wt0=1.0-ii[yb].wt1; yaf+=scInv;
  }
  return hb;
}

void      resample( double *A, double *B, int ha, int hb, int w, int nCh )
{
  /* resample every column in A, store in B, result is transposed */
  int n, ch, x, y; double *a, *b;
  const bool downsample=(hb<=ha);
  vector<InterpInfo> ii;
  if (downsample)
    n = interpInfoDn(ha,hb,ii);
  else
    n = interpInfoUp(ha,hb,ii);

  for(x=0; x<n; x++) ii[x].yb*=w; /* transpose B */
  for(ch=0; ch<nCh; ch++) for(x=0; x<w; x++) {
    a = A + ch*w*ha + x*ha;
    b = B + ch*w*hb + x;
    if( downsample )
      for(y=0; y<n; y++) b[ii[y].yb]+=ii[y].wt0*a[ii[y].ya0];
    else
      for(y=0; y<n; y++)
        b[ii[y].yb]=a[ii[y].ya0]*ii[y].wt0+a[ii[y].ya1]*ii[y].wt1;
  }
}



