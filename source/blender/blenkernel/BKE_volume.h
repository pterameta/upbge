/* SPDX-FileCopyrightText: 2023 Blender Foundation
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

/** \file
 * \ingroup bke
 * \brief Volume data-block.
 */
#ifdef __cplusplus
extern "C" {
#endif

struct BoundBox;
struct Depsgraph;
struct Main;
struct Object;
struct ReportList;
struct Scene;
struct Volume;
struct VolumeGridVector;

/* Module */

void BKE_volumes_init(void);

/* Data-block Management */

void BKE_volume_init_grids(struct Volume *volume);
void *BKE_volume_add(struct Main *bmain, const char *name);

struct BoundBox *BKE_volume_boundbox_get(struct Object *ob);

bool BKE_volume_is_y_up(const struct Volume *volume);
bool BKE_volume_is_points_only(const struct Volume *volume);

/* Depsgraph */

void BKE_volume_eval_geometry(struct Depsgraph *depsgraph, struct Volume *volume);
void BKE_volume_data_update(struct Depsgraph *depsgraph,
                            struct Scene *scene,
                            struct Object *object);

void BKE_volume_grids_backup_restore(struct Volume *volume,
                                     struct VolumeGridVector *grids,
                                     const char *filepath);

/* Draw Cache */

enum {
  BKE_VOLUME_BATCH_DIRTY_ALL = 0,
};

void BKE_volume_batch_cache_dirty_tag(struct Volume *volume, int mode);
void BKE_volume_batch_cache_free(struct Volume *volume);

extern void (*BKE_volume_batch_cache_dirty_tag_cb)(struct Volume *volume, int mode);
extern void (*BKE_volume_batch_cache_free_cb)(struct Volume *volume);

/* Grids
 *
 * For volumes referencing a file, the list of grids and metadata must be
 * loaded before it can be accessed. This happens on-demand, only when needed
 * by the user interface, dependency graph or render engine. */

typedef struct VolumeGrid VolumeGrid;

bool BKE_volume_load(const struct Volume *volume, const struct Main *bmain);
void BKE_volume_unload(struct Volume *volume);
bool BKE_volume_is_loaded(const struct Volume *volume);

int BKE_volume_num_grids(const struct Volume *volume);
const char *BKE_volume_grids_error_msg(const struct Volume *volume);
const char *BKE_volume_grids_frame_filepath(const struct Volume *volume);
const VolumeGrid *BKE_volume_grid_get_for_read(const struct Volume *volume, int grid_index);
VolumeGrid *BKE_volume_grid_get_for_write(struct Volume *volume, int grid_index);
const VolumeGrid *BKE_volume_grid_active_get_for_read(const struct Volume *volume);
/* Tries to find a grid with the given name. Make sure that the volume has been loaded. */
const VolumeGrid *BKE_volume_grid_find_for_read(const struct Volume *volume, const char *name);
VolumeGrid *BKE_volume_grid_find_for_write(struct Volume *volume, const char *name);

/* Tries to set the name of the velocity field. If no such grid exists with the given base name,
 * this will try common post-fixes in order to detect velocity fields split into multiple grids.
 * Return false if neither finding with the base name nor with the post-fixes succeeded. */
bool BKE_volume_set_velocity_grid_by_name(struct Volume *volume, const char *base_name);

/* Grid
 *
 * By default only grid metadata is loaded, for access to the tree and voxels
 * BKE_volume_grid_load must be called first. */

typedef enum VolumeGridType {
  VOLUME_GRID_UNKNOWN = 0,
  VOLUME_GRID_BOOLEAN,
  VOLUME_GRID_FLOAT,
  VOLUME_GRID_DOUBLE,
  VOLUME_GRID_INT,
  VOLUME_GRID_INT64,
  VOLUME_GRID_MASK,
  VOLUME_GRID_VECTOR_FLOAT,
  VOLUME_GRID_VECTOR_DOUBLE,
  VOLUME_GRID_VECTOR_INT,
  VOLUME_GRID_POINTS,
} VolumeGridType;

bool BKE_volume_grid_load(const struct Volume *volume, const struct VolumeGrid *grid);
void BKE_volume_grid_unload(const struct Volume *volume, const struct VolumeGrid *grid);
bool BKE_volume_grid_is_loaded(const struct VolumeGrid *grid);

/* Metadata */

const char *BKE_volume_grid_name(const struct VolumeGrid *grid);
VolumeGridType BKE_volume_grid_type(const struct VolumeGrid *grid);
int BKE_volume_grid_channels(const struct VolumeGrid *grid);
/**
 * Transformation from index space to object space.
 */
void BKE_volume_grid_transform_matrix(const struct VolumeGrid *grid, float mat[4][4]);
void BKE_volume_grid_transform_matrix_set(const struct Volume *volume,
                                          struct VolumeGrid *volume_grid,
                                          const float mat[4][4]);

/* Volume Editing
 *
 * These are intended for modifiers to use on evaluated data-blocks.
 *
 * new_for_eval creates a volume data-block with no grids or file path, but
 * preserves other settings such as viewport display options.
 *
 * copy_for_eval creates a volume data-block preserving everything except the
 * file path. Grids are shared with the source data-block, not copied. */

struct Volume *BKE_volume_new_for_eval(const struct Volume *volume_src);
struct Volume *BKE_volume_copy_for_eval(const struct Volume *volume_src);

struct VolumeGrid *BKE_volume_grid_add(struct Volume *volume,
                                       const char *name,
                                       VolumeGridType type);
void BKE_volume_grid_remove(struct Volume *volume, struct VolumeGrid *grid);

/**
 * OpenVDB crashes when the determinant of the transform matrix becomes too small.
 */
bool BKE_volume_grid_determinant_valid(double determinant);

/* Simplify */
int BKE_volume_simplify_level(const struct Depsgraph *depsgraph);
float BKE_volume_simplify_factor(const struct Depsgraph *depsgraph);

/* File Save */
bool BKE_volume_save(const struct Volume *volume,
                     const struct Main *bmain,
                     struct ReportList *reports,
                     const char *filepath);

#ifdef __cplusplus
}
#endif

/* OpenVDB Grid Access
 *
 * Access to OpenVDB grid for C++. These will automatically load grids from
 * file or copy shared grids to make them writeable. */

#ifdef __cplusplus

#  include "BLI_math_vector_types.hh"

bool BKE_volume_min_max(const Volume *volume, blender::float3 &r_min, blender::float3 &r_max);

#endif
