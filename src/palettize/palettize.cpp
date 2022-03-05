// This file is part of palettize -- A palette generator based on k-means
// clustering with CIELAB colors.
// 
// MIT License
// 
// Copyright (c) 2021-2022 gvlsq
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma warning(pop)

#include "palettize.h"

static const int MAX_BITMAP_DIM = 100;

static const int PALETTE_BITMAP_WIDTH = 512;
static const int PALETTE_BITMAP_HEIGHT = 64;

static Palettize_Config parse_config_from_command_line(int argc, char **argv) {
	Palettize_Config config = {};
	config.source_path = 0;
	config.cluster_count = 5;
	config.seed = (u32)time(0);
	config.sort_type = SORT_TYPE_WEIGHT;
	config.dest_path = "palette.bmp";

	if (argc > 1) {
		config.source_path = argv[1];
	}
	if (argc > 2) {
		int cluster_count = atoi(argv[2]);
		config.cluster_count = clampi(1, cluster_count, 64);
	}
	if (argc > 3) {
		config.seed = (u32)atoi(argv[3]);
	}
	if (argc > 4) {
		char *sort_type_string = argv[4];
		if (strings_match(sort_type_string, "red", false)) {
			config.sort_type = SORT_TYPE_RED;
		} else if (strings_match(sort_type_string, "green", false)) {
			config.sort_type = SORT_TYPE_GREEN;
		} else if (strings_match(sort_type_string, "blue", false)) {
			config.sort_type = SORT_TYPE_BLUE;
		}
	}
	if (argc > 5) {
		config.dest_path = argv[5];
	}

	return config;
}

static void load_bitmap(Bitmap *bitmap, char *path) {
	bitmap->memory = stbi_load(path, &bitmap->width, &bitmap->height, 0, STBI_rgb_alpha);
	if (!bitmap->memory) {
		fprintf(stderr, "stb_image failed to load %s: %s\n", path, stbi_failure_reason());
		exit(EXIT_FAILURE);
	}

	bitmap->pitch = sizeof(u32)*bitmap->width;
}

static void allocate_bitmap(Bitmap *bitmap, int width, int height) {
	bitmap->memory = malloc(sizeof(u32)*width*height);
	bitmap->width = width;
	bitmap->height = height;
	bitmap->pitch = sizeof(u32)*width;
}

static void free_bitmap(Bitmap *bitmap) {
	free(bitmap->memory);
	bitmap->memory = 0;
}

static void resize_bitmap(Bitmap *bitmap, int max_dim) {
	float resize_factor = (float)max_dim / (float)maximum(bitmap->width, bitmap->height);

	Bitmap resized_bitmap;
	allocate_bitmap(&resized_bitmap,
					roundi(bitmap->width*resize_factor),
					roundi(bitmap->height*resize_factor));

	int x0 = 0;
	int x1 = resized_bitmap.width;
	int y0 = 0;
	int y1 = resized_bitmap.height;

	u8 *row = (u8 *)resized_bitmap.memory;
	for (int y = y0; y < y1; y++) {
		u32 *texel = (u32 *)row;
		for (int x = x0; x < x1; x++) {
			float u = (float)x / ((float)resized_bitmap.width - 1.0f);
			float v = (float)y / ((float)resized_bitmap.height - 1.0f);
			assert(0.0f <= u && u <= 1.0f);
			assert(0.0f <= v && v <= 1.0f);

			int sample_x = roundi(u*((float)bitmap->width - 1.0f));
			int sample_y = roundi(v*((float)bitmap->height - 1.0f));
			assert(0 <= sample_x && sample_x < bitmap->width);
			assert(0 <= sample_y && sample_y < bitmap->height);

			u32 sample = *(u32 *)((u8 *)bitmap->memory + sample_x*sizeof(u32) + sample_y*bitmap->pitch);

			*texel++ = sample;
		}

		row += resized_bitmap.pitch;
	}

	free_bitmap(bitmap);
	*bitmap = resized_bitmap;
}

static u32 assign_observation_to_cluster(KMeans_Cluster *clusters, int cluster_count,
										 Vector3 observation) {
	float closest_dist_squared = FLOAT_MAX;
	u32 closest_cluster_index = 0;

	for (int i = 0; i < cluster_count; i++) {
		KMeans_Cluster *cluster = &clusters[i];

		float d = length_squared(observation - cluster->centroid);
		if (d < closest_dist_squared) {
			closest_dist_squared = d;
			closest_cluster_index = (u32)i;
		}
	}
	assert(0 <= closest_cluster_index && closest_cluster_index < (u32)cluster_count);

	KMeans_Cluster *closest_cluster = &clusters[closest_cluster_index];
	if (closest_cluster->observation_count == closest_cluster->observation_capacity) {
		closest_cluster->observation_capacity *= 2;
		closest_cluster->observations = (Vector3 *)realloc(closest_cluster->observations, sizeof(Vector3)*closest_cluster->observation_capacity);
	}
	closest_cluster->observations[closest_cluster->observation_count++] = observation;

	return closest_cluster_index;
}

static void recalculate_cluster_centroids(KMeans_Cluster *clusters, int cluster_count) {
	for (int i = 0; i < cluster_count; i++) {
		KMeans_Cluster *cluster = &clusters[i];

		Vector3 sum = V3i(0, 0, 0);
		for (int j = 0; j < cluster->observation_count; j++) {
			sum += cluster->observations[j];
		}

		// It's erroneous to assert that cluster->observation_count is nonzero,
		// see: https://stackoverflow.com/a/54821667. Replace the zero case below
		// with a reseed?

		if (cluster->observation_count) {
			cluster->centroid = sum*(1.0f / cluster->observation_count);
		} else {
			cluster->centroid = sum;
		}

		cluster->observation_count = 0;
	}
}

static void sort_clusters_by_centroid(KMeans_Cluster *clusters, int cluster_count,
									  Sort_Type sort_type) {
	Vector3 focal_color = V3i(0, 0, 0);
	switch (sort_type) {
		case SORT_TYPE_RED:
			focal_color = {
				53.23288178584245f,
				80.10930952982204f,
				67.22006831026425f
			};
			break;

		case SORT_TYPE_GREEN:
			focal_color = {
				 87.73703347354422f,
				-86.18463649762525f,
				 83.18116474777854f
			};
			break;

		case SORT_TYPE_BLUE:
			focal_color = {
				 32.302586667249486f,
				 79.19666178930935f,
				-107.86368104495168f
			};
			break;
	}

	for (int i = 0; i < cluster_count; i++) {
		bool swapped = false;

		for (int j = 0; j < (cluster_count - 1); j++) {
			KMeans_Cluster *cluster_a = clusters + j;
			KMeans_Cluster *cluster_b = clusters + j + 1;

			if (sort_type == SORT_TYPE_WEIGHT) {
				if (cluster_b->observation_count > cluster_a->observation_count) {
					KMeans_Cluster swap = *cluster_a;
					*cluster_a = *cluster_b;
					*cluster_b = swap;

					swapped = true;
				}
			} else if (sort_type == SORT_TYPE_RED || sort_type == SORT_TYPE_GREEN || sort_type == SORT_TYPE_BLUE) {
				float dist_squared_to_color_a = length_squared(cluster_a->centroid - focal_color);
				float dist_squared_to_color_b = length_squared(cluster_b->centroid - focal_color);
				if (dist_squared_to_color_b < dist_squared_to_color_a) {
					KMeans_Cluster swap = *cluster_a;
					*cluster_a = *cluster_b;
					*cluster_b = swap;

					swapped = true;
				}
			}
		}

		if (!swapped) break;
	}
}

static void export_bmp(Bitmap *bitmap, char *path) {
	FILE *file = fopen(path, "wb");
	if (!file) {
		fprintf(stderr, "fopen failed on %s\n", path);
		exit(EXIT_FAILURE);
	}

	// Swizzling red and blue channels because that's what BMPs expect
	int x0 = 0;
	int x1 = bitmap->width;
	int y0 = 0;
	int y1 = bitmap->height;

	u8 *row = (u8 *)bitmap->memory;
	for (int y = y0; y < y1; y++) {
		u32 *texel = (u32 *)row;
		for (int x = x0; x < x1; x++) {
			u32 swizzled = (*texel & 0xFF) << 16 | ((*texel >> 8) & 0xFF) << 8 | ((*texel >> 16) & 0xFF) << 0 | ((*texel >> 24) & 0xFF) << 24;

			*texel++ = swizzled;
		}

		row += bitmap->pitch;
	}

	u32 bitmap_size = (u32)(bitmap->pitch*bitmap->height);

	Bitmap_Header header = {};
	header.type = 0x4D42;
	header.file_size = sizeof(header) + bitmap_size;
	header.off_bits = sizeof(header);
	header.size = 40; // sizeof(BITMAPINFOHEADER)
	header.width = bitmap->width;
	header.height = bitmap->height;
	header.planes = 1;
	header.bit_count = 32;
	header.compression = BI_RGB;

	fwrite(&header, sizeof(header), 1, file);
	fwrite(bitmap->memory, bitmap_size, 1, file);

	fclose(file);
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		fprintf(stderr, "Usage: %s <source path> [cluster count] [seed] [sort type] [dest path]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	Palettize_Config config = parse_config_from_command_line(argc, argv);

	// To improve performance, source images with extents greater than
	// MAX_BITMAP_DIM pixels are resized with nearest neighbor sampling
	Bitmap source_bitmap;
	load_bitmap(&source_bitmap, config.source_path);
	if (source_bitmap.width > MAX_BITMAP_DIM || source_bitmap.height > MAX_BITMAP_DIM) {
		resize_bitmap(&source_bitmap, MAX_BITMAP_DIM);
	}

	Bitmap prev_cluster_index_buffer;
	allocate_bitmap(&prev_cluster_index_buffer, source_bitmap.width, source_bitmap.height);

	Random_Series entropy = seed_series(config.seed);

	int cluster_count = config.cluster_count;
	KMeans_Cluster *clusters = (KMeans_Cluster *)malloc(sizeof(KMeans_Cluster)*cluster_count);
	for (int i = 0; i < cluster_count; i++) {
		KMeans_Cluster *cluster = &clusters[i];

		cluster->observation_capacity = 512;
		cluster->observation_count = 0;
		cluster->observations = (Vector3 *)malloc(sizeof(Vector3)*cluster->observation_capacity);

		// Naive cluster seeding
		u32 sample_x = random_u32_between(&entropy, 0, (u32)(source_bitmap.width - 1));
		u32 sample_y = random_u32_between(&entropy, 0, (u32)(source_bitmap.height - 1));
		u32 sample = *(u32 *)get_bitmap_ptr(source_bitmap, sample_x, sample_y);

		cluster->centroid = unpack_rgba_to_cielab(sample);
	}

	int x0 = 0;
	int x1 = source_bitmap.width;
	int y0 = 0;
	int y1 = source_bitmap.height;

	for (int iteration = 0;; iteration++) {
		bool assignments_changed = false;

		u8 *row = (u8 *)get_bitmap_ptr(source_bitmap, x0, y0);
		u8 *prev_cluster_index_row = (u8 *)get_bitmap_ptr(prev_cluster_index_buffer, x0, y0);
		for (int y = y0; y < y1; y++) {
			u32 *texel = (u32 *)row;
			u32 *prev_cluster_index_ptr = (u32 *)prev_cluster_index_row;
			for (int x = x0; x < x1; x++) {
				Vector3 texel_v3 = unpack_rgba_to_cielab(*texel);

				u32 closest_cluster_index = assign_observation_to_cluster(clusters, cluster_count, texel_v3);
				if (iteration > 0) {
					u32 prev_cluster_index = *prev_cluster_index_ptr;
					if (closest_cluster_index != prev_cluster_index) {
						assignments_changed = true;
					}
				}

				texel++;
				*prev_cluster_index_ptr++ = closest_cluster_index;
			}

			row += source_bitmap.pitch;
			prev_cluster_index_row += prev_cluster_index_buffer.pitch;
		}

		if (iteration == 0 || assignments_changed) recalculate_cluster_centroids(clusters, cluster_count); else break;
	}

	sort_clusters_by_centroid(clusters, cluster_count, config.sort_type);

	u8 *scanline = (u8 *)malloc(sizeof(u32)*PALETTE_BITMAP_WIDTH);

	u32 *row = (u32 *)scanline;
	for (int i = 0; i < cluster_count; i++) {
		KMeans_Cluster *cluster = &clusters[i];

		float weight = (float)cluster->observation_count / (float)(source_bitmap.width*source_bitmap.height);
		int cluster_width = roundi(weight*PALETTE_BITMAP_WIDTH);

		u32 color = pack_cielab_to_rgba(cluster->centroid);
		while (cluster_width--) {
			*row++ = color;
		}
	}

	Bitmap palette_bitmap;
	allocate_bitmap(&palette_bitmap, PALETTE_BITMAP_WIDTH, PALETTE_BITMAP_HEIGHT);
	u8 *dest_row = (u8 *)palette_bitmap.memory;
	for (int y = 0; y < palette_bitmap.height; y++) {
		u32 *source_texel = (u32 *)scanline;
		u32 *dest_texel = (u32 *)dest_row;
		for (int x = 0; x < palette_bitmap.width; x++) {
			*dest_texel++ = *source_texel++;
		}

		dest_row += palette_bitmap.pitch;
	}

	export_bmp(&palette_bitmap, config.dest_path);

	return 0;
}
