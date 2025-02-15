#pragma once

// pre-declare.
class datamap_t;

// prototype.
using inputfunc_t = void( __cdecl* )( void* data );

enum fieldtype_t {
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)
	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)
	FIELD_VECTOR2D,			// 2 floats
	FIELD_TYPECOUNT,		// MUST BE LAST
};

enum {
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT,
};

class typedescription_t {
public:
	fieldtype_t field_type;                         // 0x00
	const char* field_name;                         // 0x04
	int field_offset;                               // 0x08 Local offset value
	uint16_t field_size;                            // 0x10
	int16_t flags;                                  // 0x12
	const char* external_name;                      // 0x14 The name of the variable in the map/fgd data, or the name of the actionT
	void* /*i_save_restore_ops */ save_restore_ops;// 0x18 Pointer to the function set for save/restoring of custom data types
	void* /*inputfunc_t*/ input_func;              // 0x1C For associating function with string names
	datamap_t* td;                                  // 0x20 For embedding additional datatables inside this one
	int32_t field_size_in_bytes;                    // 0x24 Stores the actual member variable size in bytes
	typedescription_t* override_field;              // 0x28 FTYPEDESC_OVERRIDE point to first baseclass instance if chains_validated has occurred
	int32_t override_count;                         // 0x2C Used to track exclusion of baseclass fields
	float field_tolerance;                          // 0x30 Tolerance for field errors for float fields
	int flat_offset[ TD_OFFSET_COUNT ];
	unsigned short flat_group;
};

struct optimized_datamap_t;

class datamap_t {
public:
	typedescription_t* data_desc;
	int data_num_fields;
	char const* data_class_name;
	datamap_t* base_map;
	int packed_size;
	optimized_datamap_t* optimized_datamap;
};