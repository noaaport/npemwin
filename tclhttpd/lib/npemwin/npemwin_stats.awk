#
# $Id$
#
# Daily satistics summary

BEGIN {

  frames_received = 0;
  frames_processed = 0;
  frames_errors = 0;
  bytes_received = 0;

  fmt = "<tr><td>%s</td><td>%.3e</td><td>%.3e</td></tr>\n";
  fmt1 = "<tr><td>%s</td><td>%f</td><td></td></tr>\n";
}

END {
    
  frames_errors_fraction = frames_errors/frames_received;

  print "<h3>Daily Statistics frames and bytes</h3>";
  print "<table border>";
  printf("<tr><td></td><td>%s</td><td>%s</td></tr>\n", "total", "per minute");
  printf(fmt, "frames received", frames_received, frames_received/NR);
  printf(fmt, "frames processed", frames_processed, frames_processed/NR);
  printf(fmt, "frames errors", frames_errors, frames_errors/NR);
  printf(fmt, "bytes received", bytes_received, bytes_received/NR);
  printf(fmt1, "fractional error", frames_errors_fraction);
  print "</table>";
}

{
  frames_received += $2;
  frames_processed += $3;;
  frames_errors += $4;
  bytes_received += $5;
}
