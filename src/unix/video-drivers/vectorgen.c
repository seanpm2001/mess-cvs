#include "xmame.h"
#include "driver.h"
#include "vidhrdw/vector.h"

static float intensity_correction = 1.0;

void vector_set_gamma(float gamma)
{
	sysdep_palette_set_gamma(sysdep_palette, gamma);
}

float vector_get_gamma(void)
{
	return sysdep_palette_get_gamma(sysdep_palette);
}

void vector_set_intensity(float _intensity)
{
	intensity_correction = _intensity;
}

float vector_get_intensity(void)
{
	return intensity_correction;
}

void vector_vh_update_backdrop(struct osd_bitmap *bitmap, struct artwork *a, int full_refresh)
{
   vector_vh_update(bitmap, full_refresh);
}

void vector_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	if (artwork_backdrop)
		vector_vh_update_backdrop(bitmap, artwork_backdrop, full_refresh);
	else
		vector_vh_update(bitmap, full_refresh);
}
