/* Compares hijri_odeh_evaluate_evening's own ARCV and crescent width
 * against Odeh's printed Table VI values, for the Table VI transcription's
 * evening rows (Task 3's table_vi_transcription.csv). Not a test binary:
 * it is built to build/compare_arcv, outside the tests tree, so it never
 * joins make check. Its input and output live in the scratchpad, this
 * file carries no data.
 *
 * Table VI's W column is printed in ARC SECONDS. crescent_width_arcmin is
 * in arc minutes, so W is divided by 60 before it is compared.
 *
 * The CSV columns, in order, are:
 * No.,R,E,Date,Observer,Long,Lat,Ele,N,B,T,JD,Age,Lag,ARCV,DAZ,ARCL,W,V,page
 * Date is dd-mm-yyyy, local. Long is signed degrees east, Lat signed
 * degrees north, matching HijriLocation's own convention.
 *
 * `--emit-zones OUTPUT.csv` writes one `No.,zone` line per input row, in
 * the scratch CSV's own row order, so `extract.py`'s fixture emit step can
 * zip it positionally against the same scratch CSV. `zone` is this
 * library's own `hijri_odeh_classify` result, recomputed from the row's
 * own date, latitude, longitude and elevation through
 * `hijri_odeh_evaluate_evening`, using the same name strings as
 * `tests/hijri_research_probe.c`'s `odeh_zone_name`. Morning (`E` column
 * `M`) rows are not evaluated (the function models evenings only) and get
 * an empty zone; an evening row whose sunset or moonset event fails also
 * gets `UNAVAILABLE`, matching the research probe's convention for a NaN
 * result. This mode never prints Odeh's own ARCV/DAZ/ARCL/W/V: those never
 * leave this scratch tool. */

#define HIJRI_IMPLEMENTATION
#include "hijri.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 600
#define MAX_COLS 20

/* Splits a CSV line in place on commas, tolerating empty fields (the N, B
 * and T columns are frequently empty). Returns the field count. Table VI
 * has no quoted or comma-containing fields, so a plain split is safe. */
static int split_csv(char *line, char *fields[], int max_fields) {
  int count = 0;
  char *start = line;
  for (char *p = line; ; p++) {
    if (*p == ',' || *p == '\0' || *p == '\n' || *p == '\r') {
      char terminator = *p;
      *p = '\0';
      if (count < max_fields) fields[count++] = start;
      if (terminator == '\0' || terminator == '\n' || terminator == '\r')
        break;
      start = p + 1;
    }
  }
  return count;
}

static int compare_double(const void *a, const void *b) {
  double da = *(const double *)a;
  double db = *(const double *)b;
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

static double median_of(double *values, long count) {
  if (count == 0) return NAN;
  qsort(values, (size_t)count, sizeof(double), compare_double);
  if (count % 2 == 1) return values[count / 2];
  return 0.5 * (values[count / 2 - 1] + values[count / 2]);
}

/* Same strings as tests/hijri_research_probe.c's odeh_zone_name. */
static const char *odeh_zone_name(HijriOdehZone zone) {
  static const char *names[] = {"NOT_VISIBLE", "OPTICAL_AID_ONLY",
                                 "OPTICAL_AID_OR_NAKED_EYE", "NAKED_EYE"};
  return names[(int)zone];
}

static int emit_zones(const char *input_path, const char *output_path) {
  FILE *f = fopen(input_path, "r");
  if (!f) {
    fprintf(stderr, "cannot open %s\n", input_path);
    return 2;
  }
  FILE *out = fopen(output_path, "w");
  if (!out) {
    fprintf(stderr, "cannot open %s for writing\n", output_path);
    fclose(f);
    return 2;
  }

  char line[512];
  if (!fgets(line, sizeof line, f)) { /* header */
    fclose(f);
    fclose(out);
    return 2;
  }
  fprintf(out, "No.,zone\n");

  long row_count = 0;
  while (fgets(line, sizeof line, f)) {
    char *fields[MAX_COLS];
    int nfields = split_csv(line, fields, MAX_COLS);
    if (nfields != 20) {
      fprintf(stderr, "row %ld: expected 20 fields, got %d\n", row_count + 1,
              nfields);
      fclose(f);
      fclose(out);
      return 2;
    }
    row_count++;

    if (strcmp(fields[2], "M") == 0) {
      fprintf(out, "%s,\n", fields[0]);
      continue;
    }

    int day, month, year;
    if (sscanf(fields[3], "%d-%d-%d", &day, &month, &year) != 3) {
      fprintf(stderr, "row %ld: unparsable Date %s\n", row_count, fields[3]);
      fclose(f);
      fclose(out);
      return 2;
    }

    double lon_deg = atof(fields[5]);
    double lat_deg = atof(fields[6]);
    double ele = atof(fields[7]);

    HijriLocation loc = {lat_deg, lon_deg, ele, NULL};
    HijriOdehResult result =
        hijri_odeh_evaluate_evening(year, month, day, &loc);

    if (isnan(result.v)) {
      fprintf(out, "%s,UNAVAILABLE\n", fields[0]);
    } else {
      fprintf(out, "%s,%s\n", fields[0], odeh_zone_name(result.zone));
    }
  }

  fclose(f);
  fclose(out);
  fprintf(stderr, "wrote %ld zone rows to %s\n", row_count, output_path);
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr,
            "usage: %s <table_vi-evening.csv>\n"
            "       %s <table_vi_transcription.csv> --emit-zones <output.csv>\n",
            argv[0], argv[0]);
    return 2;
  }

  if (argc == 4 && strcmp(argv[2], "--emit-zones") == 0)
    return emit_zones(argv[1], argv[3]);

  FILE *f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "cannot open %s\n", argv[1]);
    return 2;
  }

  char line[512];
  if (!fgets(line, sizeof line, f)) { /* header */
    fclose(f);
    return 2;
  }

  static double arcv_residual[MAX_ROWS];
  static double width_residual_arcmin[MAX_ROWS];
  long evening_count = 0;
  long nan_count = 0;
  double max_arcv_residual = -1.0;
  double max_width_residual = -1.0;
  long arcv_beyond_1deg = 0;
  long width_beyond_1arcmin = 0;

  while (fgets(line, sizeof line, f)) {
    char *fields[MAX_COLS];
    int nfields = split_csv(line, fields, MAX_COLS);
    /* No.,R,E,Date,Observer,Long,Lat,Ele,N,B,T,JD,Age,Lag,ARCV,DAZ,ARCL,W,V,page */
    if (nfields != 20) continue;
    if (strcmp(fields[2], "E") != 0) continue;

    int day, month, year;
    if (sscanf(fields[3], "%d-%d-%d", &day, &month, &year) != 3) continue;

    double lon_deg = atof(fields[5]);
    double lat_deg = atof(fields[6]);
    double ele = atof(fields[7]);
    double arcv_table = atof(fields[14]);
    double w_arcsec = atof(fields[17]);

    if (evening_count >= MAX_ROWS) {
      fprintf(stderr, "more evening rows than MAX_ROWS, increase it\n");
      break;
    }

    HijriLocation loc = {lat_deg, lon_deg, ele, NULL};
    HijriOdehResult result =
        hijri_odeh_evaluate_evening(year, month, day, &loc);

    if (isnan(result.arcv_deg) || isnan(result.crescent_width_arcmin)) {
      nan_count++;
      continue;
    }

    double w_table_arcmin = w_arcsec / 60.0;

    double dv = result.arcv_deg - arcv_table;
    double dw = result.crescent_width_arcmin - w_table_arcmin;

    arcv_residual[evening_count] = dv;
    width_residual_arcmin[evening_count] = dw;
    evening_count++;

    if (fabs(dv) > max_arcv_residual) max_arcv_residual = fabs(dv);
    if (fabs(dw) > max_width_residual) max_width_residual = fabs(dw);
    if (fabs(dv) > 1.0) arcv_beyond_1deg++;
    if (fabs(dw) > 1.0) width_beyond_1arcmin++;
  }

  fclose(f);

  if (evening_count == 0) {
    fprintf(stderr, "no evening rows compared\n");
    return 1;
  }

  double arcv_sum = 0.0, arcv_abs_sum = 0.0;
  double width_sum = 0.0, width_abs_sum = 0.0;
  static double arcv_abs[MAX_ROWS];
  static double width_abs[MAX_ROWS];
  for (long i = 0; i < evening_count; i++) {
    arcv_sum += arcv_residual[i];
    arcv_abs_sum += fabs(arcv_residual[i]);
    arcv_abs[i] = fabs(arcv_residual[i]);

    width_sum += width_residual_arcmin[i];
    width_abs_sum += fabs(width_residual_arcmin[i]);
    width_abs[i] = fabs(width_residual_arcmin[i]);
  }

  double arcv_mean = arcv_sum / (double)evening_count;
  double arcv_mean_abs = arcv_abs_sum / (double)evening_count;
  double arcv_median_abs = median_of(arcv_abs, evening_count);

  double width_mean = width_sum / (double)evening_count;
  double width_mean_abs = width_abs_sum / (double)evening_count;
  double width_median_abs = median_of(width_abs, evening_count);

  printf("evening_rows_compared=%ld\n", evening_count);
  printf("nan_count=%ld\n", nan_count);
  printf("arcv_mean_signed_deg=%.6f\n", arcv_mean);
  printf("arcv_mean_abs_deg=%.6f\n", arcv_mean_abs);
  printf("arcv_median_abs_deg=%.6f\n", arcv_median_abs);
  printf("arcv_max_abs_deg=%.6f\n", max_arcv_residual);
  printf("arcv_beyond_1deg_count=%ld\n", arcv_beyond_1deg);
  printf("width_mean_signed_arcmin=%.6f\n", width_mean);
  printf("width_mean_abs_arcmin=%.6f\n", width_mean_abs);
  printf("width_median_abs_arcmin=%.6f\n", width_median_abs);
  printf("width_max_abs_arcmin=%.6f\n", max_width_residual);
  printf("width_beyond_1arcmin_count=%ld\n", width_beyond_1arcmin);

  return 0;
}
