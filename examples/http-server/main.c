// Copyright (c) 2020 Cesanta Software Limited
// All rights reserved

//Run following command from this directory to compile  
//cc ../../mongoose.c main.c -I../.. -I../.. -W -Wall -DMG_ENABLE_IPV6=1 -DMG_ENABLE_LINES=1  -o http_server -lws2_32 -mwindows

#include <signal.h>
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include "mongoose.h"

static const char *s_debug_level = "2";
static const char *s_root_dir = ".";
static const char *s_listening_address = "http://localhost:8000";
static const char *s_enable_hexdump = "no";
static const char *s_ssi_pattern = "#.shtml";

// Handle interrupts, like Ctrl-C
static int s_signo;
static void signal_handler(int signo) {
  s_signo = signo;
}

// Event handler for the listening connection.
// Simply serve static files from `s_root_dir`
static void cb(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_serve_opts opts = {.root_dir = s_root_dir,
                                      .ssi_pattern = s_ssi_pattern};
    mg_http_serve_dir(c, ev_data, &opts);
  }
  (void) fn_data;
}

static void usage(const char *prog) {
  fprintf(stderr,
          "Mongoose v.%s\n"
          "Usage: %s OPTIONS\n"
          "  -H yes|no - enable traffic hexdump, default: '%s'\n"
          "  -S GLOB   - glob pattern for SSI files, default: '%s'\n"
          "  -d DIR    - directory to serve, default: '%s'\n"
          "  -l ADDR   - listening address, default: '%s'\n"
          "  -v LEVEL  - debug level, from 0 to 4, default: '%s'\n",
          MG_VERSION, prog, s_debug_level, s_enable_hexdump, s_ssi_pattern,
          s_root_dir, s_listening_address);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  struct mg_mgr mgr;
  struct mg_connection *c;
  int i;

  // Parse command-line flags
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      s_root_dir = argv[++i];
    } else if (strcmp(argv[i], "-H") == 0) {
      s_enable_hexdump = argv[++i];
    } else if (strcmp(argv[i], "-S") == 0) {
      s_ssi_pattern = argv[++i];
    } else if (strcmp(argv[i], "-l") == 0) {
      s_listening_address = argv[++i];
    } else if (strcmp(argv[i], "-v") == 0) {
      s_debug_level = argv[++i];
    } else {
      usage(argv[0]);
    }
  }

  // Initialise stuff
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
  mg_log_set(s_debug_level);
  mg_mgr_init(&mgr);
  if ((c = mg_http_listen(&mgr, s_listening_address, cb, &mgr)) == NULL) {
      ShellExecute(0, 0, s_listening_address, 0, 0 , SW_SHOW );
  } else {
      if (mg_casecmp(s_enable_hexdump, "yes") == 0) c->is_hexdumping = 1;
      // Start infinite event loop
      ShellExecute(0, 0, s_listening_address, 0, 0 , SW_SHOW );
      while (s_signo == 0) mg_mgr_poll(&mgr, 1000);
      mg_mgr_free(&mgr);
  }
  return 0;
}
