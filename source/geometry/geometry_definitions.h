/*
GEOMETRY_DEFINITIONS.H

A geometry section: the vertices and the triangles of a piece of geometry.
A tag keeps them in a block of the cache file apart from the tag, and has a
section info and a block info for each.

The block is

	'blkh', the size of the resources
	the section (geometry_section), with no addresses in its fields
	the resources, which start with 'rsrc'; none if the section is empty
	'blkf'

and each resource is the data of one field of the section.
*/

#ifndef __GEOMETRY_DEFINITIONS_H
#define __GEOMETRY_DEFINITIONS_H

#include "tag_files/tag_groups.h"

#include <stddef.h>

/* ---------- constants */

#define GEOMETRY_BLOCK_HEADER_SIGNATURE 'blkh'
#define GEOMETRY_BLOCK_RESOURCES_SIGNATURE 'rsrc'
#define GEOMETRY_BLOCK_FOOTER_SIGNATURE 'blkf'

enum
{
	/* the two high bits of a block offset tell the file that holds the
	block: the map itself, mainmenu, shared or single_player_shared */
	GEOMETRY_BLOCK_OFFSET_FILE_SHIFT= 30,
	GEOMETRY_BLOCK_OFFSET_MASK= 0x3FFFFFFF
};

enum
{
	/* the structure bsp's clusters have 0 and its instanced geometry has 1;
	the sections of the render models have 1, 2 or 3, by how their vertices
	move with the skeleton */
	_geometry_classification_worldspace,
	_geometry_classification_rigid
};

enum
{
	/* the part's indices are a list of triangles, three indices each;
	without it they are a triangle strip. The structure bsps have only
	lists; the render models have both */
	_geometry_part_triangle_list_bit= 3
};

enum
{
	/* the secondary locator is the size of an element */
	_geometry_block_resource_type_tag_block,
	_geometry_block_resource_type_tag_data,
	/* the primary locator is the offset of the vertex buffers and the
	secondary locator is the index of the vertex buffer */
	_geometry_block_resource_type_vertex_buffer
};

/* the types of vertex buffer, with the bytes of a vertex. A compressed
coordinate is a 16-bit integer, which goes from the least to the greatest
value that the tag's compression info gives. The types with more bytes than
the position needs have the node indices and the node weights of a vertex
after the position */
enum
{
	/* three reals: 12 bytes */
	_geometry_vertex_buffer_type_position= 0x01,
	/* three compressed coordinates: 6 bytes */
	_geometry_vertex_buffer_type_compressed_position= 0x02,
	/* 8 bytes */
	_geometry_vertex_buffer_type_compressed_position_rigid_boned= 0x04,
	/* 12 bytes each */
	_geometry_vertex_buffer_type_compressed_position_skinned_a= 0x06,
	_geometry_vertex_buffer_type_compressed_position_skinned_b= 0x08,
	/* three reals and 4 bytes: 16 bytes */
	_geometry_vertex_buffer_type_position_skinned= 0x0A,
	/* two reals: 8 bytes */
	_geometry_vertex_buffer_type_texcoord= 0x18,
	/* two compressed coordinates: 4 bytes */
	_geometry_vertex_buffer_type_compressed_texcoord= 0x19
	/* seen and not read: 0x1B (12 bytes), 0x1F (4 bytes), 0x2F (3 bytes),
	0x30 (4 bytes) */
};

/* ---------- structures */

struct geometry_section_info
{
	uint16_t total_vertex_count;
	uint16_t total_triangle_count;
	uint16_t total_part_count;
	uint8_t unknown6[0xE];
	uint16_t geometry_classification;
	uint16_t geometry_compression_flags;
	struct tag_block compression_info;
	uint8_t unknown20[8];
};

struct geometry_block_info
{
	uint32_t block_offset;
	uint32_t block_size;
	uint32_t section_data_size;
	uint32_t resource_data_size;
	struct tag_block resources; /* geometry_block_resource */
	uint32_t owner_tag_index;
	int16_t owner_tag_section_offset;
	uint8_t unknown1E[6];
};

struct geometry_block_resource
{
	uint8_t type;
	uint8_t pad[3];
	/* the offset in the section of the field that the data is of */
	int16_t primary_locator;
	int16_t secondary_locator;
	uint32_t resource_data_size;
	/* from the start of the resources */
	uint32_t resource_data_offset;
};

struct geometry_section
{
	struct tag_block parts; /* geometry_part */
	struct tag_block unknown8; /* 8 bytes each */
	struct tag_block unknown10;
	struct tag_block unknown18;
	/* each part has some: a list of triangles or a triangle strip */
	struct tag_block indices; /* uint16_t */
	uint8_t unknown28[8]; /* tag data */
	struct tag_block unknown30; /* 2 bytes each */
	struct tag_block vertex_buffers; /* geometry_vertex_buffer */
	uint32_t unknown40;
};

struct geometry_part
{
	uint16_t type;
	uint16_t flags;
	int16_t material_index;
	uint16_t first_index;
	uint16_t index_count;
	uint8_t unknownA[0x3E];
};

struct geometry_vertex_buffer
{
	uint8_t type;
	/* the bytes of a vertex */
	uint8_t vertex_size;
	uint8_t unknown2[0x1E];
};

typedef char verify_geometry_section_info_size[
	sizeof(struct geometry_section_info) == 0x28 ? 1 : -1];
typedef char verify_geometry_section_info_compression_info_offset[
	offsetof(struct geometry_section_info, compression_info) == 0x18 ? 1 : -1];
typedef char verify_geometry_block_info_size[
	sizeof(struct geometry_block_info) == 0x24 ? 1 : -1];
typedef char verify_geometry_block_resource_size[
	sizeof(struct geometry_block_resource) == 0x10 ? 1 : -1];
typedef char verify_geometry_section_size[
	sizeof(struct geometry_section) == 0x44 ? 1 : -1];
typedef char verify_geometry_section_vertex_buffers_offset[
	offsetof(struct geometry_section, vertex_buffers) == 0x38 ? 1 : -1];
typedef char verify_geometry_part_size[
	sizeof(struct geometry_part) == 0x48 ? 1 : -1];
typedef char verify_geometry_vertex_buffer_size[
	sizeof(struct geometry_vertex_buffer) == 0x20 ? 1 : -1];

#endif
