/*
 * X-mame main-routine
 */

#define __MAIN_C_
#include "xmame.h"
#include "effect.h"

#if defined HAVE_MPROTECT || defined __QNXNTO__
#include <sys/mman.h>
#endif

/* From video.c. */
void osd_video_initpre();

/* put here anything you need to do when the program is started. Return 0 if */
/* initialization was successful, nonzero otherwise. */
int osd_init(void)
{
	/* now invoice system-dependent initialization */
#ifdef XMAME_NET
	if (osd_net_init()      !=OSD_OK) return OSD_NOT_OK;
#endif	
	if (osd_input_initpre() !=OSD_OK) return OSD_NOT_OK;

	return OSD_OK;
}

/*
 * Cleanup routines to be executed when the program is terminated.
 */
void osd_exit(void)
{
#ifdef XMAME_NET
	osd_net_close();
#endif
	osd_input_close();
}

void *osd_alloc_executable(size_t size)
{
	void *addr = malloc(size);
#ifdef HAVE_MPROTECT
	mprotect(addr, size, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
	return addr;
}

void osd_free_executable(void *ptr)
{
	free(ptr);
}

int main(int argc, char **argv)
{
	int res, res2;

#ifdef __QNXNTO__
	printf("info: Trying to enable swapfile.... ");
	munlockall();
	printf("Success!\n");
#endif

	/* some display methods need to do some stuff with root rights */
	res2 = sysdep_init();

	/* to be absolutely safe force giving up root rights here in case
	   a display method doesn't */
	if (setuid(getuid()))
	{
		perror("setuid");
		sysdep_close();
		return OSD_NOT_OK;
	}

	/* Set the title, now auto build from defines from the makefile */
	sprintf(title,"%s (%s) version %s", NAME, DISPLAY_METHOD,
			build_version);

	/* parse configuration file and environment */
	if ((res = config_init(argc, argv)) != 1234 || res2 == OSD_NOT_OK)
		goto leave;

	/* the effect code might want to change the scaling options
	   to match the chosen effect */
	effect_init();

	/* 
	 * Initialize whatever is needed before the display is actually 
	 * opened, e.g., artwork setup.
	 */
	osd_video_initpre();

	/* go for it */
	res = run_game (game_index);

leave:
	sysdep_close();
	/* should be done last since this also closes stdout and stderr */
	config_exit();

	return res;
}
