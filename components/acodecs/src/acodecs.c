#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <acodecs.h>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.h>
#include <dr_mp3.h>
#include <xmp.h>

static int acodec_mp3_open(void **handle, const char *filename);
static int acodec_mp3_get_info(void *handle, AudioInfo *info);
static int acodec_mp3_decode(void *handle, int16_t *buf_out, int num_c, unsigned len);
static int acodec_mp3_close(void *handle);

static int acodec_ogg_open(void **handle, const char *filename);
static int acodec_ogg_get_info(void *handle, AudioInfo *info);
static int acodec_ogg_decode(void *handle, int16_t *buf_out, int num_c, unsigned len);
static int acodec_ogg_close(void *handle);

static int acodec_libxmp_open(void **handle, const char *filename);
static int acodec_libxmp_get_info(void *handle, AudioInfo *info);
static int acodec_libxmp_decode(void *handle, int16_t *buf_out, int num_c, unsigned len);
static int acodec_libxmp_close(void *handle);

static AudioDecoder mp3_decoder = {
    .open = acodec_mp3_open,
    .get_info = acodec_mp3_get_info,
    .decode = acodec_mp3_decode,
    .close = acodec_mp3_close,
};

static AudioDecoder ogg_decoder = {
    .open = acodec_ogg_open,
    .get_info = acodec_ogg_get_info,
    .decode = acodec_ogg_decode,
    .close = acodec_ogg_close,
};

static AudioDecoder libxmp_decoder = {
    .open = acodec_libxmp_open,
    .get_info = acodec_libxmp_get_info,
    .decode = acodec_libxmp_decode,
    .close = acodec_libxmp_close,
};

// TODO: Add function that describes the error
static int acodec_error = 0;

AudioDecoder *acodec_get_decoder(AudioCodec codec)
{
	switch (codec) {
	case AudioCodecMP3:
		return &mp3_decoder;
	case AudioCodecOGG:
		return &ogg_decoder;
	case AudioCodecMOD:
		return &libxmp_decoder;
	default:
		return NULL;
	}
}

/* ---------------------------------------------------------- */
/* MP3 */
/* ---------------------------------------------------------- */

static int acodec_mp3_open(void **handle, const char *filename)
{
	assert(filename != NULL);

	drmp3 *mp3 = malloc(sizeof(drmp3));
	if (!drmp3_init_file(mp3, filename, NULL)) {
		free(mp3);
		return -1;
	}
	*handle = mp3;
	return 0;
}

static int acodec_mp3_get_info(void *handle, AudioInfo *info)
{
	drmp3 *mp3 = (drmp3 *)handle;
	info->channels = mp3->channels;
	info->sample_rate = mp3->sampleRate;
	info->buf_size = 4096;
	return 0;
}

static int acodec_mp3_decode(void *handle, int16_t *buf_out, int num_c, unsigned len)
{
	(void)num_c;

	drmp3 *mp3 = (drmp3 *)handle;
	drmp3_uint64 n_frames = drmp3_read_pcm_frames_s16(mp3, ((drmp3_uint64)len / 2), buf_out);
	return (int)n_frames;
}

static int acodec_mp3_close(void *handle)
{
	drmp3 *mp3 = (drmp3 *)handle;
	drmp3_uninit(mp3);
	free(mp3);
	return 0;
}

/* ---------------------------------------------------------- */
/* OGG */
/* ---------------------------------------------------------- */

static int acodec_ogg_open(void **handle, const char *filename)
{
	assert(filename != NULL);

	int error;
	*handle = stb_vorbis_open_filename(filename, &error, NULL);

	if (*handle == NULL) {
		acodec_error = error;
		return -1;
	}
	return 0;
}

static int acodec_ogg_get_info(void *handle, AudioInfo *info)
{
	assert(handle != NULL);

	stb_vorbis_info vorbis_info = stb_vorbis_get_info(handle);
	info->sample_rate = vorbis_info.sample_rate;
	info->channels = (unsigned)vorbis_info.channels;
	info->buf_size = 4096;

	return 0;
}

static int acodec_ogg_decode(void *handle, int16_t *buf_out, int num_c, unsigned len)
{
	assert(handle != NULL);

	int n_frames = stb_vorbis_get_frame_short_interleaved(handle, num_c, buf_out, (int)len);
	/* n_frames is number of decoded frames per channel */

	return n_frames;
}

static int acodec_ogg_close(void *handle)
{
	assert(handle != NULL);

	stb_vorbis_close(handle);
	return 0;
}

/* ---------------------------------------------------------- */
/* libxmp-lite for .xm, .mod, .s3m and .it */
/* ---------------------------------------------------------- */

#define LIBXMP_SAMPLERATE 44100

static int acodec_libxmp_open(void **handle, const char *filename)
{
	assert(filename != NULL);

	xmp_context ctx = xmp_create_context();
	if (xmp_load_module(ctx, (char *)filename) != 0) {
		// TODO: Add error handling
		return -1;
	}

	xmp_start_player(ctx, LIBXMP_SAMPLERATE, 0);
	*handle = ctx;

	return 0;
}

static int acodec_libxmp_get_info(void *handle, AudioInfo *info)
{
	assert(handle != NULL);

	info->sample_rate = LIBXMP_SAMPLERATE;
	info->channels = 2;
	info->buf_size = 4096;

	return 0;
}

static int acodec_libxmp_decode(void *handle, int16_t *buf_out, int num_c, unsigned len)
{
	assert(handle != NULL);
	xmp_context ctx = (xmp_context)handle;
	(void)num_c;

	if (xmp_play_buffer(ctx, buf_out, (int)(len * sizeof(uint16_t)), 1) != 0) {
		// TODO: Error handling
		fprintf(stderr, "error while playing module!\n");
		return 0;
	}

	return len / 2;
}

static int acodec_libxmp_close(void *handle)
{
	assert(handle != NULL);

	xmp_context ctx = (xmp_context)handle;
	xmp_end_player(ctx);
	xmp_release_module(ctx); /* unload module */
	xmp_free_context(ctx);   /* destroy the player context */
	return 0;
}
