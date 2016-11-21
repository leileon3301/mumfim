/******************************************************************************

  (c) 2004-2010 Scientific Computation Research Center,
      Rensselaer Polytechnic Institute. All rights reserved.

  The LICENSE-SCOREC file included with this distribution describes the terms
  of the SCOREC Non-Commercial License this program is distributed under.

*******************************************************************************/

/**
 * @brief     SCORECUtil
 * @file      file name (relative path should be included e.g. /cint/FMDB.h)
 *
 * @brief     Gauss Quadrature functions for triangles.
 *
 */

#include "GaussQuadrature.h"
#include "IntPt.h"
#include "GaussLegendreSimplex.h"

IntPt2d GQT1[1] = { {{.333333333333333,.333333333333333},1.0} };
IntPt2d GQT2[3] = {
	{ {0.16666666666666,0.1666666666666},.333333333333333 },
	{ {0.66666666666666,0.1666666666666},.333333333333333 },
	{ {0.16666666666666,0.6666666666666},.333333333333333 }
};

IntPt2d GQT3[4] = {
	{ {0.333333333333333,0.3333333333333333},-0.5625},
	{ {0.6,0.2},.520833333333333 },
	{ {0.2,0.6},.520833333333333 },
	{ {0.2,0.2},.520833333333333 }
};

IntPt2d GQT4[6] = {
	{ {0.816847572980459,0.091576213509771},0.109951743655322},
	{ {0.091576213509771,0.816847572980459},0.109951743655322},
	{ {0.091576213509771,0.091576213509771},0.109951743655322},
	{ {0.108103018168070,0.445948490915965},0.223381589678011},
	{ {0.445948490915965,0.108103018168070},0.223381589678011},
	{ {0.445948490915965,0.445948490915965},0.223381589678011}
};

IntPt2d GQT5[7] = {
	{ {0.333333333333333,0.333333333333333},0.225000000000000},
	{ {0.797426985353087,0.101286507323456},0.125939180544827},
	{ {0.101286507323456,0.797426985353087},0.125939180544827},
	{ {0.101286507323456,0.101286507323456},0.125939180544827},
	{ {0.470142064105115,0.059715871789770},0.132394152788506},
	{ {0.059715871789770,0.470142064105115},0.132394152788506},
	{ {0.470142064105115,0.470142064105115},0.132394152788506}
};

IntPt2d GQT6[12] = {
  { {0.873821971016996,0.063089014491502},0.050844906370207},
  { {0.063089014491502,0.873821971016996},0.050844906370207},
  { {0.063089014491502,0.063089014491502},0.050844906370207},
  { {0.501426509658179,0.249286745170910},0.116786275726379},
  { {0.249286745170910,0.501426509658179},0.116786275726379},
  { {0.249286745170910,0.249286745170910},0.116786275726379},
  { {0.636502499121399,0.310352451033785},0.082851075618374},
  { {0.310352451033785,0.636502499121399},0.082851075618374},
  { {0.636502499121399,0.053145049844816},0.082851075618374},
  { {0.310352451033785,0.053145049844816},0.082851075618374},
  { {0.053145049844816,0.310352451033785},0.082851075618374},
  { {0.053145049844816,0.636502499121399},0.082851075618374}
};

IntPt2d GQT7[13] = {
	{ {0.333333333333333,0.333333333333333},-0.149570044467682},
	{ {0.479308067841920,0.260345966079040},0.175615257433208},
	{ {0.260345966079040,0.479308067841920},0.175615257433208},
	{ {0.260345966079040,0.260345966079040},0.175615257433208},
	{ {0.869739794195568,0.065130102902216},0.053347235608838},
	{ {0.065130102902216,0.869739794195568},0.053347235608838},
	{ {0.065130102902216,0.065130102902216},0.053347235608838},
	{ {0.048690315425316,0.312865496004874},0.077113760890257},
	{ {0.312865496004874,0.048690315425316},0.077113760890257},
	{ {0.638444188569810,0.048690315425316},0.077113760890257},
	{ {0.048690315425316,0.638444188569810},0.077113760890257},
	{ {0.312865496004874,0.638444188569810},0.077113760890257},
	{ {0.638444188569810,0.312865496004874},0.077113760890257}
};

IntPt2d GQT8[16] = {
	{ {0.333333333333333,0.333333333333333},0.144315607677787},
	{ {0.081414823414554,0.459292588292723},0.095091634267285},
	{ {0.459292588292723,0.081414823414554},0.095091634267285},
	{ {0.459292588292723,0.459292588292723},0.095091634267285},
	{ {0.658861384496480,0.170569307751760},0.103217370534718},
	{ {0.170569307751760,0.658861384496480},0.103217370534718},
	{ {0.170569307751760,0.170569307751760},0.103217370534718},
	{ {0.898905543365938,0.050547228317031},0.032458497623198},
	{ {0.050547228317031,0.898905543365938},0.032458497623198},
	{ {0.050547228317031,0.050547228317031},0.032458497623198},
	{ {0.008394777409958,0.728492392955404},0.027230314174435},
	{ {0.728492392955404,0.008394777409958},0.027230314174435},
	{ {0.263112829634638,0.008394777409958},0.027230314174435},
	{ {0.008394777409958,0.263112829634638},0.027230314174435},
	{ {0.263112829634638,0.728492392955404},0.027230314174435},
	{ {0.728492392955404,0.263112829634638},0.027230314174435}
};


IntPt2d * GQT[9] = {GQT1,GQT1,GQT2,GQT3,GQT4,GQT5,GQT6,GQT7,GQT8};
IntPt2d * GQTdegen[17] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int GQTnPt[9] = {1,1,3,4,6,7,12,13,16};

IntPt2d *getGQTPts(int order)
{
	if (order < 7) {
		return GQT[order];
	}

	int n = (order + 3) / 2;
	int index = n - 4;
	if (!GQTdegen[index]) {
		int npts = n*n;
		GQTdegen[index] = new IntPt2d[npts];
		GaussLegendreTri(n,n,GQTdegen[index]);
	}

	return GQTdegen[index];
}

int getNGQTPts(int order)
{
  if(order < 7)
    return GQTnPt[order];
  else
    return ((order + 3) / 2) * ((order + 3) / 2);
}

