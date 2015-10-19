#include <ofc/sema.h>
#include <string.h>


static ofc_hashmap_t* ofc_sema_type__map = NULL;


static void ofc_sema_type__delete(ofc_sema_type_t* type)
{
	if (!type)
		return;

	switch (type->type)
	{
		case OFC_SEMA_TYPE_ARRAY:
			ofc_sema_array_delete(type->array);
			break;
		default:
			break;
	}

	free(type);
}

uint8_t ofc_sema_type_hash(
	const ofc_sema_type_t* type)
{
	if (!type)
		return 0;

	uint8_t hash = type->type;

	if (type->is_static   ) hash +=  8;
	if (type->is_automatic) hash += 16;
	if (type->is_volatile ) hash += 32;

	switch (type->type)
	{
		case OFC_SEMA_TYPE_STRUCTURE:
			hash += ofc_sema_structure_hash(
				type->structure);
			break;

		case OFC_SEMA_TYPE_POINTER:
			hash += ofc_sema_type_hash(
				type->subtype);
			break;

		case OFC_SEMA_TYPE_ARRAY:
			hash += ofc_sema_type_hash(
				type->subtype);
			hash += ofc_sema_array_hash(
				type->array);
			break;

		default:
			hash += type->kind;
			break;
	}

	return hash;
}

static const ofc_sema_type_t* ofc_sema_type__key(
	const ofc_sema_type_t* type)
{
	return type;
}


static const ofc_sema_type_t* ofc_sema_type__create(
	ofc_sema_type_e type,
	unsigned kind,
	ofc_sema_array_t* array,
	ofc_sema_type_t* subtype,
	const ofc_sema_structure_t* structure,
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	if (!ofc_sema_type__map)
	{
		ofc_sema_type__map = ofc_hashmap_create(
			(void*)ofc_sema_type_hash,
			(void*)ofc_sema_type_compare,
			(void*)ofc_sema_type__key,
			(void*)ofc_sema_type__delete);
		if (!ofc_sema_type__map)
			return NULL;
	}

	ofc_sema_type_t stype =
		{
			.type         = type,
			.is_static    = is_static,
			.is_automatic = is_automatic,
			.is_volatile  = is_volatile,
		};

	switch (type)
	{
		case OFC_SEMA_TYPE_POINTER:
			stype.subtype = subtype;
			break;
		case OFC_SEMA_TYPE_ARRAY:
			stype.subtype = subtype;
			stype.array   = array;
			break;
		case OFC_SEMA_TYPE_STRUCTURE:
			stype.structure = structure;
			break;
		default:
			stype.kind = kind;
			break;
	}

	const ofc_sema_type_t* gtype
		= ofc_hashmap_find(
			ofc_sema_type__map, &stype);
	if (gtype) return gtype;

	ofc_sema_type_t* ntype
		= (ofc_sema_type_t*)malloc(
			sizeof(ofc_sema_type_t));
	if (!ntype) return NULL;
	*ntype = stype;

	if (!ofc_hashmap_add(
		ofc_sema_type__map, ntype))
	{
		free(ntype);
		return NULL;
	}

	return ntype;
}

const ofc_sema_type_t* ofc_sema_type_create_primitive(
	ofc_sema_type_e type,
	unsigned kind,
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	switch (type)
	{
		case OFC_SEMA_TYPE_LOGICAL:
		case OFC_SEMA_TYPE_INTEGER:
		case OFC_SEMA_TYPE_REAL:
		case OFC_SEMA_TYPE_COMPLEX:
			break;
		case OFC_SEMA_TYPE_BYTE:
			if (kind > 1)
				return NULL;
			break;
		default:
			return NULL;
	}

	return ofc_sema_type__create(
		type, kind,
		NULL, NULL, NULL,
		is_static, is_automatic, is_volatile);
}

const ofc_sema_type_t* ofc_sema_type_create_byte(
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	return ofc_sema_type_create_primitive(
		OFC_SEMA_TYPE_BYTE, 0,
		is_static, is_automatic, is_volatile);
}

const ofc_sema_type_t* ofc_sema_type_create_structure(
	const ofc_sema_structure_t* structure,
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	return ofc_sema_type__create(
		OFC_SEMA_TYPE_STRUCTURE, 0,
		NULL, NULL, structure,
		is_static, is_automatic, is_volatile);
}

const ofc_sema_type_t* ofc_sema_type_create_pointer(
	ofc_sema_type_t* target,
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	return ofc_sema_type__create(
		OFC_SEMA_TYPE_POINTER, 0,
		NULL, target, NULL,
		is_static, is_automatic, is_volatile);
}

const ofc_sema_type_t* ofc_sema_type_create_array(
	ofc_sema_type_t* type, ofc_sema_array_t* array,
	bool is_static,
	bool is_automatic,
	bool is_volatile)
{
	return ofc_sema_type__create(
		OFC_SEMA_TYPE_ARRAY, 0,
		array, type, NULL,
		is_static, is_automatic, is_volatile);
}



const ofc_sema_type_t* ofc_sema_type_create(
	const ofc_parse_type_t* ptype)
{
	if (!ptype)
		return NULL;


	unsigned kind = 0;

	unsigned i;
	for (i = 0; i < ptype->params->count; i++)
	{
		/* TODO - Handle unnamed kind */
		if (ofc_str_ref_equal_strz_ci(ptype->params->call_arg[i]->name, "KIND")
			&& (ptype->params->call_arg[i]->type == OFC_PARSE_CALL_ARG_EXPR))
		{
			/* TODO - Expression evaluation to get kind */

			if (kind == 0)
			{
				/* TODO - Print error. */
				return false;
			}
		}
	}

	if (ptype->kind)
	{
		if (ptype->kind == 0)
		{
			/* TODO - Print error. */
			return NULL;
		}

		if (kind > 0)
		{
			if (ptype->kind == kind)
			{
				/* TODO - Print warning. */
			}
			else
			{
				/* TODO - Print error. */
				return NULL;
			}
		}

		kind = ptype->kind;
	}
	else
	{
		/* TODO - If KIND is not set, get default from lang_opts. */
		kind = 4;
	}

	switch (ptype->type)
	{
		case OFC_PARSE_TYPE_DOUBLE_PRECISION:
			kind *= 2;
			break;
		case OFC_PARSE_TYPE_DOUBLE_COMPLEX:
			kind *= 2;
			break;
		default:
			break;
	}

	const ofc_sema_type_t* stype = NULL;
	switch (ptype->type)
	{
		case OFC_PARSE_TYPE_LOGICAL:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_LOGICAL, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_CHARACTER:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_CHARACTER, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_INTEGER:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_INTEGER, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_REAL:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_REAL, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_DOUBLE_PRECISION:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_REAL, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_COMPLEX:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_COMPLEX, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_DOUBLE_COMPLEX:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_COMPLEX, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_BYTE:
			stype = ofc_sema_type__create(
				OFC_SEMA_TYPE_BYTE, kind,
				NULL, NULL, NULL,
				ptype->attr.is_static,
				ptype->attr.is_automatic,
				ptype->attr.is_volatile);
			break;
		case OFC_PARSE_TYPE_TYPE:
			break;

		default:
			return NULL;
	}

	/* TODO - Array from count or (LEN=?). */

	/* TODO - How do we handle type attributes? */

	return stype;
}

bool ofc_sema_type_compare(
	const ofc_sema_type_t* a,
	const ofc_sema_type_t* b)
{
	if (!a || !b)
		return false;

	if (a == b)
		return true;

	if (a->type != b->type)
		return false;

	switch (a->type)
	{
		case OFC_SEMA_TYPE_STRUCTURE:
			return ofc_sema_structure_compare(
				a->structure, b->structure);

		case OFC_SEMA_TYPE_POINTER:
			return ofc_sema_type_compare(
				a->subtype, b->subtype);

		case OFC_SEMA_TYPE_ARRAY:
			return (ofc_sema_type_compare(
				a->subtype, b->subtype)
				&& ofc_sema_array_compare(
					a->array, b->array));

		default:
			break;
	}

	return (a->kind == b->kind);
}


unsigned ofc_sema_type_size(const ofc_sema_type_t* type)
{
	if (!type)
		return 0;

	switch (type->type)
	{
		case OFC_SEMA_TYPE_LOGICAL:
		case OFC_SEMA_TYPE_INTEGER:
		case OFC_SEMA_TYPE_REAL:
			/* TODO - Do this based on lang_opts. */
			if (type->kind == 0)
				return sizeof(int);
			return type->kind;

		case OFC_SEMA_TYPE_COMPLEX:
			/* TODO - Do this based on lang_opts. */
			if (type->kind == 0)
				return (sizeof(int) * 2);
			return (type->kind * 2);

		case OFC_SEMA_TYPE_BYTE:
			return 1;

		case OFC_SEMA_TYPE_STRUCTURE:
			return ofc_sema_structure_size(
				type->structure);

		case OFC_SEMA_TYPE_POINTER:
			/* TODO - Do this based on target arch. */
			return sizeof(void*);

		case OFC_SEMA_TYPE_ARRAY:
			return (ofc_sema_type_size(type->subtype)
				* ofc_sema_array_total(type->array));

		default:
			break;
	}

	return 0;
}

bool ofc_sema_type_is_integer(const ofc_sema_type_t* type)
{
	if (!type)
		return false;

	switch (type->type)
	{
		case OFC_SEMA_TYPE_INTEGER:
		case OFC_SEMA_TYPE_BYTE:
			return true;
		default:
			break;
	}

	return false;
}

bool ofc_sema_type_is_logical(const ofc_sema_type_t* type)
{
	if (!type)
		return false;

	switch (type->type)
	{
		case OFC_SEMA_TYPE_LOGICAL:
		case OFC_SEMA_TYPE_BYTE:
			return true;
		default:
			break;
	}

	return false;
}
