/* Update check: once at startup, in the background, asks GitHub which
 * release is the latest. github.com/<repo>/releases/latest answers with a
 * redirect to .../releases/tag/<tag>, so only that redirect is read (no
 * JSON, no download). Skipped quietly without Wi-Fi.
 *
 * With libcurl (devkitPro's 3ds-curl, found by gpsp3d.mk) it goes through
 * curl + mbedTLS, like Universal-Updater: the 3DS's own HTTPS (httpc) can't
 * talk to github.com any more (its TLS is too old for GitHub's certificate).
 * Without it, httpc is tried anyway. */
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "update.h"
#ifdef HAVE_CURL
#include <curl/curl.h>
#ifdef CP_ENABLED
#include "update_roots.h"
#endif
#endif

#ifdef CP_ENABLED
#define RELEASE_ROOT "https://github.com/Slickmaxim/unbound-dual/releases/"
#define USER_AGENT "Unbound-Dual-update-check"
#else
#define RELEASE_ROOT "https://github.com/bheppe/gpsp-3d/releases/"
#define USER_AGENT "gpSP-3D-update-check"
#endif
#define LATEST_URL RELEASE_ROOT "latest"

static Thread         upd_thr;
static volatile bool  upd_running, upd_cancel;
static volatile int   upd_state;          /* UPD_* */
static char           upd_latest[24];
static char           upd_error[64];
static const char    *upd_current;

/* "0.1.10" > "0.1.9": compare number by number */
static int ver_cmp(const char *a, const char *b)
{
   while (*a || *b)
   {
      long x = strtol(a, (char**)&a, 10), y = strtol(b, (char**)&b, 10);
      if (x != y)
         return x < y ? -1 : 1;
      if (*a == '.') a++;
      if (*b == '.') b++;
      if ((*a && (*a < '0' || *a > '9')) || (*b && (*b < '0' || *b > '9')))
         break;   /* "-test" and the like: stop at the numbers */
   }
   return 0;
}

static bool wifi_up(void)
{
   u32 st = 0;
   if (R_FAILED(acInit()))
      return false;
   if (R_FAILED(ACU_GetWifiStatus(&st)))
      st = 0;
   acExit();
   return st != 0;
}

/* .../releases/tag/v0.1.7 -> "0.1.7" */
static bool take_tag(const char *loc)
{
   static const char prefix[] = RELEASE_ROOT "tag/";
   const char *t = loc && !strncmp(loc, prefix, sizeof(prefix)-1) ? loc+sizeof(prefix)-1 : NULL;
   size_t n;
   if (!t)
      return false;
   if (*t == 'v' || *t == 'V')
      t++;
   n = strcspn(t, "/?#\r\n");
   if (n == 0 || n >= sizeof(upd_latest))
      return false;
   for (size_t i = 0; i < n; ++i)
      if ((t[i] < '0' || t[i] > '9') && t[i] != '.') return false;
   if (t[0] == '.' || t[n-1] == '.') return false;
   memcpy(upd_latest, t, n);
   upd_latest[n] = 0;
   upd_state = ver_cmp(upd_latest, upd_current) > 0 ? UPD_NEWER : UPD_CURRENT;
   return true;
}

#ifdef HAVE_CURL
#define SOC_SIZE 0x40000
static int xfer_cb(void *p, curl_off_t a, curl_off_t b, curl_off_t c, curl_off_t d)
{
   (void)p; (void)a; (void)b; (void)c; (void)d;
   return upd_cancel;   /* non-zero aborts */
}

static size_t sink_cb(char *p, size_t size, size_t n, void *u)
{
   (void)p; (void)u;
   return size * n;   /* the page itself isn't needed */
}

static void check_curl(void)
{
   u32 *soc = (u32*)memalign(0x1000, SOC_SIZE);
   CURL *c;
   Result r;
   if (!soc)
   {
      snprintf(upd_error, sizeof(upd_error), "no memory for the network");
      return;
   }
   if (R_FAILED(r = socInit(soc, SOC_SIZE)))
   {
      snprintf(upd_error, sizeof(upd_error), "network start failed (%08lX)", (unsigned long)r);
      free(soc);
      return;
   }
   curl_global_init(CURL_GLOBAL_DEFAULT);
   c = curl_easy_init();
   if (c)
   {
      CURLcode e;
      long code = 0;
      char *loc = NULL;
      curl_easy_setopt(c, CURLOPT_URL, LATEST_URL);
      curl_easy_setopt(c, CURLOPT_USERAGENT, USER_AGENT);
      curl_easy_setopt(c, CURLOPT_NOBODY, 1L);            /* HEAD: only the redirect */
      curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 0L);
#ifdef CP_ENABLED
      struct curl_blob roots = {(void*)release_roots, sizeof(release_roots)-1, CURL_BLOB_NOCOPY};
      if (curl_easy_setopt(c, CURLOPT_CAINFO_BLOB, &roots) != CURLE_OK) {
         snprintf(upd_error, sizeof(upd_error), "HTTPS trust configuration unavailable");
         curl_easy_cleanup(c);
         goto cleanup;
      }
#else
      curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
      curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 8L);
      curl_easy_setopt(c, CURLOPT_TIMEOUT, 15L);
      curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
      curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, sink_cb);
      curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
      curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, xfer_cb);
      e = curl_easy_perform(c);
      if (e == CURLE_OK)
      {
         curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &code);
         curl_easy_getinfo(c, CURLINFO_REDIRECT_URL, &loc);
         if (!take_tag(loc))
            snprintf(upd_error, sizeof(upd_error), "unexpected answer (HTTP %ld)", code);
      }
      else
         snprintf(upd_error, sizeof(upd_error), "%.50s", curl_easy_strerror(e));
      curl_easy_cleanup(c);
   }
#ifdef CP_ENABLED
cleanup:
#endif
   curl_global_cleanup();
   socExit();
   free(soc);
}
#endif

static void check_httpc(void)
{
   static httpcContext ctx;
   char loc[256] = "";
   u32 code = 0;
   Result r;
   if (R_FAILED(r = httpcInit(0)))
   {
      snprintf(upd_error, sizeof(upd_error), "httpc start failed (%08lX)", (unsigned long)r);
      return;
   }
   if (R_SUCCEEDED(r = httpcOpenContext(&ctx, HTTPC_METHOD_GET, LATEST_URL, 0)))
   {
      httpcSetSSLOpt(&ctx, SSLCOPT_DisableVerify);
      httpcSetKeepAlive(&ctx, HTTPC_KEEPALIVE_DISABLED);
      httpcAddRequestHeaderField(&ctx, "User-Agent", USER_AGENT);
      if (R_FAILED(r = httpcBeginRequest(&ctx)))
         snprintf(upd_error, sizeof(upd_error), "httpc request failed (%08lX)", (unsigned long)r);
      else if (R_FAILED(r = httpcGetResponseStatusCodeTimeout(&ctx, &code, 10000000000ULL)))
         snprintf(upd_error, sizeof(upd_error), "httpc no answer (%08lX)", (unsigned long)r);
      else if (code < 300 || code >= 400 ||
               R_FAILED(httpcGetResponseHeader(&ctx, "Location", loc, sizeof(loc) - 1)) ||
               !take_tag(loc))
         snprintf(upd_error, sizeof(upd_error), "unexpected answer (HTTP %lu)", (unsigned long)code);
      httpcCloseContext(&ctx);
   }
   else
      snprintf(upd_error, sizeof(upd_error), "httpc open failed (%08lX)", (unsigned long)r);
   httpcExit();
}

static void upd_main(void *arg)
{
   (void)arg;
   if (!wifi_up())
      upd_state = UPD_OFFLINE;
   else
   {
      upd_state = UPD_CHECKING;
#ifdef HAVE_CURL
      check_curl();
#else
      check_httpc();
#endif
      if (upd_state == UPD_CHECKING)
         upd_state = UPD_FAILED;
   }
   __dmb();
   upd_running = false;
}

void update_check_start(const char *current_version)
{
   if (upd_running)
      return;
   if (upd_thr)
   {
      threadJoin(upd_thr, U64_MAX);
      threadFree(upd_thr);
      upd_thr = NULL;
   }
   upd_current = current_version;
   upd_error[0] = 0;
   upd_cancel = false;
   upd_state = UPD_CHECKING;
   upd_running = true;
   /* lowest priority, on the main core: it only waits on the network */
   upd_thr = threadCreate(upd_main, NULL, 32 * 1024, 0x3F, 0, false);
   if (!upd_thr)
   {
      upd_running = false;
      upd_state = UPD_FAILED;
      snprintf(upd_error, sizeof(upd_error), "couldn't start the check");
   }
}

int update_state(void)
{
   return upd_state;
}

const char *update_latest(void)
{
   return upd_latest;
}

const char *update_error(void)
{
   return upd_error;
}

const char *update_method(void)
{
#ifdef HAVE_CURL
   return "curl";
#else
   return "httpc";
#endif
}

void update_check_stop(void)
{
   if (!upd_thr)
      return;
   upd_cancel = true;
   threadJoin(upd_thr, 2000000000ULL);   /* 2 s at most: never hold up closing */
   if (!upd_running)
      threadFree(upd_thr);
   upd_thr = NULL;
}
#if defined(CP_ENABLED) && defined(HAVE_CURL)
#define UPDATE_LIMIT (16u * 1024u * 1024u)
struct download_file { FILE *file; size_t bytes; };

static size_t write_asset(char *data, size_t size, size_t count, void *ctx)
{
   struct download_file *out = ctx;
   if (size && count > (UPDATE_LIMIT - out->bytes) / size) return 0;
   size_t written = fwrite(data, 1, size * count, out->file);
   out->bytes += written;
   return written;
}

static bool download_asset(const char *asset, const char *path)
{
   char url[192];
   snprintf(url, sizeof(url), RELEASE_ROOT "download/v%s/%s", upd_latest, asset);
   struct download_file out = {fopen(path, "wb"), 0};
   if (!out.file) {
      snprintf(upd_error, sizeof(upd_error), "Can't open update file on SD.");
      return false;
   }
   void *soc = memalign(0x1000, SOC_SIZE);
   bool connected = soc && R_SUCCEEDED(socInit(soc, SOC_SIZE));
   bool ok = false;
   if (connected) {
      if (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK) {
         CURL *c = curl_easy_init();
         if (c) {
            struct curl_blob roots = {(void*)release_roots, sizeof(release_roots)-1, CURL_BLOB_NOCOPY};
            CURLcode e = curl_easy_setopt(c, CURLOPT_CAINFO_BLOB, &roots);
            if (e == CURLE_OK) e = curl_easy_setopt(c, CURLOPT_PROTOCOLS_STR, "https");
            if (e == CURLE_OK) e = curl_easy_setopt(c, CURLOPT_REDIR_PROTOCOLS_STR, "https");
            if (e == CURLE_OK) {
               curl_easy_setopt(c, CURLOPT_URL, url);
               curl_easy_setopt(c, CURLOPT_USERAGENT, USER_AGENT);
               curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
               curl_easy_setopt(c, CURLOPT_MAXREDIRS, 4L);
               curl_easy_setopt(c, CURLOPT_FAILONERROR, 1L);
               curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10L);
               curl_easy_setopt(c, CURLOPT_TIMEOUT, 90L);
               curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_asset);
               curl_easy_setopt(c, CURLOPT_WRITEDATA, &out);
               e = curl_easy_perform(c);
               long status = 0;
               curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status);
               ok = e == CURLE_OK && status == 200 && out.bytes > 0;
            }
            if (!ok) snprintf(upd_error, sizeof(upd_error), "Download: %.50s", curl_easy_strerror(e));
            curl_easy_cleanup(c);
         }
         curl_global_cleanup();
      }
      socExit();
   }
   free(soc);
   if (fclose(out.file)) ok = false;
   if (!ok) {
      remove(path);
      if (!upd_error[0]) snprintf(upd_error, sizeof(upd_error), "Could not download release.");
   }
   return ok;
}

static bool valid_3dsx(const char *path)
{
   uint32_t hdr[8] = {0};
   FILE *f = fopen(path, "rb");
   if (!f) return false;
   bool ok = fread(hdr, sizeof(hdr), 1, f) == 1 && fseek(f, 0, SEEK_END) == 0;
   long size = ok ? ftell(f) : 0;
   fclose(f);
   // Base header, relocation headers, then code/rodata/initialized data.
   uint32_t header = hdr[1] & 0xffff, reloc = hdr[1] >> 16;
   return ok && hdr[0] == 0x58534433 && header >= 32 && reloc >= 8 &&
      hdr[6] >= hdr[7] && hdr[4] > 0 &&
      (uint64_t)header + 3u*reloc + hdr[4] + hdr[5] + hdr[6] - hdr[7] <= (uint64_t)size;
}
#endif

bool update_install(const char *launch_path)
{
#if defined(CP_ENABLED) && defined(HAVE_CURL)
   if (upd_state != UPD_NEWER) return false;
   // A completed check owns SOC until its thread exits.
   if (upd_thr) {
      threadJoin(upd_thr, U64_MAX);
      threadFree(upd_thr);
      upd_thr = NULL;
   }
   upd_error[0] = 0;
   char stage[544], backup[544];
   size_t n = launch_path ? strlen(launch_path) : 0;
   if (n < 12 || n > 512 || strncmp(launch_path, "sdmc:/", 6) ||
       strcasecmp(launch_path + n - 5, ".3dsx") || !valid_3dsx(launch_path)) {
      snprintf(upd_error, sizeof(upd_error), "3DSX launch path unavailable.");
      return false;
   }
   snprintf(stage, sizeof(stage), "%s.update", launch_path);
   snprintf(backup, sizeof(backup), "%s.bak", launch_path);
   if (!download_asset("unbound-dual.3dsx", stage)) return false;
   if (!valid_3dsx(stage)) {
      snprintf(upd_error, sizeof(upd_error), "Invalid 3DSX release.");
   } else {
      remove(backup);
      if (rename(launch_path, backup) == 0) {
         if (rename(stage, launch_path) == 0) return true;
         if (rename(backup, launch_path) != 0) {
            snprintf(upd_error, sizeof(upd_error), "Restore the previous .3dsx.bak.");
            return false;
         }
      }
      snprintf(upd_error, sizeof(upd_error), "Could not replace 3DSX.");
   }
   remove(stage);
   return false;
#else
   (void)launch_path;
   snprintf(upd_error, sizeof(upd_error), "HTTPS installer unavailable.");
   return false;
#endif
}
