/* LavaVu VideoEncoder
 * Owen Kaluza 2015, (c) Monash University
 * Based on code from libavformat/output-example.c
 ****************************************************
 * Libavformat API example: Output a media file in any supported
 * libavformat format. The default codecs are used.
 *
 * Copyright (c) 2003 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifdef HAVE_LIBAVCODEC

#include "VideoEncoder.h"

/**************************************************************/
/* video output */

/* add a video output stream */
AVStream* VideoEncoder::add_video_stream(enum AVCodecID codec_id)
{
   AVCodecContext *c;
   AVStream *st;
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53,10,1)
   st = avformat_new_stream(oc, NULL);
#else
   st = av_new_stream(oc, 0);
#endif
   if (!st)
   {
      fprintf(stderr, "Could not alloc stream\n");
      exit(1);
   }

   c = st->codec;
   c->codec_id = codec_id;
   c->codec_type = AVMEDIA_TYPE_VIDEO;

   /* resolution must be a multiple of two */
   c->width = width;
   c->height = height;

   if (codec_id == AV_CODEC_ID_H264)
   {
      /* h264 settings */
      c->profile = FF_PROFILE_H264_MAIN; //BASELINE;
      c->flags |= CODEC_FLAG_LOOP_FILTER;

      c->coder_type = 0;
      c->me_cmp |= 1; // cmp=+chroma, where CHROMA = 1
      c->me_method = ME_HEX;
      c->me_subpel_quality = 7; //9, 7, 0
      c->me_range = 16;

      c->scenechange_threshold = 40;
      c->i_quant_factor = 0.71;
      c->qcompress = 0.6; // qcomp=0.5
      c->qmin = 2;        //10 default
      c->qmax = 31;       //51 default

      c->max_qdiff = 4;
      c->refs = 3; // reference frames

      c->max_b_frames = 16; //Default
      c->b_frame_strategy = 1;
      c->chromaoffset = 0;
      c->thread_count = 1;

      //c->trellis = 1/0;
      //c->level = 13/41; //?????

      c->bit_rate = (int)(3200000.f * 0.80f);
      c->bit_rate_tolerance = (int)(3200000.f * 0.20f);
   }
   else
   {
      if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO)
      {
         /* just for testing, we also add B frames */
         c->max_b_frames = 2;
      }
      if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
      {
         /* Needed to avoid using macroblocks in which some coeffs overflow.
            This does not happen with normal video, it just happens here as
            the motion of the chroma plane does not match the luma plane. */
         c->mb_decision = 2;
      }
      // some formats want stream headers to be separate
      if(oc->oformat->flags & AVFMT_GLOBALHEADER)
         c->flags |= CODEC_FLAG_GLOBAL_HEADER;

      //MPEG needs a higher bitrate for quality
      c->bit_rate = (int)(16000000.f * 0.80f);
      c->bit_rate_tolerance = (int)(16000000.f * 0.20f);
   }

   /* frames per second */
   /* time base: this is the fundamental unit of time (in seconds) in terms
    of which frame timestamps are represented. for fixed-fps content,
    timebase should be 1/framerate and timestamp increments should be
    identically 1. */
   c->time_base.num = 1;
   c->time_base.den = fps; /* frames per second */
   c->pix_fmt = PIX_FMT_YUV420P;
   c->gop_size = 4; /* Maximum distance between key-frames, low setting allows fine granularity seeking */
   //c->keyint_min = 4; /*Minimum distance between keyframes */

   if (oc->oformat->flags & AVFMT_GLOBALHEADER)
      c->flags |= CODEC_FLAG_GLOBAL_HEADER;

   return st;
}

AVFrame *VideoEncoder::alloc_picture(enum PixelFormat pix_fmt)
{
   AVFrame *picture;
   uint8_t *picture_buf;
   int size;

   picture = avcodec_alloc_frame();
   if (!picture)
      return NULL;
   size = avpicture_get_size(pix_fmt, width, height);
   picture_buf = (uint8_t*)av_malloc(size);
   if (!picture_buf)
   {
      av_free(picture);
      return NULL;
   }
   avpicture_fill((AVPicture *)picture, picture_buf,
                  pix_fmt, width, height);
   return picture;
}

void VideoEncoder::open_video()
{
   AVCodec *codec;
   AVCodecContext *c;

   c = video_st->codec;

   /* find the video encoder */
   codec = avcodec_find_encoder(c->codec_id);
   if (!codec)
   {
      fprintf(stderr, "codec not found\n");
      exit(1);
   }

   /* open the codec */
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT(53,6,1)
   /* This is supposed to be the way to do it but doesn't work... */
   AVDictionary *opts = NULL;
   av_dict_set(&opts, "vprofile", "main", 0);
   av_dict_set(&opts, "preset", "fast",0);
   av_dict_set(&opts, "tune", "film",0);
   if (avcodec_open2(c, codec, &opts) < 0)
#else
   if (avcodec_open(c, codec) < 0)
#endif
   {
      fprintf(stderr, "could not open codec\n");
      exit(1);
   }

   video_outbuf = NULL;

   /* Not supporting this anymore */
   assert(!(oc->oformat->flags & AVFMT_RAWPICTURE));

   /* allocate output buffer */
   /* buffers passed into lav* can be allocated any way you prefer,
      as long as they're aligned enough for the architecture, and
      they're freed appropriately (such as using av_free for buffers
      allocated with av_malloc) */
   video_outbuf_size = 6000000;
   video_outbuf = (uint8_t*)av_malloc(video_outbuf_size);

   /* allocate the encoded raw picture */
   picture = alloc_picture(c->pix_fmt);
   if (!picture)
   {
      fprintf(stderr, "Could not allocate picture\n");
      exit(1);
   }

   /* Only supporting YUV420P now */
   assert(c->pix_fmt == PIX_FMT_YUV420P);
}

void VideoEncoder::write_video_frame()
{
   int ret = 0;
   AVCodecContext *c = video_st->codec;
   AVPacket pkt;
   av_init_packet(&pkt);

   /* encode the image */
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(54,0,0)
   pkt.size = video_outbuf_size;
   pkt.data = video_outbuf;
   int got_packet = 0;
   ret = avcodec_encode_video2(c, &pkt, picture, &got_packet);
   if (got_packet)
   {
      if (pkt.pts != AV_NOPTS_VALUE)
         pkt.pts = av_rescale_q(pkt.pts, c->time_base, video_st->time_base);
      if (pkt.dts != AV_NOPTS_VALUE)
         pkt.dts = av_rescale_q(pkt.dts, c->time_base, video_st->time_base);
#else
   ret = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
   /* if zero size, it means the image was buffered */
   if (ret > 0)
   {
      if (c->coded_frame->pts != AV_NOPTS_VALUE)
         pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, video_st->time_base);
      if (c->coded_frame->key_frame)
         pkt.flags |= AV_PKT_FLAG_KEY;
      pkt.stream_index = video_st->index;
      pkt.data = video_outbuf;
      pkt.size = ret;
#endif
      /* write the compressed frame in the media file */
      ret = av_interleaved_write_frame(oc, &pkt);
   }
   
   if (ret != 0)
   {
      fprintf(stderr, "Error while writing video frame\n");
      exit(1);
   }
   frame_count++;
}

void VideoEncoder::close_video()
{
   avcodec_close(video_st->codec);
   av_free(picture->data[0]);
   av_free(picture);
   av_free(video_outbuf);
}

/**************************************************************/
/* media file output */

AVOutputFormat *VideoEncoder::defaultCodec(const char *filename)
{
   AVOutputFormat *fmt;
   /* auto detect the output format from the name. default is mpeg. */
   fmt = av_guess_format(NULL, filename, NULL);
   if (!fmt)
   {
      fprintf(stderr, "Could not deduce output format from file extension: using MPEG.\n");
      fmt = av_guess_format("mpeg", NULL, NULL);
   }
   if (!fmt)
   {
      fprintf(stderr, "Could not find suitable output format\n");
      exit(1);
   }
   return fmt;
}

VideoEncoder::VideoEncoder(const char *filename, int width, int height, int fps) : width(width), height(height), fps(fps)
{
   fprintf(stderr, "Using libavformat %d.%d libavcodec %d.%d\n", LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR);
   //Create the frame buffer
   buffer = new unsigned char[width * height * 3];

   /* initialize libavcodec, and register all codecs and formats */
   av_register_all();

   /* allocate the output media context */
   oc = avformat_alloc_context();
   if (!oc)
   {
      fprintf(stderr, "Memory error\n");
      exit(1);
   }
   oc->oformat = defaultCodec(filename);
   snprintf(oc->filename, sizeof(oc->filename), "%s", filename);

   /* Codec override, use h264 for mp4 if available */
   if (strstr(filename, ".mp4") && avcodec_find_encoder(AV_CODEC_ID_H264))
      oc->oformat->video_codec = AV_CODEC_ID_H264; //h.264

   /* add the audio and video streams using the default format codecs
      and initialize the codecs */
   video_st = NULL;
   if (oc->oformat->video_codec != AV_CODEC_ID_NONE)
      video_st = add_video_stream(oc->oformat->video_codec);

   oc->oformat->audio_codec = AV_CODEC_ID_NONE;

#if LIBAVFORMAT_VERSION_MAJOR <= 52
   av_set_parameters(oc, NULL);
#endif

   av_dump_format(oc, 0, filename, 1);

   /* now that all the parameters are set, we can open the audio and
      video codecs and allocate the necessary encode buffers */
   if (video_st)
      open_video();

   /* open the output file */
   if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0)
   {
      fprintf(stderr, "Could not open '%s'\n", filename);
      exit(1);
   }

   /* write the stream header, if any */
   /* also sets the output parameters (none). */
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53,2,0)
   avformat_write_header(oc, NULL);
#else
   av_write_header(oc);
#endif
}

void VideoEncoder::frame(int channels)
{
   /* YUV420P encoded frame */
   int            pixel_I;
   int            xPixel_I;
   int            yPixel_I;
   unsigned char* macroPixel0;
   unsigned char* macroPixel1;
   unsigned char* macroPixel2;
   unsigned char* macroPixel3;
   float          red, green, blue;

   /* Setup the 'Y' (luminance) part of the frame */
   for ( yPixel_I = 0 ; yPixel_I < height ; yPixel_I++ )
   {
      for ( xPixel_I = 0 ; xPixel_I < width ; xPixel_I++ )
      {
         pixel_I = yPixel_I * width + xPixel_I;

         red   = (float) buffer[channels*pixel_I];
         green = (float) buffer[channels*pixel_I+1];
         blue  = (float) buffer[channels*pixel_I+2];

         picture->data[0][yPixel_I * picture->linesize[0] + xPixel_I] =
            (unsigned char)((0.257 * red) + (0.504 * green) + (0.098 * blue) + 16);
      }
   }

   /* Setup the 'Cb (U)' and 'Cr (V)' part of the frame
    * loop over macro pixels - pixels twice the size as the normal ones */
   for ( yPixel_I = 0 ; yPixel_I < height/2 ; yPixel_I++ )
   {
      for ( xPixel_I = 0 ; xPixel_I < width/2 ; xPixel_I++ )
      {
         /* Find four pixels in this macro pixel */
         int idx0 = channels*(yPixel_I*2) * width + xPixel_I * 2*channels;
         int idx1 = channels*(yPixel_I*2+1) * width + xPixel_I * 2*channels;
         macroPixel0 = buffer + idx0;
         macroPixel1 = buffer + idx0 + channels;
         macroPixel2 = buffer + idx1;
         macroPixel3 = buffer + idx1 + channels;

         /* Average red, green and blue for four pixels around point */
         red   = 0.25 * ((float) (macroPixel0[0] + macroPixel1[0] + macroPixel2[0] + macroPixel3[0] ));
         green = 0.25 * ((float) (macroPixel0[1] + macroPixel1[1] + macroPixel2[1] + macroPixel3[1] ));
         blue  = 0.25 * ((float) (macroPixel0[2] + macroPixel1[2] + macroPixel2[2] + macroPixel3[2] ));

         /* 'Cb (U)' Component */
         picture->data[1][yPixel_I * picture->linesize[1] + xPixel_I] =
            (unsigned char) (-(0.148 * red) - (0.291 * green) + (0.439 * blue) + 128);

         /* 'Cr (V)' Component */
         picture->data[2][yPixel_I * picture->linesize[2] + xPixel_I] =
            (unsigned char) ((0.439 * red) - (0.368 * green) - (0.071 * blue) + 128);
      }
   }

   /* Calculate PTS for h264 */
   picture->pts = video_st->codec->frame_number;

   /* write video frames */
   write_video_frame();
}

VideoEncoder::~VideoEncoder()
{
   /* OK: Moved this comment to here, writing extra frames or
      we seem to skip the last few...
    * No more frame to compress. The codec has a latency of a few
      frames if using B frames, so we get the last frames by
      passing the same picture again */
   for (int i=0; i<8; i++)
   {
      picture->pts++;
      write_video_frame();
   }

   /* write the trailer, if any.  the trailer must be written
    * before you close the CodecContexts open when you wrote the
    * header; otherwise write_trailer may try to use memory that
    * was freed on av_codec_close() */
   av_write_trailer(oc);

   /* close each codec */
   if (video_st)
      close_video();

   /* free the streams */
   for(unsigned int i = 0; i < oc->nb_streams; i++)
   {
      av_freep(&oc->streams[i]->codec);
      av_freep(&oc->streams[i]);
   }

   /* close the output file */
   avio_close(oc->pb);

   /* free the stream */
   av_free(oc);

   //Free framebuffer
   delete[] buffer;
}

#endif //HAVE_LIBAVCODEC
