#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "pool.h"
#include "opresolv.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum resolution_entry_state
{
	RESOLUTION_ENTRY_STATE_UNSPECIFIED,
	RESOLUTION_ENTRY_STATE_SPECIFIED,
};

struct option_resolution_entry
{
	const struct OptionGuide *guide_entry;
	enum resolution_entry_state state;
	union
	{
		int int_value;
		const char *str_value;
	} u;
};

struct _option_resolution
{
	memory_pool pool;
	const char *specification;
	size_t option_count;
	struct option_resolution_entry *entries;
};



static optreserr_t resolve_single_param(const char *specification, int *param_value,
	struct OptionRange *range, size_t range_count)
{
	int FLAG_IN_RANGE			= 0x01;
	int FLAG_IN_DEFAULT			= 0x02;
	int FLAG_DEFAULT_SPECIFIED	= 0x04;
	int FLAG_HALF_RANGE			= 0x08;

	int last_value = 0;
	int value = 0;
	int flags = 0;
	const char *s = specification;

	while(*s && !isalpha(*s))
	{
		if (*s == '-')
		{
			/* range specifier */
			if (flags & (FLAG_IN_RANGE|FLAG_IN_DEFAULT))
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			flags |= FLAG_IN_RANGE;
			s++;

			if (range)
			{
				range->max = -1;
				if ((flags & FLAG_HALF_RANGE) == 0)
				{
					range->min = -1;
					flags |= FLAG_HALF_RANGE;
				}
			}
		}
		else if (*s == '[')
		{
			/* begin default value */
			if (flags & (FLAG_IN_DEFAULT|FLAG_DEFAULT_SPECIFIED))
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			flags |= FLAG_IN_DEFAULT;
			s++;
		}
		else if (*s == ']')
		{
			/* end default value */
			if ((flags & FLAG_IN_DEFAULT) == 0)
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			flags &= ~FLAG_IN_DEFAULT;
			flags |= FLAG_DEFAULT_SPECIFIED;
			s++;

			if (param_value && *param_value == -1)
				*param_value = value;
		}
		else if (*s == '/')
		{
			/* value separator */
			if (flags & (FLAG_IN_DEFAULT|FLAG_IN_RANGE))
				return OPTIONRESOLUTION_ERROR_SYNTAX;
			s++;

			/* if we are spitting out ranges, complete the range */
			if (range && (flags & FLAG_HALF_RANGE))
			{
				range++;
				flags &= ~FLAG_HALF_RANGE;
				if (--range_count == 0)
					range = NULL;
			}
		}
		else if (*s == ';')
		{
			/* basic separator */
			s++;
		}
		else if (isdigit(*s))
		{
			/* numeric value */
			last_value = value;
			value = 0;
			do
			{
				value *= 10;
				value += *s - '0';
				s++;
			}
			while(isdigit(*s));

			if (range)
			{
				if ((flags & FLAG_HALF_RANGE) == 0)
				{
					range->min = value;
					flags |= FLAG_HALF_RANGE;
				}
				range->max = value;
			}

			/* if we have a value; check to see if it is out of range */
			if (param_value && (*param_value != -1) && (*param_value != value))
			{
				if ((last_value < *param_value) && (*param_value < value))
				{
					if ((flags & FLAG_IN_RANGE) == 0)
						return OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE;
				}
			}
			flags &= ~FLAG_IN_RANGE;
		}
		else
			return OPTIONRESOLUTION_ERROR_SYNTAX;
	}

	/* we can't have zero length guidelines strings */
	if (s == specification)
		return OPTIONRESOLUTION_ERROR_SYNTAX;

	return OPTIONRESOLUTION_ERROR_SUCCESS;
}



static const char *lookup_in_specification(const char *specification, const struct OptionGuide *option)
{
	const char *s;
	s = strchr(specification, option->parameter);
	return s ? s + 1 : NULL;
}


static optreserr_t read_string(const char *s, char *buffer, size_t buffer_size)
{
	if (*s++ != '\'')
		return OPTIONRESOLUTION_ERROR_SYNTAX;
	
	/* save space for trailing NUL */
	buffer_size--;

	while((s[0] != '\'') || (s[1] == '\''))
	{
		if (!buffer_size)
			return OPTIONRESOLUTION_ERROR_OUTOFMEMORY;
		buffer_size--;

		*buffer++ = *s;
		s += (*s == '\'') ? 2 : 1;
	}

	*buffer = '\0';
	return OPTIONRESOLUTION_ERROR_SUCCESS;
}



option_resolution *option_resolution_create(const struct OptionGuide *guide, const char *specification)
{
	option_resolution *resolution = NULL;
	const struct OptionGuide *guide_entry;
	int option_count;
	int opt = -1;

	assert(guide);

	/* first count the number of options specified in the guide */
	option_count = 0;
	guide_entry = guide;
	while(guide_entry->option_type != OPTIONTYPE_END)
	{
		switch(guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_STRING:
		case OPTIONTYPE_ENUM_BEGIN:
			if (lookup_in_specification(specification, guide_entry))
				option_count++;
			break;
		case OPTIONTYPE_ENUM_VALUE:
			break;
		default:
			goto unexpected;
		}
		guide_entry++;
	}

	/* allocate the main structure */
	resolution = malloc(sizeof(option_resolution));
	if (!resolution)
		goto outofmemory;
	memset(resolution, 0, sizeof(*resolution));
	pool_init(&resolution->pool);

	/* set up the entries list */
	resolution->option_count = option_count;
	resolution->specification = specification;
	resolution->entries = pool_malloc(&resolution->pool, sizeof(struct option_resolution_entry) * option_count);
	if (!resolution->entries)
		goto outofmemory;
	memset(resolution->entries, 0, sizeof(struct option_resolution_entry) * option_count);

	/* initialize each of the entries */
	opt = 0;
	guide_entry = guide;
	while(guide_entry->option_type != OPTIONTYPE_END)
	{
		switch(guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_ENUM_BEGIN:
		case OPTIONTYPE_STRING:
			if (lookup_in_specification(specification, guide_entry))
				resolution->entries[opt++].guide_entry = guide_entry;
			break;
		case OPTIONTYPE_ENUM_VALUE:
			break;
		default:
			goto unexpected;
		}
		guide_entry++;
	}
	assert(opt == option_count);
	return resolution;

unexpected:
	assert(FALSE);
outofmemory:
	if (resolution)
		option_resolution_close(resolution);
	return NULL;
}



optreserr_t option_resolution_add_param(option_resolution *resolution, const char *param, const char *value)
{
	int i;
	int must_resolve;
	optreserr_t err;
	const char *option_specification;
	struct option_resolution_entry *entry = NULL;

	for (i = 0; i < resolution->option_count; i++)
	{
		if (!strcmp(param, resolution->entries[i].guide_entry->identifier))
		{
			entry = &resolution->entries[i];
			break;
		}
	}
	if (!entry)
		return OPTIONRESOLUTION_ERROR_PARAMNOTFOUND;

	if (entry->state != RESOLUTION_ENTRY_STATE_UNSPECIFIED)
		return OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED;

	switch(entry->guide_entry->option_type) {
	case OPTIONTYPE_INT:
		entry->u.int_value = atoi(value);
		entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		must_resolve = TRUE;
		break;

	case OPTIONTYPE_STRING:
		entry->u.str_value = pool_strdup(&resolution->pool, value);
		if (!entry->u.str_value)
		{
			err = OPTIONRESOLUTION_ERROR_OUTOFMEMORY;
			goto done;
		}
		entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		must_resolve = FALSE;
		break;

	case OPTIONTYPE_ENUM_BEGIN:
		for (i = 1; entry->guide_entry[i].option_type == OPTIONTYPE_ENUM_VALUE; i++)
		{
			if (!stricmp(value, entry->guide_entry[i].identifier))
			{
				entry->u.int_value = entry->guide_entry[i].parameter;
				entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
				break;
			}
		}
		if (entry->state != RESOLUTION_ENTRY_STATE_SPECIFIED)
		{
			err = OPTIONRESOLUTION_ERROR_BADPARAM;
			goto done;
		}
		must_resolve = TRUE;
		break;

	default:
		err = OPTIONRESOLTUION_ERROR_INTERNAL;
		assert(0);
		goto done;
	}

	/* do a resolution step if necessary */
	if (must_resolve)
	{
		option_specification = lookup_in_specification(resolution->specification, entry->guide_entry);
		err = resolve_single_param(option_specification, &entry->u.int_value, NULL, 0);
		if (err)
			goto done;

		/* did we not get a real value? */
		if (entry->u.int_value < 0)
		{
			err = OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED;
			goto done;
		}
	}

	err = OPTIONRESOLUTION_ERROR_SUCCESS;

done:
	if (err)
	{
		option_resolution_close(resolution);
		resolution = NULL;
	}
	return err;
}



void option_resolution_close(option_resolution *resolution)
{
	pool_exit(&resolution->pool);
	free(resolution);
}



optreserr_t option_resolution_finish(option_resolution *resolution)
{
	int i;
	optreserr_t err;
	struct option_resolution_entry *entry;
	const char *option_specification;

	for (i = 0; i < resolution->option_count; i++)
	{
		entry = &resolution->entries[i];

		if (entry->state == RESOLUTION_ENTRY_STATE_UNSPECIFIED)
		{
			switch(entry->guide_entry->option_type) {
			case OPTIONTYPE_INT:
			case OPTIONTYPE_ENUM_BEGIN:
				option_specification = lookup_in_specification(resolution->specification, entry->guide_entry);
				assert(option_specification);
				entry->u.int_value = -1;
				err = resolve_single_param(option_specification, &entry->u.int_value, NULL, 0);
				if (err)
					return err;
				break;

			case OPTIONTYPE_STRING:
				entry->u.str_value = "";
				break;

			default:
				assert(FALSE);
				return OPTIONRESOLTUION_ERROR_INTERNAL;
			}
			entry->state = RESOLUTION_ENTRY_STATE_SPECIFIED;
		}
	}
	return OPTIONRESOLUTION_ERROR_SUCCESS;
}



static const struct option_resolution_entry *option_resolution_lookup_entry(option_resolution *resolution, int option_char)
{
	size_t i;
	const struct option_resolution_entry *entry;

	for (i = 0; i < resolution->option_count; i++)
	{
		entry = &resolution->entries[i];

		switch(entry->guide_entry->option_type) {
		case OPTIONTYPE_INT:
		case OPTIONTYPE_STRING:
		case OPTIONTYPE_ENUM_BEGIN:
			if (entry->guide_entry->parameter == option_char)
				return entry;
			break;

		default:
			assert(FALSE);
			return NULL;
		}
	}
	return NULL;
}



int option_resolution_lookup_int(option_resolution *resolution, int option_char)
{
	const struct option_resolution_entry *entry;
	entry = option_resolution_lookup_entry(resolution, option_char);
	return entry ? entry->u.int_value : -1;
}



const char *option_resolution_lookup_string(option_resolution *resolution, int option_char)
{
	const struct option_resolution_entry *entry;
	entry = option_resolution_lookup_entry(resolution, option_char);
	return entry ? entry->u.str_value : NULL;
}



optreserr_t option_resolution_listranges(const char *specification, int option_char,
	struct OptionRange *range, size_t range_count)
{
	assert(range_count > 0);

	/* clear out range */
	memset(range, -1, sizeof(*range) * range_count);
	range_count--;

	specification = strchr(specification, option_char);
	if (!specification)
		return OPTIONRESOLUTION_ERROR_SYNTAX;

	return resolve_single_param(specification + 1, NULL, range, range_count);
}



optreserr_t option_resolution_getdefault(const char *specification, int option_char, int *val)
{
	assert(val);

	/* clear out default */
	*val = -1;

	specification = strchr(specification, option_char);
	if (!specification)
		return OPTIONRESOLUTION_ERROR_SYNTAX;

	return resolve_single_param(specification + 1, val, NULL, 0);
}



const char *option_resolution_error_string(optreserr_t err)
{
	static const char *errors[] =
	{
		"The operation completed successfully",		/* OPTIONRESOLUTION_ERROR_SUCCESS */
		"Out of memory",							/* OPTIONRESOLUTION_ERROR_OUTOFMEMORY */
		"Parameter out of range",					/* OPTIONRESOLUTION_ERROR_PARAMOUTOFRANGE */
		"Parameter not specified",					/* OPTIONRESOLUTION_ERROR_PARAMNOTSPECIFIED */
        "Unknown parameter",						/* OPTIONRESOLUTION_ERROR_PARAMNOTFOUND */
		"Paramter specified multiple times",		/* OPTIONRESOLUTION_ERROR_PARAMALREADYSPECIFIED */
		"Invalid parameter",						/* OPTIONRESOLUTION_ERROR_BADPARAM */
		"Syntax error",								/* OPTIONRESOLUTION_ERROR_SYNTAX */
		"Internal error"							/* OPTIONRESOLTUION_ERROR_INTERNAL */
	};

	if ((err < 0) || (err >= sizeof(errors) / sizeof(errors[0])))
		return NULL;
	return errors[err];
}

