/* Compares hijri_yallop_evaluate_evening's q against Yallop's own published
 * q_yallop for the TN69 evening rows (Task 1's tn69-full.csv), and gates on
 * the maximum residual per halt condition 2. Not a test binary: it is built
 * to build/compare_q, outside the tests tree, so it never joins make check.
 * Its input and output live in the scratchpad, this file carries no data. */

#define HIJRI_IMPLEMENTATION
#include "hijri.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Same zone boundaries as hijri_yallop_classify, applied directly to
 * q_yallop instead of a q recomputed from arcv/width. */
static HijriYallopZone zone_from_q(double q) {
  if (q > 0.216) return HIJRI_YALLOP_A_EASILY_VISIBLE;
  if (q > -0.014) return HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS;
  if (q > -0.160) return HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID;
  if (q > -0.232) return HIJRI_YALLOP_D_NEEDS_OPTICAL_AID;
  if (q > -0.293) return HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE;
  return HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT;
}

/* Single-letter zone codes for the --emit-zones fixture column. */
static char zone_letter(HijriYallopZone zone) {
  switch (zone) {
    case HIJRI_YALLOP_A_EASILY_VISIBLE: return 'A';
    case HIJRI_YALLOP_B_VISIBLE_PERFECT_CONDITIONS: return 'B';
    case HIJRI_YALLOP_C_MAY_NEED_OPTICAL_AID: return 'C';
    case HIJRI_YALLOP_D_NEEDS_OPTICAL_AID: return 'D';
    case HIJRI_YALLOP_E_NOT_VISIBLE_TELESCOPE: return 'E';
    case HIJRI_YALLOP_F_NOT_VISIBLE_BELOW_LIMIT: return 'F';
  }
  return '?';
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <tn69-full.csv> [--emit-zones <output.csv>]\n",
            argv[0]);
    return 2;
  }

  /* --emit-zones writes year,month,day,lat_deg,lon_deg,zone for every
   * evening row, in the same row order the input file presents them, so
   * extract.py's --emit-fixture mode can pair them positionally without
   * re-deriving the zone itself. It does not change the gating exit-status
   * contract below: the residual halt still fires the same way. */
  FILE *zones_out = NULL;
  if (argc >= 4 && strcmp(argv[2], "--emit-zones") == 0) {
    zones_out = fopen(argv[3], "w");
    if (!zones_out) {
      fprintf(stderr, "cannot open %s for writing\n", argv[3]);
      return 2;
    }
    fprintf(zones_out, "year,month,day,lat_deg,lon_deg,zone\n");
  }

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "cannot open %s\n", argv[1]);
    if (zones_out) fclose(zones_out);
    return 2;
  }

  char line[512];
  if (!fgets(line, sizeof line, f)) { /* header */
    fclose(f);
    return 2;
  }

  long evening_count = 0;
  long nan_count = 0;
  long zone_mismatch_count = 0;
  double max_residual = -1.0;
  int worst_year = 0, worst_month = 0, worst_day = 0;
  double worst_lat = 0.0, worst_lon = 0.0;
  double worst_q_yallop = 0.0, worst_q_ours = 0.0;

  while (fgets(line, sizeof line, f)) {
    int obs, year, month, day;
    char phase[16];
    double lat_deg, lon_deg, arcv_deg, width_arcmin, q_yallop;
    char observed_code[16];

    int n = sscanf(line, "%d,%d,%d,%d,%15[^,],%lf,%lf,%lf,%lf,%lf,%15[^\n]",
                   &obs, &year, &month, &day, phase, &lat_deg, &lon_deg,
                   &arcv_deg, &width_arcmin, &q_yallop, observed_code);
    if (n != 11) continue;
    if (strcmp(phase, "evening") != 0) continue;

    evening_count++;

    /* TN69 gives no elevation for these historical sites; 0.0 m is the
     * documented substitute. */
    HijriLocation loc = {lat_deg, lon_deg, 0.0, NULL};
    HijriYallopResult result =
        hijri_yallop_evaluate_evening(year, month, day, &loc);

    if (zones_out) {
      if (isnan(result.q)) {
        fprintf(zones_out, "%d,%d,%d,%.4f,%.4f,NA\n", year, month, day,
                lat_deg, lon_deg);
      } else {
        fprintf(zones_out, "%d,%d,%d,%.4f,%.4f,%c\n", year, month, day,
                lat_deg, lon_deg, zone_letter(result.zone));
      }
    }

    if (isnan(result.q)) {
      nan_count++;
      continue;
    }

    double residual = fabs(result.q - q_yallop);
    if (residual > max_residual) {
      max_residual = residual;
      worst_year = year;
      worst_month = month;
      worst_day = day;
      worst_lat = lat_deg;
      worst_lon = lon_deg;
      worst_q_yallop = q_yallop;
      worst_q_ours = result.q;
    }

    if (result.zone != zone_from_q(q_yallop)) zone_mismatch_count++;
  }

  fclose(f);
  if (zones_out) fclose(zones_out);

  printf("evening_rows=%ld\n", evening_count);
  printf("nan_count=%ld\n", nan_count);
  printf("zone_mismatch_count=%ld\n", zone_mismatch_count);
  printf("max_residual=%.6f\n", max_residual);
  printf("worst_row=%04d-%02d-%02d lat=%.4f lon=%.4f q_yallop=%.6f q_ours=%.6f\n",
         worst_year, worst_month, worst_day, worst_lat, worst_lon,
         worst_q_yallop, worst_q_ours);

  if (!(max_residual <= 0.10)) {
    fprintf(stderr, "HALT: max_residual exceeds 0.10 (or is NAN)\n");
    return 1;
  }

  return 0;
}
