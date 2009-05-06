/***************************************************************************
 *   Copyright (C) 2004 by Ralf Kaestner                                   *
 *   ralf.kaestner@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <timer.h>

#include <epos.h>
#include <position_profile.h>

int quit = 0;

void epos_signaled(int signal) {
  quit = 1;
}

int main(int argc, char **argv) {
  epos_node_t node;
  epos_position_profile_t profile;
  signal(SIGINT, epos_signaled);

  if (argc < 5) {
    fprintf(stderr, "usage: %s POS VEL ACC DEC\n", argv[0]);
    return -1;
  }
  float pos = atof(argv[1])*M_PI/180.0;
  float vel = atof(argv[2])*M_PI/180.0;
  float acc = atof(argv[3])*M_PI/180.0;
  float dec = atof(argv[4])*M_PI/180.0;
  double t;

  if (epos_init_arg(&node, argc, argv))
    return -1;
  epos_position_profile_init(&profile, pos, vel, acc, dec, epos_sinusoidal);
  profile.relative = 1;
  if (!epos_position_profile_start(&node, &profile)) {
    while (!quit && epos_profile_wait(&node, 0.1)) {
      timer_start(&t);
      float pos_a = epos_get_position(&node)*180.0/M_PI;
      timer_correct(&t);
      float pos_e = epos_position_profile_estimate(&profile, t)*180.0/M_PI;
      fprintf(stdout, "%7.2f  %8.2f  %8.2f\n", t-profile.start_time,
        pos_a, pos_e);
    }
    epos_position_profile_stop(&node);
  }
  epos_close(&node);

  return 0;
}