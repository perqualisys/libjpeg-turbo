#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef NDEBUG
#include <assert.h>

#include <jpeglib.h>

/* Compress a smooth RGB gradient to memory, then decompress it at every scale
 * (1/1 to 1/8), in RGB and RGBA, and check the pixels are close to the
 * original. The color conversion, upsampling and IDCT all run on the way.
 */

#define W 256
#define H 192

static unsigned char
pixel (int x, int y, int c)
{
  return (unsigned char) (c == 0 ? x : c == 1 ? (y * 255) / (H - 1) : (x + y) / 2);
}

static int
decode (unsigned char* jpeg, unsigned long size, int denom, J_COLOR_SPACE cs)
{
  struct jpeg_decompress_struct d;
  struct jpeg_error_mgr e;
  JSAMPROW row;
  unsigned char* buf;
  int max_err = 0;
  int comps;

  d.err = jpeg_std_error (&e);
  jpeg_create_decompress (&d);
  jpeg_mem_src (&d, jpeg, size);
  jpeg_read_header (&d, TRUE);
  d.scale_num = 1;
  d.scale_denom = denom;
  d.out_color_space = cs;
  jpeg_start_decompress (&d);

  assert ((int) d.output_width == W / denom);
  assert ((int) d.output_height == H / denom);

  comps = d.output_components;
  assert (comps == (cs == JCS_EXT_RGBA ? 4 : 3));

  buf = malloc ((size_t) d.output_width * comps);

  while (d.output_scanline < d.output_height)
  {
    int y = (int) d.output_scanline;
    int x, c;

    row = buf;
    jpeg_read_scanlines (&d, &row, 1);

    for (x = 0; x < (int) d.output_width; ++x)
    {
      for (c = 0; c < 3; ++c)
      {
        int ref = pixel (x * denom + denom / 2, y * denom + denom / 2, c);
        int err = abs ((int) buf[x * comps + c] - ref);
        if (err > max_err)
          max_err = err;
      }
      if (comps == 4)
        assert (buf[x * comps + 3] == 255);
    }
  }

  jpeg_finish_decompress (&d);
  jpeg_destroy_decompress (&d);
  free (buf);
  return max_err;
}

int
main (void)
{
  struct jpeg_compress_struct c;
  struct jpeg_error_mgr e;
  unsigned char* jpeg = NULL;
  unsigned long size = 0;
  unsigned char row_buf[W * 3];
  JSAMPROW row = row_buf;
  int denoms[] = {1, 2, 4, 8};
  size_t i;

  c.err = jpeg_std_error (&e);
  jpeg_create_compress (&c);
  jpeg_mem_dest (&c, &jpeg, &size);
  c.image_width = W;
  c.image_height = H;
  c.input_components = 3;
  c.in_color_space = JCS_RGB;
  jpeg_set_defaults (&c);
  jpeg_set_quality (&c, 95, TRUE);
  jpeg_start_compress (&c, TRUE);

  while (c.next_scanline < c.image_height)
  {
    int x, ch;
    for (x = 0; x < W; ++x)
      for (ch = 0; ch < 3; ++ch)
        row_buf[x * 3 + ch] = pixel (x, (int) c.next_scanline, ch);
    jpeg_write_scanlines (&c, &row, 1);
  }

  jpeg_finish_compress (&c);
  jpeg_destroy_compress (&c);

  assert (size > 0);

  for (i = 0; i < sizeof (denoms) / sizeof (denoms[0]); ++i)
  {
    int rgb = decode (jpeg, size, denoms[i], JCS_RGB);
    int rgba = decode (jpeg, size, denoms[i], JCS_EXT_RGBA);
    printf ("1/%d: max error rgb %d, rgba %d\n", denoms[i], rgb, rgba);

    /* Scaled decoding averages blocks, so allow for the gradient's slope. */
    assert (rgb <= 8 * denoms[i] && rgba == rgb);
  }

  free (jpeg);
  return 0;
}
